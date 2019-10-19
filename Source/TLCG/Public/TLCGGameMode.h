// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "TLCGGameMode.generated.h"


/**
 * 
 */
UCLASS()
class TLCG_API ATLCGGameMode : public AGameModeBase
{
	GENERATED_UCLASS_BODY()
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual FString	InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	UFUNCTION()
	void OnKilled(AActor* PlayerPawn);

	TArray<APlayerController*> PlayerControllers;

	UPROPERTY(EditDefaultsOnly)
	int32 PlayersToStart;

	TArray<FString> PlayerNames;

	uint32 CountPlayerNamesDuplicates;
};
