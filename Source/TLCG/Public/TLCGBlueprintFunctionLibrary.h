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
	
public:
	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void StartBattle();

	UFUNCTION(BlueprintCallable, Category = "Battle")
	static void StopBattle();

	UFUNCTION(BlueprintCallable, Category = "Configuration")
	static void SetStringToConfig(const FString& Section, const FString& ValueName, const FString& Value);

	UFUNCTION(BlueprintPure, Category = "Configuration")
	static FString GetStringFromConfig(const FString& Section, const FString& ValueName, bool& IfFind);
};
