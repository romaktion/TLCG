// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TLCGGameInstance.h"
#include "TLCGPlayerState.h"
#include "TimerManager.h"

ATLCGPlayerController::ATLCGPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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
