// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPawn.h"
#include "UnrealNetwork.h"
#include "Engine/CollisionProfile.h"
#include "TLCGMovement.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"


// Sets default values
ATLCGPawn::ATLCGPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	SphereComponent->InitSphereRadius(10.f);
	SphereComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	SphereComponent->CanCharacterStepUpOn = ECB_No;
	SphereComponent->SetShouldUpdatePhysicsVolume(true);
	SphereComponent->SetCanEverAffectNavigation(false);
	SphereComponent->bDynamicObstacle = true;
	SphereComponent->SetGenerateOverlapEvents(false);
	RootComponent = SphereComponent;

	TLCMovement = CreateDefaultSubobject<UTLCGMovement>("MovementComponent");
	if (TLCMovement)
	{
		TLCMovement->UpdatedComponent = SphereComponent;
	}

	SetReplicateMovement(false);
}

// Called when the game starts or when spawned
void ATLCGPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void ATLCGPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("TurnLeft", IE_Pressed, this, &ATLCGPawn::TurnLeft);
	PlayerInputComponent->BindAction("TurnRight", IE_Pressed, this, &ATLCGPawn::TurnRight);
	PlayerInputComponent->BindAction("Skill", IE_Pressed, this, &ATLCGPawn::Skill);
}

void ATLCGPawn::StartBattle()
{
	if (TLCMovement)
		TLCMovement->Activate();
}

void ATLCGPawn::StopBattle()
{
	if (TLCMovement)
		TLCMovement->Deactivate();
}

void ATLCGPawn::TurnLeft()
{
	ServerTurnLeft();
}

void ATLCGPawn::TurnRight()
{
	ServerTurnLRight();
}

void ATLCGPawn::ServerTurnLeft_Implementation()
{
	if (TLCMovement && TLCMovement->IsActive())
		AddActorWorldRotation(FQuat(FRotator(0.f, -90.f, 0.f)));
}

bool ATLCGPawn::ServerTurnLeft_Validate()
{
	return true;
}

void ATLCGPawn::ServerTurnLRight_Implementation()
{
	if (TLCMovement && TLCMovement->IsActive())
		AddActorWorldRotation(FQuat(FRotator(0.f, 90.f, 0.f)));
}

bool ATLCGPawn::ServerTurnLRight_Validate()
{
	return true;
}

void ATLCGPawn::Skill()
{
	//
}
