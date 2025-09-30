#include "GrabbableComponent.h"
#include "GameFramework/Actor.h"

// Character / physics includes for special-case handling
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"

UGrabbableComponent::UGrabbableComponent()
{
    // dont need tick, only responds to interaction events
    PrimaryComponentTick.bCanEverTick = false;
}

void UGrabbableComponent::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("GrabbableComponent BeginPlay on: %s"), *GetOwner()->GetName());
}

void UGrabbableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// creates physics handle that handles smooth movement
void UGrabbableComponent::EnsurePhysicsHandle()
{
    if (!PhysicsHandle)
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating PhysicsHandle for GrabbableComponent"));

        // dynamically create physics handle component
        PhysicsHandle = NewObject<UPhysicsHandleComponent>(GetOwner(), TEXT("PhysicsHandle_Grab"));
        PhysicsHandle->RegisterComponent();

        // apply base settings (we may scale/change these per grab)
        PhysicsHandle->LinearDamping = LinearDamping;
        PhysicsHandle->LinearStiffness = LinearStiffness;
        PhysicsHandle->AngularDamping = AngularDamping;
        PhysicsHandle->AngularStiffness = AngularStiffness;
        PhysicsHandle->InterpolationSpeed = 30.f;
    }
}

// finds which physics object to grab. uses TargetComponent if set, otherwise uses whatever was hit
UPrimitiveComponent* UGrabbableComponent::GetTargetPrimitive(const FHitResult& Hit)
{
    if (TargetComponent) return TargetComponent;
    return Cast<UPrimitiveComponent>(Hit.GetComponent());
}

// tells system this component supports grabbing
EInteractCaps UGrabbableComponent::GetInteractionCapabilities_Implementation() const
{
    return EInteractCaps::Grab;
}

// called when player clicks on object. sets up the grab
void UGrabbableComponent::OnInteractStart_Implementation(const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("GrabbableComponent::OnInteractStart called"));

    EnsurePhysicsHandle();
    if (!PhysicsHandle) return;

    bLockRotationWhileHeld = false;   // reset each grab
    GrabbedBone = NAME_None;

    // --- SPECIAL CASE: If we hit a Character, grab its SkeletalMesh in ragdoll, not the capsule
    if (ACharacter* AsChar = Hit.GetActor() ? Cast<ACharacter>(Hit.GetActor()) : nullptr)
    {
        HeldCharacter = AsChar;
        HeldCapsule = AsChar->GetCapsuleComponent();
        HeldSkel = AsChar->GetMesh();

        if (!HeldSkel)
        {
            UE_LOG(LogTemp, Error, TEXT("Character has no SkeletalMesh to grab."));
            return;
        }

        // 1) Disable character movement so physics can take over
        if (UCharacterMovementComponent* Move = AsChar->GetCharacterMovement())
        {
            SavedMoveMode = static_cast<uint8>(Move->MovementMode);
            Move->DisableMovement();
        }

        // 2) Turn off capsule collision (we’ll collide with the ragdoll instead)
        if (HeldCapsule)
        {
            SavedCapsuleCollision = HeldCapsule->GetCollisionEnabled();
            HeldCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 3) Enable physics on the mesh & set a physics-friendly profile
        SavedMeshCollisionProfile = HeldSkel->GetCollisionProfileName();
        HeldSkel->SetCollisionProfileName(TEXT("Ragdoll")); // requires a Physics Asset
        HeldSkel->SetSimulatePhysics(true);

        // 4) Decide where to grab: nearest bone to impact, else center of mass
        GrabbedBone = HeldSkel->FindClosestBone(Hit.ImpactPoint);
        const FVector GrabPoint = (GrabbedBone != NAME_None)
            ? HeldSkel->GetBoneLocation(GrabbedBone)
            : HeldSkel->GetCenterOfMass();

        // --- Mass scaling (you can swap for the power-law mapping if you prefer)
        const float ObjectMass = FMath::Max(HeldSkel->GetMass(), 1.f);
        UE_LOG(LogTemp, Warning, TEXT("Character mesh mass: %f kg"), ObjectMass);
        const float MassScale = FMath::Sqrt(ObjectMass / 50.f); // 50kg baseline

        PhysicsHandle->LinearDamping = LinearDamping * MassScale;
        PhysicsHandle->LinearStiffness = LinearStiffness * MassScale;
        PhysicsHandle->AngularDamping = AngularDamping * MassScale;
        PhysicsHandle->AngularStiffness = AngularStiffness * MassScale;
        PhysicsHandle->InterpolationSpeed = FMath::Clamp(30.f / MassScale, 5.f, 30.f);

        // --- NEW: if rotation is locked on the body, do NOT drive rotation
        const FBodyInstance* BI = HeldSkel->GetBodyInstance();
        bLockRotationWhileHeld =
            (BI && (BI->bLockXRotation || BI->bLockYRotation || BI->bLockZRotation));

        if (bLockRotationWhileHeld)
        {
            // kill angular drive and grab by location only
            PhysicsHandle->AngularStiffness = 0.f;
            PhysicsHandle->AngularDamping = 0.f;

            PhysicsHandle->GrabComponentAtLocation(
                HeldSkel,
                GrabbedBone,
                GrabPoint
            );
        }
        else
        {
            InitialRotation = HeldSkel->GetComponentRotation();
            RotationOffset = FRotator::ZeroRotator;

            PhysicsHandle->GrabComponentAtLocationWithRotation(
                HeldSkel,
                GrabbedBone,
                GrabPoint,
                InitialRotation
            );
        }

        UE_LOG(LogTemp, Warning, TEXT("Grabbing Character mesh (bone: %s) MassScale: %f (RotLock=%s)"),
            *GrabbedBone.ToString(), MassScale, bLockRotationWhileHeld ? TEXT("YES") : TEXT("NO"));

        return; // done with Character path
    }

    // --- DEFAULT PATH: non-Character props (your original logic) ---
    UPrimitiveComponent* Primitive = GetTargetPrimitive(Hit);
    if (Primitive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Grabbing primitive: %s"), *Primitive->GetName());

        if (!Primitive->IsSimulatingPhysics())
        {
            UE_LOG(LogTemp, Warning, TEXT("Enabling physics simulation on: %s"), *Primitive->GetName());
            Primitive->SetSimulatePhysics(true);
        }

        // get mass and scale physics handle strength accordingly
        const float ObjectMass = FMath::Max(Primitive->GetMass(), 1.f);
        UE_LOG(LogTemp, Warning, TEXT("Object mass: %f kg"), ObjectMass);

        const float MassScale = FMath::Sqrt(ObjectMass / 50.f); // 50kg baseline

        PhysicsHandle->LinearDamping = LinearDamping * MassScale;
        PhysicsHandle->LinearStiffness = LinearStiffness * MassScale;
        PhysicsHandle->AngularDamping = AngularDamping * MassScale;
        PhysicsHandle->AngularStiffness = AngularStiffness * MassScale;

        // heavier objects respond slower
        PhysicsHandle->InterpolationSpeed = FMath::Clamp(30.f / MassScale, 5.f, 30.f);

        // --- NEW: respect rotation locks
        const FBodyInstance* BI = Primitive->GetBodyInstance();
        bLockRotationWhileHeld =
            (BI && (BI->bLockXRotation || BI->bLockYRotation || BI->bLockZRotation));

        const FVector CoM = Primitive->GetCenterOfMass();

        if (bLockRotationWhileHeld)
        {
            PhysicsHandle->AngularStiffness = 0.f;
            PhysicsHandle->AngularDamping = 0.f;

            PhysicsHandle->GrabComponentAtLocation(
                Primitive,
                NAME_None,
                CoM
            );
        }
        else
        {
            InitialRotation = Primitive->GetComponentRotation();
            RotationOffset = FRotator::ZeroRotator;

            PhysicsHandle->GrabComponentAtLocationWithRotation(
                Primitive,
                NAME_None,
                CoM, // (or Hit.ImpactPoint if you prefer)
                InitialRotation
            );
        }

        UE_LOG(LogTemp, Warning, TEXT("PhysicsHandle grabbed component. MassScale: %f (RotLock=%s)"),
            MassScale, bLockRotationWhileHeld ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get target primitive!"));
    }
}

