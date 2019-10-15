// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/FloatingPawnMovement.h"
#include "TLCGMovement.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams(FOnRotateSignature, UTLCGMovement, OnRotate, const FTransform&, NewTransform, ATLCGPawnTrack*, NewTrack, ATLCGPawnTrack*, OldTrack);

USTRUCT()
struct FRepData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector_NetQuantize Location;

	UPROPERTY()
	float Yaw;

	UPROPERTY()
	ATLCGPawnTrack* Track;

	FRepData()
	{
		Location = FVector_NetQuantize();
		Yaw = 0.f;
		Track = nullptr;
	}

	FRepData(const FVector& NewLocation, float NewYaw, ATLCGPawnTrack* NewTrack)
	{
		Location = NewLocation;
		Yaw = NewYaw;
		Track = NewTrack;
	}
};

/**
 * 
 */
UCLASS()
class TLCG_API UTLCGMovement : public UFloatingPawnMovement
{
	GENERATED_UCLASS_BODY()
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void TurnRight(ATLCGPawnTrack* NewTrack);

	void TurnLeft(ATLCGPawnTrack* NewTrack);

	void SetRepData(const FRepData& NewRepData);

	const FRepData& GetRepData() const;

	UPROPERTY()
	FOnRotateSignature OnRotate;

private:
	UFUNCTION()
	void OnRep_RepData(FRepData OldData);

	UPROPERTY(ReplicatedUsing = OnRep_RepData)
	FRepData RepData;

	FHitResult HitResult;

	AActor* CachedOwner;

	FVector PrevLoc;

	bool Sweep;
};
