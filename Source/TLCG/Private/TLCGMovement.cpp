// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGMovement.h"
#include "UnrealNetwork.h"

UTLCGMovement::UTLCGMovement(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
, PermissibleNetworkDiscrepancy(15.f)
, RepLocation(FVector::ZeroVector)
, RepZRotation(0.f)
, CachedOwner(nullptr)
, PrevLoc(FVector::ZeroVector)
{
	MaxSpeed = 100;
	bReplicates = true;
	SetAutoActivate(false);
}

void UTLCGMovement::BeginPlay()
{
	Super::BeginPlay();

	CachedOwner = GetOwner();
	PrevLoc = CachedOwner->GetActorLocation();
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

	MoveUpdatedComponent(CachedOwner->GetActorForwardVector() * DeltaTime * GetMaxSpeed(), FQuat(CachedOwner->GetActorRotation()), false, &HitResult, ETeleportType::None);

	if (CachedOwner->Role == ROLE_Authority)
	{
		RepLocation = GetActorLocation();

		if (!FMath::IsNearlyEqual(CachedOwner->GetActorRotation().Yaw, RepZRotation))
		{
			RepZRotation = CachedOwner->GetActorRotation().Yaw;
		}
	}
	else
	{
		//Possible Correction
		if ((CachedOwner->GetActorLocation() - RepLocation).Size() > PermissibleNetworkDiscrepancy)
		{
			CachedOwner->SetActorLocation(RepLocation);
			UE_LOG(LogTemp, Log, TEXT("Correction!!!"));
		}
	}

	//Set Velocity
	Velocity = CachedOwner->GetActorLocation() - PrevLoc;
	PrevLoc = CachedOwner->GetActorLocation();
}

void UTLCGMovement::OnRep_RepZRotation()
{
	if (CachedOwner)
		CachedOwner->SetActorRotation(FRotator(0.f, RepZRotation, 0.f));
}

void UTLCGMovement::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTLCGMovement, RepLocation);
	DOREPLIFETIME(UTLCGMovement, RepZRotation);
}