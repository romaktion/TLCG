// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CanBeDamagerInterface.h"
#include "TLCGPawnTrack.generated.h"

class UBoxComponent;
class USceneComponent;

UCLASS(Abstract)
class TLCG_API ATLCGPawnTrack : public AActor, public ICanBeDamagerInterface
{
	GENERATED_UCLASS_BODY()
	
public:	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetTransform(const FTransform& NewTransform);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEnable(bool Enable);

	UPROPERTY(Category="TLCGPawnTrack", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UBoxComponent* BoxComponent;

	UPROPERTY(Category="TLCGPawnTrack", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;
};
