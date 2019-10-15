// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPawn.h"
#include "UnrealNetwork.h"
#include "Engine/CollisionProfile.h"
#include "TLCGMovement.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "TLCGPawnTrack.h"


// Sets default values
ATLCGPawn::ATLCGPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, TrackClass(nullptr)
, InitialTracksPoolSize(100)
, Killed(false)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	if (BoxComponent)
	{
		BoxComponent->InitBoxExtent(FVector(10.f, 10.f, 10.f));
		BoxComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

		BoxComponent->CanCharacterStepUpOn = ECB_No;
		BoxComponent->SetShouldUpdatePhysicsVolume(true);
		BoxComponent->SetCanEverAffectNavigation(false);
		BoxComponent->bDynamicObstacle = true;
		BoxComponent->SetGenerateOverlapEvents(false);
		RootComponent = BoxComponent;
	}

	TLCMovement = CreateDefaultSubobject<UTLCGMovement>("MovementComponent");
	if (TLCMovement)
	{
		TLCMovement->UpdatedComponent = BoxComponent;
	}

	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;

	SetReplicateMovement(false);
}

// Called when the game starts or when spawned
void ATLCGPawn::BeginPlay()
{
	Super::BeginPlay();
	
	if (TLCMovement)
	{
		TLCMovement->OnRotate.AddUniqueDynamic(this, &ATLCGPawn::OnRotate);
	}

	if (Role == ROLE_Authority)
	{
		if (BoxComponent)
			BoxComponent->OnComponentHit.AddUniqueDynamic(this, &ATLCGPawn::OnKilled);

		if (TLCMovement)
		{
			TLCMovement->OnComponentActivated.AddUniqueDynamic(this, &ATLCGPawn::OnMoveActivated);
			TLCMovement->OnComponentDeactivated.AddUniqueDynamic(this, &ATLCGPawn::OnMoveDeactivated);
		}

		auto World = GetWorld();

		if (World && TrackClass && InitialTracksPoolSize > 0 && BoxComponent)
		{
			TracksPool.Reserve(InitialTracksPoolSize);
			SpawnedTracks.Reserve(InitialTracksPoolSize);

			for (uint32 i = 0; i < InitialTracksPoolSize; i++)
			{
				FActorSpawnParameters ActorSpawnParameters;
				ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				auto SpawnedTrack = World->SpawnActor<ATLCGPawnTrack>(TrackClass, FTransform::Identity, ActorSpawnParameters);
				if (SpawnedTrack)
				{
					SpawnedTrack->MulticastEnable(false);
					SpawnedTrack->BoxComponent->SetBoxExtent(FVector(BoxComponent->GetScaledBoxExtent().Y, BoxComponent->GetScaledBoxExtent().Y, BoxComponent->GetScaledBoxExtent().Z));
					TracksPool.Push(SpawnedTrack);
				}
			}
		}
	}
}

// Called to bind functionality to input
void ATLCGPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("TurnLeft", IE_Pressed, this, &ATLCGPawn::TurnLeft);
	PlayerInputComponent->BindAction("TurnRight", IE_Pressed, this, &ATLCGPawn::TurnRight);
	PlayerInputComponent->BindAction("Skill", IE_Pressed, this, &ATLCGPawn::Skill);
}

void ATLCGPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (TLCMovement && TLCMovement->IsActive() && TLCMovement->GetRepData().Track)
	{
		auto Speed = TLCMovement->Velocity.Size();

		TLCMovement->GetRepData().Track->AddActorWorldOffset(TLCMovement->GetRepData().Track->GetActorForwardVector() * (Speed * 0.5f), true);
		TLCMovement->GetRepData().Track->SetActorScale3D(FVector(TLCMovement->GetRepData().Track->GetActorScale3D().X + Speed * 0.05f, TLCMovement->GetRepData().Track->GetActorScale3D().Y, TLCMovement->GetRepData().Track->GetActorScale3D().Z));
	}
}

void ATLCGPawn::StartBattle()
{
	if (Role == ROLE_Authority && TLCMovement)
		TLCMovement->Activate();
}

