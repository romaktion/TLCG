// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TLCGGameState.h"
#include "TLCGPlayerController.generated.h"


/**
 * 
 */
UCLASS()
class TLCG_API ATLCGPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGoToMainMenu(bool Win, const TArray<FLeaderBoard>& InLeaderBoard);

	UFUNCTION(BlueprintCallable, Category = "UserInterface")
	void PerformClientTravel(const FString& Path);

};
