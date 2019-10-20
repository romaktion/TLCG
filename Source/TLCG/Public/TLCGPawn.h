// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "TLCGBattleInterface.h"
#include "TLCGPawn.generated.h"

class UTLCGMovement;
class UBoxComponent;
class ATLCGPawnTrack;

UCLASS(Abstract)
class TLCG_API ATLCGPawn : public APawn, public ITLCGBattleInterface
{
	GENERATED_UCLASS_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void StartBattle() override;

	virtual void StopBattle() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCGPawn", DisplayName = "OnKilled")
	void K2_OnKilled();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCGPawn", DisplayName = "OnRespawn")
	void K2_OnRespawn();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCGPawn", DisplayName = "OnMoveActivated")
	void K2_OnMoveActivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCGPawn", DisplayName = "OnMoveDeactivated")
	void K2_OnMoveDeactivated();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCGPawn", DisplayName = "OnRotate")
	void K2_OnRotate();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCGPawn", DisplayName = "ActivateSkill")
	void K2_ActivateSkill();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRespawn();

	UPROPERTY(Category="TLCGPawn", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UBoxComponent* BoxComponent;

	UPROPERTY(Category="TLCGPawn", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UTLCGMovement* TLCMovement;

	TSubclassOf<ATLCGPawnTrack> TrackClass;

	UPROPERTY(EditDefaultsOnly, Category = "Track")
	uint32 InitialTracksPoolSize;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TLCGPawn")
	FLinearColor Color;

private:
	UFUNCTION()
	void TurnLeft();

	UFUNCTION()
	void TurnRight();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnLeft();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnLRight();

	UFUNCTION()
	void Skill();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSkill();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSkill();

	TArray<ATLCGPawnTrack*> TracksPool;

	TArray<ATLCGPawnTrack*> SpawnedTracks;

protected:
	UFUNCTION()
	virtual void OnKilled(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );
	
	UFUNCTION()
	virtual void OnMoveActivated(UActorComponent* Component, bool bReset);

	UFUNCTION()
	virtual void OnMoveDeactivated(UActorComponent* Component);

	UFUNCTION()
	virtual void OnRotate(const FTransform& NewTransform, ATLCGPawnTrack* NewTrack, ATLCGPawnTrack* OldTrack);

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnKilled();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnMoveActivated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnMoveDeactivated();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRotate();

	ATLCGPawnTrack* SpawnTrack();

	FTransform StartTransform;
};
