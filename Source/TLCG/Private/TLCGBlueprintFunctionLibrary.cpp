// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGBlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework//PlayerController.h"
#include "TLCGBattleInterface.h"
#include "ConfigCacheIni.h"

void UTLCGBlueprintFunctionLibrary::StartBattle()
{
	auto World = GWorld;
	if (!World)
		return;

	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->IsValid())
		{
			auto Object = Cast<ITLCGBattleInterface>(It->Get()->GetPawn());
			if (Object)
			{
				Object->StartBattle();
			}
		}
	}
}


void UTLCGBlueprintFunctionLibrary::StopBattle()
{
	auto World = GWorld;
	if (!World)
		return;

	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->IsValid())
		{
			auto Object = Cast<ITLCGBattleInterface>(It->Get()->GetPawn());
			if (Object)
			{
				Object->StopBattle();
			}
		}
	}
}

void UTLCGBlueprintFunctionLibrary::SetStringToConfig(const FString& Section, const FString& ValueName, const FString& Value)
{
	if (!GConfig)
		return;

	GConfig->SetString(*Section, *ValueName, *Value, GGameIni);
	GConfig->Flush(false, GGameIni);
}

FString UTLCGBlueprintFunctionLibrary::GetStringFromConfig(const FString& Section, const FString& ValueName, bool& IfFind)
{
	if (!GConfig)
		return FString("");

	FString NewValue = "";
	IfFind = GConfig->GetString(*Section, *ValueName, NewValue, GGameIni);
	return NewValue;
}
