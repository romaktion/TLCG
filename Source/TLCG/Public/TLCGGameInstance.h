// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "TLCGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TLCG_API UTLCGGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FString, int32> LeaderBoard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool Win;

	int32 CountDuplicates;
};
