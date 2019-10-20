// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPlayerState.h"
#include "TLCGPawn.h"
#include "UnrealNetwork.h"
#include "TLCGGameInstance.h"
#include "TLCGGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATLCGPlayerState::ATLCGPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, PlayerPawnClass(nullptr)
, PlayerNumber(-1)
, PlayerState(EPlayerStateEnum::PS_Alive)
{
	
}

void ATLCGPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATLCGPlayerState, PlayerState);
	DOREPLIFETIME(ATLCGPlayerState, PlayerNumber);
	DOREPLIFETIME_CONDITION(ATLCGPlayerState, PlayerPawnClass, COND_OwnerOnly);
}

EPlayerStateEnum ATLCGPlayerState::GetPlayerState() const
{
	return PlayerState;
}

void ATLCGPlayerState::SetPlayerState(EPlayerStateEnum NewPlayerState)
{
	if (Role == ROLE_Authority)
		PlayerState = NewPlayerState;
}

void ATLCGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	auto World = GetWorld();
	if (!World)
		return;

	GetWorldTimerManager().SetTimer(OnInitPlayerTimerHandle, this, &ATLCGPlayerState::OnInitPlayerTimer, World->GetTimeSeconds() ? World->GetTimeSeconds() : 0.1f, true);
}

void ATLCGPlayerState::OnInitPlayerTimer()
{
	if (PlayerNumber > -1 && !GetPlayerName().IsEmpty())
	{
		auto World = GetWorld();
		if (!World)
			return;

		auto GS = World->GetGameState<ATLCGGameState>();
		if (!GS)
			return;

		if (!GetPawn())
			return;

		if (Role < ROLE_Authority)
			GS->MulticastOnInitPlayer_Implementation(this);
		else
			GS->MulticastOnInitPlayer(this);
		
		GetWorldTimerManager().ClearTimer(OnInitPlayerTimerHandle);
	}
}