void ATLCGPawn::StopBattle()
{
	if (Role == ROLE_Authority &&  TLCMovement)
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
	{
		auto SpawnedTrack = SpawnTrack();

		TLCMovement->TurnLeft(SpawnedTrack);
		MulticastOnRotate();
	}
}

bool ATLCGPawn::ServerTurnLeft_Validate()
{
	return true;
}

void ATLCGPawn::ServerTurnLRight_Implementation()
{
	if (TLCMovement && TLCMovement->IsActive())
	{
		auto SpawnedTrack = SpawnTrack();

		TLCMovement->TurnRight(SpawnedTrack);
		MulticastOnRotate();
	}
}

bool ATLCGPawn::ServerTurnLRight_Validate()
{
	return true;
}

void ATLCGPawn::Skill()
{
	//
}

void ATLCGPawn::OnKilled(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (TLCMovement)
		TLCMovement->Deactivate();

	MulticastOnKilled();
}

void ATLCGPawn::OnMoveActivated(UActorComponent* Component, bool bReset)
{
	MulticastOnMoveActivated();
}

void ATLCGPawn::OnMoveDeactivated(UActorComponent* Component)
{
	if (TLCMovement)
	{
		TLCMovement->SetRepData(FRepData(GetActorLocation(), GetActorRotation().Yaw, nullptr));
	}

	if (BoxComponent)
		BoxComponent->MoveIgnoreActors.Empty();

	MulticastOnMoveDeactivated();
}

void ATLCGPawn::OnRotate(const FTransform& NewTransform, ATLCGPawnTrack* NewTrack, ATLCGPawnTrack* OldTrack)
{
	if (NewTrack)
	{
		NewTrack->SetActorLocation(NewTransform.GetLocation() + NewTransform.GetRotation().Vector() * -1.f * BoxComponent->GetScaledBoxExtent().Y);
		NewTrack->SetActorRotation(NewTransform.GetRotation());
		NewTrack->SetActorScale3D(FVector(0.0001f, NewTrack->GetActorScale3D().Y, NewTrack->GetActorScale3D().Z));
		NewTrack->SetActorEnableCollision(Role == ROLE_Authority ? true : false);
		NewTrack->SetActorHiddenInGame(false);
	}

	if (OldTrack && Role == ROLE_Authority)
	{
		OldTrack->MulticastSetTransform(OldTrack->GetActorTransform());

		OldTrack->BoxComponent->AddWorldOffset(OldTrack->GetActorForwardVector() * -1.f);

		TArray<USceneComponent*> Childs;

		OldTrack->BoxComponent->GetChildrenComponents(false, Childs);

		for (auto C : Childs)
		{
			if (C)
			{
				C->AddWorldOffset(OldTrack->GetActorForwardVector() * 1.f);
			}
		}

		if (BoxComponent)
			BoxComponent->MoveIgnoreActors.Remove(OldTrack);
	}
}

ATLCGPawnTrack* ATLCGPawn::SpawnTrack()
{
	if (Role == ROLE_Authority)
	{
		auto SpawnedTrack = TracksPool.Pop();
		if (SpawnedTrack)
		{
			if (BoxComponent)
				BoxComponent->MoveIgnoreActors.Add(SpawnedTrack);

			SpawnedTracks.Push(SpawnedTrack);
		}

		return SpawnedTrack;
	}
	else
	{
		return nullptr;
	}
}

void ATLCGPawn::MulticastOnRotate_Implementation()
{
	K2_OnRotate();
}

void ATLCGPawn::MulticastOnMoveActivated_Implementation()
{
	auto SpawnedTrack = SpawnTrack();

	if (TLCMovement)
	{
		TLCMovement->SetRepData(FRepData(GetActorLocation(), GetActorRotation().Yaw, SpawnedTrack));
	}

	K2_OnMoveActivated();
}

void ATLCGPawn::MulticastOnMoveDeactivated_Implementation()
{
	K2_OnMoveDeactivated();
}

void ATLCGPawn::MulticastOnKilled_Implementation()
{
	K2_OnKilled();
}
