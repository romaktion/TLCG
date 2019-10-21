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
, DisableSpawnTracks(false)
, AvaibleSkillsAmount(3)
, SkillLocked(false)
, Pressed(false)
, Swiped(0)
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
	PlayerInputComponent->BindAction("Touch1", IE_Pressed, this, &ATLCGPawn::Touch);
	PlayerInputComponent->BindAction("Touch1", IE_Released, this, &ATLCGPawn::UnTouch);
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

	if (Pressed && !Swiped)
	{
		auto PC = GetController<APlayerController>();
		if (PC)
		{
			FVector2D Location;
			bool IsCurrentlyPressed;

			PC->GetInputTouchState(ETouchIndex::Touch1, Location.X, Location.Y, IsCurrentlyPressed);
			if ((Location - PressedLocation).Size() > 7.f)
			{
				Swiped = true;
				if (FMath::Abs((PressedLocation - Location).X) > FMath::Abs((PressedLocation - Location).Y))
				{
					if ((PressedLocation - Location).X > 0)
					{
						Swipe(ESwipeDirection::SD_Left);
					}
					else
					{
						Swipe(ESwipeDirection::SD_Right);
					}
				}
				else
				{
					if ((PressedLocation - Location).Y > 0)
					{
						Swipe(ESwipeDirection::SD_Up);
					}
					else
					{
						Swipe(ESwipeDirection::SD_Down);
					}
				}
			}
		}
	}
}

void ATLCGPawn::StartBattle()
{
	if (Role == ROLE_Authority && TLCMovement)
	{
		ClearTracks();
		
		auto PS = GetPlayerState<ATLCGPlayerState>();
		if (PS)
		{
			if (PS->StartTransform.Equals(FTransform::Identity))
			{
				PS->StartTransform = GetActorTransform();
			}
		}

		TLCMovement->Activate();
	}
}

void ATLCGPawn::StopBattle()
{
	if (Role == ROLE_Authority &&  TLCMovement)
		TLCMovement->Deactivate();
}

void ATLCGPawn::UnlockSkill()
{
	SkillLocked = false;
}

void ATLCGPawn::ClearTracks()
{
	if (Role == ROLE_Authority)
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
	}
}

void ATLCGPawn::TurnLeft()
{
	ServerTurnLeft();
}

void ATLCGPawn::TurnRight()
{
	ServerTurnRight();
}

void ATLCGPawn::ServerTurnLeft_Implementation()
{
	if (TLCMovement && TLCMovement->IsActive() && (LastLeftLocation - GetActorLocation()).Size() > (BoxComponent->GetScaledBoxExtent().Y * 2))
	{
		LastLeftLocation = GetActorLocation();

		auto SpawnedTrack = SpawnTrack();

		TLCMovement->TurnLeft(SpawnedTrack);
		MulticastOnRotate();
	}
}

bool ATLCGPawn::ServerTurnLeft_Validate()
{
	return true;
}

void ATLCGPawn::ServerTurnRight_Implementation()
{
	if (TLCMovement && TLCMovement->IsActive() && (LastRightLocataion - GetActorLocation()).Size() > (BoxComponent->GetScaledBoxExtent().Y * 2))
	{
		LastRightLocataion = GetActorLocation();

		auto SpawnedTrack = SpawnTrack();

		TLCMovement->TurnRight(SpawnedTrack);
		MulticastOnRotate();
	}
}

bool ATLCGPawn::ServerTurnRight_Validate()
{
	return true;
}

void ATLCGPawn::Skill()
{
	if (TLCMovement && TLCMovement->IsActive())
		ServerSkill();
}

void ATLCGPawn::Touch()
{
	Pressed = true;

	auto PC = GetController<APlayerController>();
	if (!PC)
		return;

	bool IsCurrentlyPressed;

	PC->GetInputTouchState(ETouchIndex::Touch1, PressedLocation.X, PressedLocation.Y, IsCurrentlyPressed);
}

void ATLCGPawn::UnTouch()
{
	if (!Swiped)
		ServerSkill();

	Pressed = false;
	Swiped = false;
}

void ATLCGPawn::MulticastSkill_Implementation(int32 InAvaibleSkillsAmount)
{
	K2_ActivateSkill(InAvaibleSkillsAmount);
}

void ATLCGPawn::ServerSkill_Implementation()
{
	if (!SkillLocked && AvaibleSkillsAmount > 0)
	{
		SkillLocked = true;
		AvaibleSkillsAmount--;
		MulticastSkill(AvaibleSkillsAmount);
	}
}

bool ATLCGPawn::ServerSkill_Validate()
{
	return true;
}

void ATLCGPawn::OnKilled(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && (Cast<ICanBeDamagerInterface>(OtherActor) || OtherActor->ActorHasTag("Damager")))
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
		LastLeftLocation = GetActorLocation();
		LastRightLocataion = GetActorLocation();

		auto PS = GetPlayerState<ATLCGPlayerState>();
		if (PS)
		{
			TLCMovement->SetRepData(FRepData(PS->StartTransform.GetLocation(), PS->StartTransform.GetRotation().Rotator().Yaw, nullptr));
		}
	}

	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);

	K2_OnRespawn();
}

ATLCGPawnTrack* ATLCGPawn::SpawnTrack()
{
	if (!DisableSpawnTracks && Role == ROLE_Authority)
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

void ATLCGPawn::Swipe(ESwipeDirection Dir)
{
	const float Error = 5.f;

	if (GetActorRotation().Equals(FRotator(GetActorRotation().Pitch, 0.f, GetActorRotation().Roll), Error))
	{
		if (Dir == ESwipeDirection::SD_Right)
		{
			ServerTurnRight();
		}
		else if (Dir == ESwipeDirection::SD_Left)
		{
			ServerTurnLeft();
		}
	}
	else if (GetActorRotation().Equals(FRotator(GetActorRotation().Pitch, 90.f, GetActorRotation().Roll), Error))
	{
		if (Dir == ESwipeDirection::SD_Up)
		{
			ServerTurnLeft();
		}
		else if (Dir == ESwipeDirection::SD_Down)
		{
			ServerTurnRight();
		}
	}
	else if (GetActorRotation().Equals(FRotator(GetActorRotation().Pitch, -179.f, GetActorRotation().Roll), Error))
	{
		if (Dir == ESwipeDirection::SD_Right)
		{
			ServerTurnLeft();
		}
		else if (Dir == ESwipeDirection::SD_Left)
		{
			ServerTurnRight();
		}
	}
	else if ((GetActorRotation().Equals(FRotator(GetActorRotation().Pitch, -90.f, GetActorRotation().Roll), Error)))
	{
		if (Dir == ESwipeDirection::SD_Up)
		{
			ServerTurnRight();
		}
		else if (Dir == ESwipeDirection::SD_Down)
		{
			ServerTurnLeft();
		}
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
