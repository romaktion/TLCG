// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameStateBase.h"
#include "TLCGPawnTrack.h"
#include "TLCGGameState.generated.h"

class APlayerState;
class ATLCGPawn;

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_TwoParams(FOnRoundOverSignature, ATLCGGameState, OnRoundOver, int32, PlayerNumber, float, Score);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnInitNewPlayerSignature, ATLCGGameState, OnInitNewPlayer, APlayerState*, NewPlayer);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnRoundStartSignature, ATLCGGameState, OnRoundStart, int32, NewRoundNumber);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE(FOnRoundPreparationSignature, ATLCGGameState, OnRoundPreparation);
DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FOnRoundStartTimerSignature, ATLCGGameState, OnRoundStartTimer, int32, Delay);


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

USTRUCT(BlueprintType)
struct FPlayerData
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	FLinearColor Color;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATLCGPawnTrack> PawnTrackClass;

	FPlayerData()
	{
		Color = FLinearColor();
		PawnTrackClass = nullptr;
		Valid = false;
	}

	FPlayerData(const FLinearColor& InColor, TSubclassOf<ATLCGPawnTrack> InPawnTrackClass)
	{
		Color = InColor;
		PawnTrackClass = InPawnTrackClass;
		Valid = true;
	}

	FPlayerData(const FPlayerData& NewPlayerData)
	{
		Color = NewPlayerData.Color;
		PawnTrackClass = NewPlayerData.PawnTrackClass;
		Valid = true;
	}

	FPlayerData& operator = (const FPlayerData& NewPlayerData)
	{
		Color = NewPlayerData.Color;
		PawnTrackClass = NewPlayerData.PawnTrackClass;
		Valid = true;
		return *this;
	}

	bool IsValid() const { return Valid; }

private:
	bool Valid;
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

protected:
	void BeginPlay() override;

public:
	void PerformRoundOver(APlayerController* AlivePlayer);

	void StartGame(TArray<APlayerController*> InPlayerControllers);

	UFUNCTION()
	void StartRound();

	UFUNCTION(BlueprintImplementableEvent, Category = "TLCHGame", DisplayName = "OnRoundOver")
	void K2_OnRoundOver();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRoundOver(int32 InPlayerNumber, float InScore);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRoundStart(int32 NewRoundNumber);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRoundPreparation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnRoundStartTmer();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnInitPlayer(APlayerState* NewPlayer);

	FPlayerData GetAvailableColor() const;

	EGameStateEnum GetGameState() const { return GameState; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ScoreToWin;

	UPROPERTY(BlueprintAssignable, Category = "TLCHGame")
	FOnRoundOverSignature OnRoundOver;

	UPROPERTY(BlueprintAssignable, Category = "TLCHGame")
	FOnRoundStartSignature OnRoundStart;

	UPROPERTY(BlueprintAssignable, Category = "TLCHGame")
	FOnInitNewPlayerSignature OnInitNewPlayer;

	UPROPERTY(BlueprintAssignable, Category = "TLCHGame")
	FOnRoundPreparationSignature OnRoundPreparation;

	UPROPERTY(BlueprintAssignable, Category = "TLCHGame")
	FOnRoundStartTimerSignature OnRoundStartTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<APlayerController*> PlayerControllers;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<APlayerState*> PlayerStates;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 RoundNumber;

	uint32 CountReadyToPlayPlayers;

private:
	UFUNCTION()
	void TryPerformStartRound();

	UFUNCTION()
	void PerformStartRound();

	void GameOver(APlayerController* Winner);

	UFUNCTION()
	void PerformGameOver();

	UPROPERTY(Replicated)
	EGameStateEnum GameState;

	FTimerHandle TryStartRoundTimerHandle;

	FTimerHandle StartRoundTimerHandle;

	FTimerHandle GameOverTimerHandle;

	UPROPERTY(EditDefaultsOnly)
	float RoundStartDelay;

	UPROPERTY(EditDefaultsOnly)
	mutable TArray<FPlayerData> Colors;

	TArray<FPlayerData> ColorsReserve;
};
