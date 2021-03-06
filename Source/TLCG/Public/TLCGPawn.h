// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "TLCGBattleInterface.h"
#include "CanBeDamagerInterface.h"
#include "TLCGPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FOnUseSkillSignature, ATLCGPawn, OnUseSkill, APlayerState*, Player, int32, NewAvaibleSkillsAmount);

UENUM(BlueprintType)
enum class ESwipeDirection : uint8
{
	SD_Up			UMETA(DisplayName = "UP"),
	SD_Down			UMETA(DisplayName = "Down"),
	SD_Left			UMETA(DisplayName = "left"),
	SD_Right		UMETA(DisplayName = "Right"),
};

class UTLCGMovement;
class UBoxComponent;
class ATLCGPawnTrack;

UCLASS(Abstract)
class TLCG_API ATLCGPawn : public APawn, public ITLCGBattleInterface, public ICanBeDamagerInterface
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
	void K2_ActivateSkill(int32 NewAvaibleSkillsAmount);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRespawn();

	UFUNCTION(BlueprintCallable, Category = "TLCGPawn")
	void UnlockSkill();

	void ClearTracks();

	UPROPERTY(Category="TLCGPawn", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UBoxComponent* BoxComponent;

	UPROPERTY(Category="TLCGPawn", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UTLCGMovement* TLCMovement;

	TSubclassOf<ATLCGPawnTrack> TrackClass;

	UPROPERTY(EditDefaultsOnly, Category = "Track")
	uint32 InitialTracksPoolSize;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TLCGPawn")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TLCGPawn")
	bool DisableSpawnTracks;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly)
	int32 AvaibleSkillsAmount;

	UPROPERTY(BlueprintAssignable, Category = "TLCGPawn")
	FOnUseSkillSignature OnUseSkill;

private:
	UFUNCTION()
	void TurnLeft();

	UFUNCTION()
	void TurnRight();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnLeft();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnRight();

	UFUNCTION()
	void Skill();

	UFUNCTION()
	void Touch();

	UFUNCTION()
	void UnTouch();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSkill();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSkill(APlayerState* Player, int32 InAvaibleSkillsAmount);

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

	void Swipe(ESwipeDirection Dir);

	UFUNCTION()
	void InitPlayerTimer();

	FTimerHandle InitPlayerTimerHandle;

	bool SkillLocked;

	FVector LastRightLocataion;

	FVector LastLeftLocation;

	//Touch
	bool Pressed;

	FVector2D PressedLocation;

	bool Swiped;
};
