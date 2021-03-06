// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGMovement.h"
#include "UnrealNetwork.h"
#include "TLCGPawnTrack.h"
#include "TLCGPawn.h"
#include "TLCGPlayerState.h"
#include "TLCGGameState.h"
#include "Engine/World.h"

UTLCGMovement::UTLCGMovement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, CachedOwner(nullptr)
, PrevLoc(FVector::ZeroVector)
, Sweep(false)
{
	MaxSpeed = 100;
	bReplicates = true;
	SetAutoActivate(false);
}

void UTLCGMovement::BeginPlay()
{
	Super::BeginPlay();

	CachedOwner = GetOwner();

	if (CachedOwner)
	{
		PrevLoc = CachedOwner->GetActorLocation();

		if (CachedOwner->Role == ROLE_Authority)
		{
			Sweep = true;
		}
	}
}

void UTLCGMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedOwner)
	{
		if (GetOwner())
		{
			CachedOwner = GetOwner();
		}
		else
		{
			return;
		}
	}

	MoveUpdatedComponent(CachedOwner->GetActorForwardVector() * DeltaTime * GetMaxSpeed(), FQuat(CachedOwner->GetActorRotation()), Sweep, &HitResult, ETeleportType::None);


	//Set Velocity
	Velocity = CachedOwner->GetActorLocation() - PrevLoc;
	PrevLoc = CachedOwner->GetActorLocation();
}

void UTLCGMovement::TurnRight(ATLCGPawnTrack* NewTrack)
{
	auto Pawn = Cast<ATLCGPawn>(GetOwner());
	if (!Pawn)
		return;

	auto PS = Pawn->GetPlayerState<ATLCGPlayerState>();
	if (!PS || PS->GetPlayerState() == EPlayerStateEnum::PS_Killed)
		return;

	if (CachedOwner->Role == ROLE_Authority)
	{
		CachedOwner->AddActorWorldRotation(FQuat(FRotator(0.f, 90.f, 0.f)));

		SetRepData(FRepData(CachedOwner->GetActorLocation(), CachedOwner->GetActorRotation().Yaw, NewTrack));
	}
}

void UTLCGMovement::TurnLeft(ATLCGPawnTrack* NewTrack)
{
	auto Pawn = Cast<ATLCGPawn>(GetOwner());
	if (!Pawn)
		return;

	auto PS = Pawn->GetPlayerState<ATLCGPlayerState>();
	if (!PS || PS->GetPlayerState() == EPlayerStateEnum::PS_Killed)
		return;

	if (CachedOwner->Role == ROLE_Authority)
	{
		CachedOwner->AddActorWorldRotation(FQuat(FRotator(0.f, -90.f, 0.f)));

		SetRepData(FRepData(CachedOwner->GetActorLocation(), CachedOwner->GetActorRotation().Yaw, NewTrack));
	}
}

void UTLCGMovement::SetRepData(const FRepData& NewRepData)
{
	if (CachedOwner && CachedOwner->Role == ROLE_Authority)
	{
		auto OldData = RepData;
		RepData = NewRepData;
		OnRep_RepData(OldData);
	}
}

const FRepData& UTLCGMovement::GetRepData() const
{
	return RepData;
}

void UTLCGMovement::Activate(bool bReset/*=false*/)
{
	auto Pawn = Cast<ATLCGPawn>(GetOwner());
	if (!Pawn)
		return;

	auto World = GetWorld();
	if (!World)
		return;

	auto GS = World->GetGameState<ATLCGGameState>();
	if (!GS)
		return;

	if (GS->GetGameState() != EGameStateEnum::GS_RoundInProgress)
		return;

	auto PS = Pawn->GetPlayerState<ATLCGPlayerState>();
	if (!PS || PS->GetPlayerState() == EPlayerStateEnum::PS_Killed)
		return;

	Super::Activate(bReset);
}

void UTLCGMovement::OnRep_RepData(FRepData OldData)
{
	if (CachedOwner)
	{
		PrevLoc = RepData.Location;

		CachedOwner->SetActorLocationAndRotation(RepData.Location, FQuat(FRotator(CachedOwner->GetActorRotation().Pitch, RepData.Yaw, CachedOwner->GetActorRotation().Roll)));
	
		OnRotate.Broadcast(CachedOwner->GetActorTransform(), RepData.Track, OldData.Track);
	}
}

void UTLCGMovement::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTLCGMovement, RepData);
}