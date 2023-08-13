#include "FacePipeComponent.h"

#include "Networking.h"
#include "HAL/Runnable.h"

#include "facepipe.h"

#define UDP_MAX_SIZE 65507

FFacePipeUDPListener::FFacePipeUDPListener(uint16 ListenPort)
	: Port(ListenPort)
{
	if (!Thread)
	{
		Thread = FRunnableThread::Create(this, TEXT("FacePipe UDP listener thread"));
	}
}

FFacePipeUDPListener::~FFacePipeUDPListener()
{
	if (Thread)
	{
		Thread->Kill(true);
		delete Thread;
		Thread = nullptr;
	}
}

bool FFacePipeUDPListener::Init()
{
    FIPv4Endpoint Endpoint(FIPv4Address::Any, Port);
    ListenSocket = FUdpSocketBuilder(*FString::Printf(TEXT("UDPSocket_%s"), *FString::FromInt(Port)))
        .AsReusable()
        .WithReceiveBufferSize(2 * 1024 * 1024)
        .BoundToEndpoint(Endpoint);

    ListenSocket->SetNonBlocking(true);

	return true;
}

uint32 FFacePipeUDPListener::Run()
{
	TArray<uint8> ReceivedData;
	ReceivedData.SetNumUninitialized(UDP_MAX_SIZE);

    while (bRunThread)
    {
		uint32 BufferSize = 0;
		while (ListenSocket->HasPendingData(BufferSize) && BufferSize > 0)
		{
			ReceivedData.SetNumUninitialized(BufferSize + 1, false); // +1 for null terminator

			int32 Read = 0;
			while (ListenSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read, ESocketReceiveFlags::None) && Read > 0)
			{
				ReceivedData.Last() = 0; // null terminate
				DatagramQueue.Enqueue(ReceivedData);
			}
		}

		// Stops working if we sleep, no idea why
		//FPlatformProcess::Sleep(0.001);
    }

    return 0;
}

void FFacePipeUDPListener::Stop()
{
	bRunThread = false;
}

void FFacePipeUDPListener::Exit()
{
    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }
}

UFacePipeComponent::UFacePipeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFacePipeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UFacePipeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopListening();

	Super::EndPlay(EndPlayReason);
}

void UFacePipeComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UDPListener)
		return;

	TArray<uint8> UEDatagram;
	while (UDPListener->DatagramQueue.Dequeue(UEDatagram))
	{
		if (UEDatagram.Num() < 2 || FacePipe::ToType(UEDatagram[0]) != FacePipe::EDatagramType::JSON)
			continue;

		if (UEDatagram.Last() != 0)
			UEDatagram.Add(0); // null terminate string

		// Data starts from second byte
		std::string Datagram(1 + reinterpret_cast<const char*>(UEDatagram.GetData()));

		FacePipe::MetaData FacePipeMetaData;
		nlohmann::json JSONData;
		if (!FacePipe::Parse(Datagram, FacePipeMetaData, JSONData))
		{
			continue;
		}

		switch (FacePipeMetaData.DataType)
		{
		case FacePipe::EFacepipeData::Blendshapes:
		{
			std::vector<std::string> Names;
			std::vector<float> Values;
			FacePipe::GetBlendshapes(FacePipeMetaData, JSONData, Names, Values);

			TArray<FFacePipeBlendshapeData> BlendshapeData;
			BlendshapeData.SetNumUninitialized(Names.size());
			for (size_t i = 0; i < Names.size(); i++)
			{
				BlendshapeData[i].Name = FName(Names[i].c_str());
				BlendshapeData[i].Value = Values[i];
			}

			OnBlendshapesUpdate.Broadcast(BlendshapeData, FacePipeMetaData.Time);
			break;
		}
		case FacePipe::EFacepipeData::Landmarks:
		{
			std::vector<float> Values;
			FacePipe::GetLandmarks(FacePipeMetaData, JSONData, Values);

			// Could probably do a memcpy but better not to... UE5 has a different data type for FVector.
			TArray<FVector> Landmarks;
			Landmarks.Reserve(Values.size()/3);
			for (int32 i=0; i<Values.size(); i += 3)
			{
				Landmarks.Add(FVector(Values[i], Values[i+1], Values[i+2]));
			}

			OnLandmarksUpdate.Broadcast(Landmarks, FacePipeMetaData.Time);
			break;
		}
		case FacePipe::EFacepipeData::Transforms:
		{
			break;
		}
		}
	}
}

void UFacePipeComponent::StartListening(int32 Port)
{
	if (!UDPListener)
	{
		UDPListener = new FFacePipeUDPListener((uint16) FMath::Abs(Port));
	}
}

void UFacePipeComponent::StopListening()
{
	if (UDPListener)
	{
		delete UDPListener;
		UDPListener = nullptr;
	}
}
