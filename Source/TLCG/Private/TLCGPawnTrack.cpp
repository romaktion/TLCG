// Fill out your copyright notice in the Description page of Project Settings.


#include "TLCGPawnTrack.h"
#include "Components/BoxComponent.h"

// Sets default values
ATLCGPawnTrack::ATLCGPawnTrack(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetupAttachment(SceneComponent);
	BoxComponent->InitBoxExtent(FVector(10.f, 10.f, 10.f));
	BoxComponent->SetCollisionProfileName("BlockAll");
	BoxComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	BoxComponent->SetCanEverAffectNavigation(false);
	BoxComponent->AreaClass = nullptr;
	BoxComponent->SetGenerateOverlapEvents(false);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ATLCGPawnTrack::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorScale3D(FVector(0.0001f, GetActorScale3D().Y, GetActorScale3D().Z));
}

// Called every frame
void ATLCGPawnTrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATLCGPawnTrack::MulticastEnable_Implementation(bool Enable)
{
	SetActorEnableCollision(Role == ROLE_Authority ? Enable : false);
	SetActorHiddenInGame(!Enable);
}

void ATLCGPawnTrack::MulticastSetTransform_Implementation(const FTransform& NewTransform)
{
	SetActorTransform(NewTransform);
}

