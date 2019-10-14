// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGBlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework//PlayerController.h"
#include "TLCGBattleInterface.h"

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