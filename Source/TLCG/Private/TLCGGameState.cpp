// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGGameState.h"
#include "UnrealNetwork.h"
#include "TLCGBlueprintFunctionLibrary.h"
#include "TLCGPlayerController.h"
#include "TLCGPlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "TLCGGameInstance.h"
#include "TLCGGameMode.h"
#include "TLCGPawn.h"

ATLCGGameState::ATLCGGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, ScoreToWin(3)
, RoundNumber(0)
, CountReadyToPlayPlayers(0)
, GameState(EGameStateEnum::GS_GameNotStarted)
, RoundStartDelay(3.f)
{

}

void ATLCGGameState::BeginPlay()
{
	Super::BeginPlay();

	ColorsReserve = Colors;
}

void ATLCGGameState::PerformRoundOver(APlayerController* AlivePlayer)
{
	GameState = EGameStateEnum::GS_RoundOverPreparation;

	UTLCGBlueprintFunctionLibrary::StopBattle();

	if (!AlivePlayer)
		return;

	auto PS = AlivePlayer->GetPlayerState<ATLCGPlayerState>();
	if (!PS)
		return;

	PS->Score += 1;

	if (PS->Score >= ScoreToWin)
	{
		GameOver(AlivePlayer);
	}
	else
	{
		//New round
		GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &ATLCGGameState::StartRound, RoundStartDelay);
	}

	auto World = GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (Iterator->IsValid())
			{
				auto PC = Cast<ATLCGPlayerController>(Iterator->Get());
				if (PC)
				{
					PS = PC->GetPlayerState<ATLCGPlayerState>();
					if (PS)
					{
						MulticastOnRoundOver(PS->PlayerNumber, PS->Score);
					}
				}
			}
		}
	}

	K2_OnRoundOver();
}

void ATLCGGameState::GameOver(APlayerController* Winner)
{
	GameState = EGameStateEnum::GS_GameOverPreparation;

	GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &ATLCGGameState::PerformGameOver, RoundStartDelay);
}

void ATLCGGameState::PerformGameOver()
{
	GameState = EGameStateEnum::GS_GameOver;

	auto World = GetWorld();
	if (!World)
		return;

	//Make individual for each name which duplicate
	uint32 CountDuplicates = 0;

	TMap<FString, int32> LB_Map;

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			auto PS = Iterator->Get()->GetPlayerState<ATLCGPlayerState>();
			if (PS)
			{
				auto PN = PS->GetPlayerName();
				/*auto Finded = LB_Map.Find(PN);
				if (Finded)
				{
					CountDuplicates++;
					PN.AppendInt(CountDuplicates);
				}*/

				LB_Map.Add(PN, PS->Score);
				LB_Map.ValueSort([](int32 A, int32 B) { return A > B; });
			}
		}
	}

	//For replication
	TArray<FLeaderBoard> LB_Struct;

	for (auto It = LB_Map.CreateConstIterator(); It; ++It)
	{
		LB_Struct.Add(FLeaderBoard(It->Key, It->Value));
	}

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			auto PC = Cast<ATLCGPlayerController>(Iterator->Get());
			if (PC)
			{
				bool Win = false;

				auto PS = PC->GetPlayerState<ATLCGPlayerState>();
				if (PS)
				{
					Win = PS->Score >= ScoreToWin;
				}

				PC->MulticastGoToMainMenu(Win, LB_Struct);
			}
		}
	}
}

void ATLCGGameState::StartGame(TArray<APlayerController*> InPlayerControllers)
{
	ensureMsgf(InPlayerControllers.Num() > 0, TEXT("No player controllers are here!"));

	PlayerControllers = InPlayerControllers;

	StartRound();
}

void ATLCGGameState::StartRound()
{
	auto World = GetWorld();
	if (!World)
		return;

	GameState = EGameStateEnum::GS_RoundPreparation;

	if (RoundNumber > 0)
	{
		CountReadyToPlayPlayers = 0;
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (Iterator->IsValid() && Iterator->Get()->GetPawn())
			{
				auto Pawn = Cast<ATLCGPawn>(Iterator->Get()->GetPawn());
				if (Pawn)
				{
					Pawn->ClearTracks();
				}
				Iterator->Get()->GetPawn()->Destroy();
			}
		}

		MulticastOnRoundPreparation();
	}

	GetWorldTimerManager().SetTimer(TryStartRoundTimerHandle, this, &ATLCGGameState::TryPerformStartRound, World->GetDeltaSeconds(), true);
}

void ATLCGGameState::MulticastOnRoundStartTmer_Implementation()
{
	OnRoundStartTimer.Broadcast(RoundStartDelay);
}

FPlayerData ATLCGGameState::GetAvailableColor() const
{
	ensureMsgf(Colors.Num() > 0, TEXT("Colors array are empty!"));

	FPlayerData Color;

	if (Colors.Num() > 0)
	{
		auto Rnd = FMath::RandRange(0, Colors.Num() - 1);
		Color = Colors[Rnd];
		Colors.RemoveAt(Rnd);
	}

	return Color;
}

void ATLCGGameState::MulticastOnRoundPreparation_Implementation()
{
	OnRoundPreparation.Broadcast();
}

void ATLCGGameState::MulticastOnRoundStart_Implementation(int32 NewRoundNumber)
{
	OnRoundStart.Broadcast(NewRoundNumber);
}

void ATLCGGameState::MulticastOnInitPlayer_Implementation(APlayerState* NewPlayer)
{
	OnInitNewPlayer.Broadcast(NewPlayer);
}

void ATLCGGameState::MulticastOnRoundOver_Implementation(int32 InPlayerNumber, float InScore)
{
	OnRoundOver.Broadcast(InPlayerNumber, InScore);
}

void ATLCGGameState::TryPerformStartRound()
{
	auto World = GetWorld();
	if (!World)
		return;

	auto GM = World->GetAuthGameMode<ATLCGGameMode>();
	if (!GM)
		return;

	if (CountReadyToPlayPlayers == GM->GetPlayersToStart())
	{
		GetWorldTimerManager().ClearTimer(TryStartRoundTimerHandle);
		GetWorldTimerManager().SetTimer(StartRoundTimerHandle, this, &ATLCGGameState::PerformStartRound, RoundStartDelay);
		MulticastOnRoundStartTmer();
	}
}

void ATLCGGameState::PerformStartRound()
{
	auto World = GetWorld();
	if (!World)
		return;

	RoundNumber++;

	uint32 CountPlayers = 0;

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			CountPlayers++;

			//Respawn pawns
			if (RoundNumber > 1)
			{
				auto PlayerState = Cast<ATLCGPlayerState>(Iterator->Get()->PlayerState);
				if (PlayerState)
				{

					PlayerState->SetPlayerState(EPlayerStateEnum::PS_Alive);
				}

				auto Pawn = Cast<ATLCGPawn>(Iterator->Get()->GetPawn());
				if (Pawn)
				{
					Pawn->MulticastOnRespawn();
				}
			}

		}
	}

	ensureMsgf(PlayerControllers.Num() == CountPlayers, TEXT("We lost player(s)?!"));

	GameState = EGameStateEnum::GS_RoundInProgress;

	UTLCGBlueprintFunctionLibrary::StartBattle();

	MulticastOnRoundStart(RoundNumber);
}

void ATLCGGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATLCGGameState, GameState);
	DOREPLIFETIME(ATLCGGameState, PlayerStates);
}
