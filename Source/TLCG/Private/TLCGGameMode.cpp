// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGGameMode.h"
#include "TLCGGameState.h"
#include "TLCGPlayerState.h"
#include "TLCGPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TLCGPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"


ATLCGGameMode::ATLCGGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, PlayersToStart(4)
{
	GameStateClass = ATLCGGameState::StaticClass();
	PlayerStateClass = ATLCGPlayerState::StaticClass();
	PlayerControllerClass = ATLCGPlayerController::StaticClass();
	DefaultPawnClass = ATLCGPawn::StaticClass();
}

UClass* ATLCGGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	ATLCGPlayerState* PlayerState = nullptr;

	if (InController)
		PlayerState = Cast<ATLCGPlayerState>(InController->PlayerState);

	if (PlayerState && PlayerState->PlayerPawnClass != nullptr)
		return PlayerState->PlayerPawnClass;

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

FString ATLCGGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal /*= TEXT("")*/)
{
	if (!NewPlayerController)
		return "";

	PlayerControllers.Add(NewPlayerController);

	FString ClassName = *UGameplayStatics::ParseOption(Options, TEXT("PlayerPawnClass"));

	UClass* PlayerPawnClass = nullptr;

	if (!ClassName.IsEmpty())
	{
		UObject* ClassPackage = ANY_PACKAGE;
		PlayerPawnClass = (UClass*)StaticLoadObject(UClass::StaticClass(), nullptr, *ClassName);
	}
	FString OptionsCopy = Options;

	FString PlayerName = *UGameplayStatics::ParseOption(Options, TEXT("PlayerName"));
	if (!PlayerName.IsEmpty())
	{
		auto grab = true;
		FString Res;
		FURL Url;

		while (grab)
		{
			grab = UGameplayStatics::GrabOption(OptionsCopy, Res);
			if (grab && !Res.Contains("Name="))
				Url.AddOption(*Res);
		}

		for (auto &O : Url.Op)
		{
			OptionsCopy.Append(FString::Printf(TEXT("?%s"), *O));
		}

		OptionsCopy.Append(FString::Printf(TEXT("?Name=%s"), *PlayerName));
	}

	auto PlayerState = Cast<ATLCGPlayerState>(NewPlayerController->PlayerState);
	if (PlayerState != nullptr)
	{
		PlayerState->OnPlayerKilled.AddUniqueDynamic(this, &ATLCGGameMode::OnKilled);
		PlayerState->PlayerPawnClass = PlayerPawnClass;
		PlayerState->PlayerNumber = PlayerControllers.Num();
	}

#if UE_BUILD_SHIPPING
	PlayersToStart = 4;
#endif

	auto GS = GetGameState<ATLCGGameState>();
	if (GS)
	{
		if (PlayerControllers.Num() == PlayersToStart)
		{
			GS->StartGame(PlayerControllers);
		}

		GS->PlayerStates.Add(PlayerState);
	}

	return Super::InitNewPlayer(NewPlayerController, UniqueId, OptionsCopy, Portal);
}

AActor* ATLCGGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	for (TActorIterator<APlayerStart> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		auto PS = Cast<ATLCGPlayerState>(Player->PlayerState);
		if (PS)
		{
			if (ActorItr && ActorItr->ActorHasTag(FName(*FString::FromInt(PS->PlayerNumber))))
			{
				return *ActorItr;
			}
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void ATLCGGameMode::OnKilled(AActor* PlayerPawn)
{
	uint32 CountPlayers = 0;
	uint32 CountAlivePlayers = 0;

	auto World = GetWorld();
	if (!World)
		return;

	APlayerController* AlivePlayer = nullptr;

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (Iterator->IsValid())
		{
			CountPlayers++;

			auto PlayerState = Cast<ATLCGPlayerState>(Iterator->Get()->PlayerState);
			if (PlayerState && PlayerState->GetPlayerState() == EPlayerStateEnum::PS_Alive)
			{
				CountAlivePlayers++;
				AlivePlayer = Iterator->Get();
			}
		}
	}

	ensureMsgf(CountPlayers == PlayerControllers.Num(), TEXT("No alive players left after round is over!"));

	if (CountAlivePlayers <= 1)
	{
		auto GS = Cast<ATLCGGameState>(World->GetGameState());
		if (GS)
		{
			GS->PerformRoundOver(AlivePlayer);
		}
	}
}
