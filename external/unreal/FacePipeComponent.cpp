#include "FacePipeComponent.h"

#include "Networking.h"
#include "HAL/Runnable.h"

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
	static TArray<uint8> ReceivedData;
	ReceivedData.SetNumUninitialized(UDP_MAX_SIZE);

    while (bRunThread)
    {
        FPlatformProcess::Sleep(0.001);

		uint32 BufferSize = 0;
		while (ListenSocket->HasPendingData(BufferSize))
		{
			ReceivedData.SetNumUninitialized(BufferSize, false);

			int32 Read = 0;
			if (ListenSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read, ESocketReceiveFlags::None))
			{
				DatagramQueue.Enqueue(ReceivedData);
			}
		}
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

	TArray<uint8> Datagram;
	while (UDPListener->DatagramQueue.Dequeue(Datagram))
	{
		Datagram.Add(0); // null terminate
		FString DebugString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Datagram.GetData())));
		UE_LOG(LogTemp, Log, TEXT("Received data:\n%s"), *DebugString);

		TArray<FFacePipeBlendshapeData> BlendshapeData;
		float Timestamp = 0.0f;
		OnBlendshapesUpdate.Broadcast(BlendshapeData, Timestamp);
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
