// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPlayerState.h"
#include "TLCGPawn.h"
#include "UnrealNetwork.h"
#include "TLCGGameInstance.h"

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
