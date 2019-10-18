// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameStateBase.h"
#include "TLCGGameState.generated.h"

class APlayerState;
class ATLCGPawn;

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FOnRoundOverSignature, ATLCGGameState, OnRoundOver, int32, PlayerNumber, float, Score);


UENUM(BlueprintType)
enum class EGameStateEnum : uint8
{
	GS_GameNotStarted			UMETA(DisplayName = "GameNotStarted"),
	GS_RoundPreparation			UMETA(DisplayName = "RoundPreparation"),
	GS_RoundInProgress 			UMETA(DisplayName = "RoundInProgress"),
	GS_RoundOverPreparation		UMETA(DisplayName = "RoundOverPreparation"),
	GS_RoundIsOver				UMETA(DisplayName = "RoundIsOver"),
	GS_GameOverPreparation		UMETA(DisplayName = "GameOverPreparation"),
	GS_GameOver 				UMETA(DisplayName = "GameOver")
};


USTRUCT()
struct FLeaderBoard
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	float Score;

	FLeaderBoard()
	{
		PlayerName = "";
		Score = 0.f;
	}

	FLeaderBoard(const FString& InPlayerName, float InScore)
	{
		PlayerName = InPlayerName;
		Score = InScore;
	}
};

/**
 * 
 */
UCLASS()
class TLCG_API ATLCGGameState : public AGameStateBase
{
	GENERATED_UCLASS_BODY()

public:
	void PerformRoundOver(APlayerController* AlivePlayer);

	void StartGame(TArray<APlayerController*> InPlayerControllers);

	void StartRound();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCHGame", DisplayName = "OnRoundOver")
	void K2_OnRoundOver();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRoundOver(int32 InPlayerNumber, float InScore);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ScoreToWin;

	UPROPERTY(BlueprintAssignable, Category = "TLCHGame")
	FOnRoundOverSignature OnRoundOver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<APlayerController*> PlayerControllers;

private:
	UFUNCTION()
	void PerformStartRound();

	void GameOver(APlayerController* Winner);

	UFUNCTION()
	void PerformGameOver();

	UPROPERTY(Replicated)
	EGameStateEnum GameState;

	FTimerHandle StartRoundTimerHandle;

	FTimerHandle GameOverTimerHandle;
};
