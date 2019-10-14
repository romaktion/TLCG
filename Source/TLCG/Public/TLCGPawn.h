// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "TLCGBattleInterface.h"
#include "TLCGPawn.generated.h"

class UTLCGMovement;
class USphereComponent;

UCLASS(Abstract)
class TLCG_API ATLCGPawn : public APawn, public ITLCGBattleInterface
{
	GENERATED_UCLASS_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void StartBattle() override;

	virtual void StopBattle() override;

	UPROPERTY(Category="TLCGPawn", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	USphereComponent* SphereComponent;

	UPROPERTY(Category="TLCGPawn", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UTLCGMovement* TLCMovement;

private:
	UFUNCTION()
	void TurnLeft();

	UFUNCTION()
	void TurnRight();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnLeft();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTurnLRight();

	UFUNCTION()
	void Skill();
	
};
