// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "TLCGGameState.h"
#include "TLCGPlayerState.generated.h"

class ATLCGPawn;

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnPlayerKilledSignature, ATLCGPlayerState, OnPlayerKilled, AActor*, PlayerPawn);


UENUM(BlueprintType)
enum class EPlayerStateEnum : uint8
{
	PS_Alive 	UMETA(DisplayName = "Alive"),
	PS_Killed 	UMETA(DisplayName = "Killed")
};


/**
 * 
 */
UCLASS()
class TLCG_API ATLCGPlayerState : public APlayerState
{
	GENERATED_UCLASS_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "PlayerState")
	EPlayerStateEnum GetPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetPlayerState(EPlayerStateEnum NewPlayerState);

	UPROPERTY(Replicated)
	TSubclassOf<ATLCGPawn> PlayerPawnClass;

	UPROPERTY(Replicated)
	FPlayerData PlayerData;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerNumber;

	UPROPERTY(BlueprintAssignable, Category = "TLCGPawn")
	FOnPlayerKilledSignature OnPlayerKilled;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnInitPlayerTimer();

	UPROPERTY(Replicated)
	EPlayerStateEnum PlayerState;

	FTimerHandle OnInitPlayerTimerHandle;
};
