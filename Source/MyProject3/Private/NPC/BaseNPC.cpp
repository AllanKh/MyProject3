// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BaseNPC.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"

// Sets default values
ABaseNPC::ABaseNPC()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	// Skeletal Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	// Call AI Controller Class
	static ConstructorHelpers::FClassFinder<AAIController> AIControllerBP(TEXT("/Game/AI/Blueprints/NPC/BP_NPC_AI"));
	if (AIControllerBP.Succeeded())
	{
		AIControllerClass = AIControllerBP.Class;
	}
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Movement Component
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;
}

// Called when the game starts or when spawned
void ABaseNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseNPC::MoveTo(const FVector& Target, float DeltaTime)
{

	FVector Direction = (Target - GetActorLocation()).GetSafeNormal();
	FVector NewLocation = GetActorLocation() + Direction * Speed * DeltaTime;

	// Simple Collision Check
	SetActorLocation(NewLocation, true);
	SetActorRotation(Direction.Rotation());
}




