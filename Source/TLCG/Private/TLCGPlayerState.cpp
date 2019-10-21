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

void ATLCGPlayerState::InitPlayer()
{
	auto World = GetWorld();
	if (!World)
		return;

	GetWorldTimerManager().SetTimer(OnInitPlayerTimerHandle, this, &ATLCGPlayerState::OnInitPlayerTimer, World->IsEditorWorld() ? 0.01f : World->GetDeltaSeconds());
}

void ATLCGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	auto World = GetWorld();
	if (!World)
		return;
}

void ATLCGPlayerState::OnInitPlayerTimer()
{
	auto World = GetWorld();
	auto GS = World->GetGameState<ATLCGGameState>();

	if (PlayerNumber > -1 && !GetPlayerName().IsEmpty() && World && GS && GetPawn())
	{
		if (Role < ROLE_Authority)
			GS->MulticastOnInitPlayer_Implementation(this);
		else
			GS->MulticastOnInitPlayer(this);
	}
	else
	{
		GetWorldTimerManager().SetTimer(OnInitPlayerTimerHandle, this, &ATLCGPlayerState::OnInitPlayerTimer, World->IsEditorWorld() ? 0.01f : World->GetDeltaSeconds());
	}
}
