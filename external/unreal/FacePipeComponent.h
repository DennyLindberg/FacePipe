// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "FacePipeComponent.generated.h"

class FFacePipeUDPListener : public FRunnable
{
public:
    FFacePipeUDPListener(uint16 ListenPort);
    virtual ~FFacePipeUDPListener();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
	virtual void Exit() override;

public:
	TQueue<TArray<uint8>> DatagramQueue;

protected:
	FRunnableThread* Thread = nullptr;
	FThreadSafeBool bRunThread = true;

    TSharedPtr<FInternetAddr> RemoteAddress;
    FSocket* ListenSocket;
    uint16 Port;
};

USTRUCT(BlueprintType)
struct FFacePipeBlendshapeData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFacePipeBlendshapesDelegate, const TArray<FFacePipeBlendshapeData>&, Data, float, Timestamp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFacePipeLandmarksDelegate, const TArray<FVector>&, Data, float, Timestamp);

UCLASS(BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class UFacePipeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFacePipeComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void StartListening(int32 Port);

	UFUNCTION(BlueprintCallable)
	void StopListening();

	UPROPERTY(BlueprintAssignable)
	FFacePipeBlendshapesDelegate OnBlendshapesUpdate;

	UPROPERTY(BlueprintAssignable)
	FFacePipeLandmarksDelegate OnLandmarksUpdate;

protected:
	FFacePipeUDPListener* UDPListener;
};
