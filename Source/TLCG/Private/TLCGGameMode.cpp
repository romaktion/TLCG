// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGGameMode.h"
#include "TLCGGameState.h"
#include "TLCGPlayerState.h"
#include "TLCGPlayerController.h"
#include "TLCGPawn.h"


ATLCGGameMode::ATLCGGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GameStateClass = ATLCGGameState::StaticClass();
	PlayerStateClass = ATLCGPlayerState::StaticClass();
	PlayerControllerClass = ATLCGPlayerController::StaticClass();
	DefaultPawnClass = ATLCGPawn::StaticClass();
}