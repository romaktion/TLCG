// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/FloatingPawnMovement.h"
#include "TLCGMovement.generated.h"

/**
 * 
 */
UCLASS()
class TLCG_API UTLCGMovement : public UFloatingPawnMovement
{
	GENERATED_UCLASS_BODY()
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category = "Network")
	float PermissibleNetworkDiscrepancy;

private:
	UFUNCTION()
	void OnRep_RepZRotation();

	UPROPERTY(Replicated)
	FVector_NetQuantize RepLocation;

	UPROPERTY(ReplicatedUsing = OnRep_RepZRotation)
	float RepZRotation;

	FHitResult HitResult;

	AActor* CachedOwner;

	FVector PrevLoc;
};
