// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TLCGBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TLCG_API UTLCGBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void StartBattle();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void StopBattle();
};
