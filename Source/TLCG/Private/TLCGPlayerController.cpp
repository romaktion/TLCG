// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TLCGGameInstance.h"
#include "TLCGPlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "TLCGGameState.h"

ATLCGPlayerController::ATLCGPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, StartSpot(nullptr)
{

}

void ATLCGPlayerController::MulticastGoToMainMenu_Implementation(bool Win, const TArray<FLeaderBoard>& InLeaderBoard)
{
	TMap<FString, int32> LB_Map;

	for (auto &LB : InLeaderBoard)
	{
		LB_Map.Add(TTuple<FString, int32>(LB.PlayerName, LB.Score));
	}

	if (IsLocalController())
	{
		auto GI = Cast<UTLCGGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->LeaderBoard = LB_Map;
			GI->Win = Win;
		}

		UGameplayStatics::OpenLevel(this, "MainMenu");
	}
}

void ATLCGPlayerController::PerformClientTravel(const FString& Path)
{
	ClientTravel(Path, ETravelType::TRAVEL_Absolute);
}

void ATLCGPlayerController::ServerAllowStartRound_Implementation()
{
	auto World = GetWorld();
	if (!World)
		return;
	auto GS = World->GetGameState<ATLCGGameState>();
	if (!GS)
		return;
	GS->CountReadyToPlayPlayers++;
}

bool ATLCGPlayerController::ServerAllowStartRound_Validate()
{
	return true;
}

void ATLCGPlayerController::ServerSpawnPawn_Implementation(const FString& ClassPath)
{
	if (!StartSpot)
		return;
	auto World = GetWorld();
	if (!World)
		return;
	auto GM = World->GetAuthGameMode();
	if (!GM)
		return;
	auto GS = World->GetGameState<ATLCGGameState>();
	if (!GS)
		return;
	auto PS = GetPlayerState<ATLCGPlayerState>();
	if (!PS)
		return;

	if (GetPawn())
		GetPawn()->Destroy();

	UObject* ClassPackage = ANY_PACKAGE;
	PS->PlayerPawnClass = (UClass*)StaticLoadObject(UClass::StaticClass(), nullptr, *ClassPath);
	if (!PS->PlayerData.IsValid())
		PS->PlayerData = GS->GetAvailableColor();

	GM->RestartPlayerAtPlayerStart(this, StartSpot);
}

bool ATLCGPlayerController::ServerSpawnPawn_Validate(const FString& ClassPath)
{
	return true;
}
