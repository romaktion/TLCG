// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPawn.h"
#include "UnrealNetwork.h"
#include "Engine/CollisionProfile.h"
#include "TLCGMovement.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "TLCGPawnTrack.h"
#include "TLCGPlayerController.h"
#include "TLCGPlayerState.h"
#include "TLCGGameState.h"


// Sets default values
ATLCGPawn::ATLCGPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, TrackClass(nullptr)
, InitialTracksPoolSize(100)
, StartTransform(FTransform::Identity)
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
	
	auto World = GetWorld();
	if (!World)
		return;

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

	auto PC = Cast<ATLCGPlayerController>(GetController());
	if (PC && PC->IsLocalPlayerController())
	{
		PC->ServerAllowStartRound();
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

void ATLCGPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	auto PC = Cast<ATLCGPlayerController>(NewController);
	if (PC && PC->IsLocalPlayerController())
	{
		PC->ServerAllowStartRound();
	}
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
	{
		if (SpawnedTracks.Num() > 0)
		{
			for (auto &ST : SpawnedTracks)
			{
				ST->MulticastEnable(false);
				TracksPool.Push(ST);
				ST = nullptr;
			}

			SpawnedTracks.RemoveAll([](auto ST) { return !ST; });
		}
		
		if (StartTransform.Equals(FTransform::Identity))
		{
			StartTransform = GetActorTransform();
		}

		TLCMovement->Activate();
	}
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
	if (TLCMovement && TLCMovement->IsActive())
		ServerSkill();
}

void ATLCGPawn::MulticastSkill_Implementation()
{
	K2_ActivateSkill();
}

void ATLCGPawn::ServerSkill_Implementation()
{
	MulticastSkill();
}

bool ATLCGPawn::ServerSkill_Validate()
{
	return true;
}

void ATLCGPawn::OnKilled(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	auto PS = Cast<ATLCGPlayerState>(GetPlayerState());

	if (!PS)
		return;

	if (PS->GetPlayerState() == EPlayerStateEnum::PS_Killed)
		return;

	PS->SetPlayerState(EPlayerStateEnum::PS_Killed);

	if (TLCMovement)
		TLCMovement->Deactivate();

	if (BoxComponent)
		BoxComponent->MoveIgnoreActors.Empty();

	PS->OnPlayerKilled.Broadcast(this);

	MulticastOnKilled();
}

void ATLCGPawn::OnMoveActivated(UActorComponent* Component, bool bReset)
{
	if (TLCMovement)
	{
		TLCMovement->SetRepData(FRepData(GetActorLocation(), GetActorRotation().Yaw, SpawnTrack()));
	}

	MulticastOnMoveActivated();
}

void ATLCGPawn::OnMoveDeactivated(UActorComponent* Component)
{
	if (TLCMovement)
	{
		TLCMovement->SetRepData(FRepData(GetActorLocation(), GetActorRotation().Yaw, nullptr));
	}

	MulticastOnMoveDeactivated();
}

void ATLCGPawn::OnRotate(const FTransform& NewTransform, ATLCGPawnTrack* NewTrack, ATLCGPawnTrack* OldTrack)
{
	if (NewTrack)
	{
		NewTrack->SetActorLocation(NewTransform.GetLocation() + NewTransform.GetRotation().Vector() * -1.f * BoxComponent->GetScaledBoxExtent().Y);
		NewTrack->SetActorRotation(NewTransform.GetRotation());
		NewTrack->SetActorScale3D(FVector(0.0001f, NewTrack->GetActorScale3D().Y, NewTrack->GetActorScale3D().Z));
		NewTrack->MulticastEnable_Implementation(true);
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

void ATLCGPawn::MulticastOnRespawn_Implementation()
{
	if (TLCMovement && Role == ROLE_Authority)
	{
		TLCMovement->SetRepData(FRepData(StartTransform.GetLocation(), StartTransform.GetRotation().Rotator().Yaw, nullptr));
	}

	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);

	K2_OnRespawn();
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
	K2_OnMoveActivated();
}

void ATLCGPawn::MulticastOnMoveDeactivated_Implementation()
{
	K2_OnMoveDeactivated();
}

void ATLCGPawn::MulticastOnKilled_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	K2_OnKilled();
}

void ATLCGPawn::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATLCGPawn, Color);
}