// called every frame while grabbing. either rotates or moves object
void UGrabbableComponent::OnInteractUpdate_Implementation(const FInteractUpdate& Update)
{
    if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent()) return;

    if (!bLockRotationWhileHeld && Update.bAltRotate)
    {
        // RMB held. rotate object based on mouse movement
        RotationOffset.Yaw += Update.ScreenDelta.X * RotationSpeed * Update.DeltaSeconds;
        RotationOffset.Pitch += -Update.ScreenDelta.Y * RotationSpeed * Update.DeltaSeconds;

        // auto rotate around Y axis while RMB held
        RotationOffset.Yaw += 180.f * Update.DeltaSeconds; // 180 degrees per second

        const FRotator NewRotation = InitialRotation + RotationOffset;
        PhysicsHandle->SetTargetRotation(NewRotation);
    }
    else
    {
        // Only move; do NOT set rotation
        PhysicsHandle->SetTargetLocation(Update.WorldTargetPoint);
    }
}

// called when player releases mouse. drops object
void UGrabbableComponent::OnInteractEnd_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("GrabbableComponent::OnInteractEnd called"));

    if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
    {
        PhysicsHandle->ReleaseComponent();
        UE_LOG(LogTemp, Warning, TEXT("PhysicsHandle released component"));
    }

    // If we ragdolled a Character, restore original state
    if (HeldCharacter)
    {
        if (HeldSkel)
        {
            HeldSkel->SetSimulatePhysics(false);
            if (SavedMeshCollisionProfile != NAME_None)
            {
                HeldSkel->SetCollisionProfileName(SavedMeshCollisionProfile);
            }
        }

        if (HeldCapsule)
        {
            HeldCapsule->SetCollisionEnabled(SavedCapsuleCollision);
        }

        if (UCharacterMovementComponent* Move = HeldCharacter->GetCharacterMovement())
        {
            Move->SetMovementMode(static_cast<EMovementMode>(SavedMoveMode));
        }

        HeldCharacter = nullptr;
        HeldSkel = nullptr;
        HeldCapsule = nullptr;
        GrabbedBone = NAME_None;
    }

    // clear lock flag after release
    bLockRotationWhileHeld = false;
}
