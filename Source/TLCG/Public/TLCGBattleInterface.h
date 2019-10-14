// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TLCGBattleInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UTLCGBattleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TLCG_API ITLCGBattleInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION()
	virtual void StartBattle();

	UFUNCTION()
	virtual void StopBattle();

};
