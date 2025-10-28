// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BaseNPC.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "MyProject3/GrabbableComponent.h"

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
    static ConstructorHelpers::FClassFinder<AAIController> AIControllerBP(TEXT("/Game/AI/Blueprints/NPC/BP_NPC_AI_TEST"));
    if (AIControllerBP.Succeeded())
    {
        AIControllerClass = AIControllerBP.Class;
    }
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Movement Component
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->UpdatedComponent = RootComponent;

    // Grabbable Component
    GrabbableComponent = CreateDefaultSubobject<UGrabbableComponent>(TEXT("GrabbableComponent"));


}

// Called when the game starts or when spawned
void ABaseNPC::BeginPlay()
{
    Super::BeginPlay();

    if (GrabbableComponent)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.9f, FColor::Yellow, TEXT("HELLO"));
    }
    
}

// Called every frame
void ABaseNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //Apply Gravity

    if (!Mesh->IsSimulatingPhysics())
    {
        FVector beamStart = GetActorLocation();
        FVector beamEnd = beamStart - FVector(0.0f, 0.0f, 50.0f);

        FHitResult hit;
        FCollisionQueryParams params;
        params.AddIgnoredActor(this);

        bool hitGround = GetWorld()->LineTraceSingleByChannel(hit, beamStart, beamEnd, ECC_Visibility, params);
        if (!hitGround)
        {
        FVector newLocation = GetActorLocation();
        newLocation.Z -= 982.0f * DeltaTime;
        SetActorLocation(newLocation, true);
        }
        else
        {
            FVector newLocation = GetActorLocation();
            newLocation.Z = hit.ImpactPoint.Z + 10.0f;
            SetActorLocation(newLocation, true);
        }


    }
}

void ABaseNPC::HandleGrabbed()
{

}

void ABaseNPC::HandleReleased()
{

}

void ABaseNPC::EnterRagdoll()
{
}

void ABaseNPC::ExitRagdoll()
{
}

bool ABaseNPC::GetRagdollState()
{
    return ragdollState;
}




