#include "GrabbableComponent.h"
#include "GameFramework/Actor.h"

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

        // apply setttings from this component to handle
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

    IsBeingGrabbed = true;

    EnsurePhysicsHandle();

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
        float ObjectMass = Primitive->GetMass();
        UE_LOG(LogTemp, Warning, TEXT("Object mass: %f kg"), ObjectMass);

        // heavier objects need stronger forces to move them
        // using square root so mass effect isn't too extreme
        float MassScale = FMath::Sqrt(ObjectMass / 50.f); // 50kg baseline

        PhysicsHandle->LinearDamping = LinearDamping * MassScale;
        PhysicsHandle->LinearStiffness = LinearStiffness * MassScale;
        PhysicsHandle->AngularDamping = AngularDamping * MassScale;
        PhysicsHandle->AngularStiffness = AngularStiffness * MassScale;

        // heavier objects respond slower
        PhysicsHandle->InterpolationSpeed = FMath::Clamp(30.f / MassScale, 5.f, 30.f);

        InitialRotation = Primitive->GetComponentRotation();
        RotationOffset = FRotator::ZeroRotator;

        PhysicsHandle->GrabComponentAtLocationWithRotation(
            Primitive,
            NAME_None,
            Hit.ImpactPoint,
            InitialRotation
        );

        UE_LOG(LogTemp, Warning, TEXT("PhysicsHandle grabbed component. MassScale: %f"), MassScale);
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

    if (Update.bAltRotate)
    {
        // RMB held. rotate object based on mouse movement
        RotationOffset.Yaw += Update.ScreenDelta.X * RotationSpeed * Update.DeltaSeconds;
        RotationOffset.Pitch += -Update.ScreenDelta.Y * RotationSpeed * Update.DeltaSeconds;

        // auto rotate around Y axis while RMB held
        RotationOffset.Yaw += 180.f * Update.DeltaSeconds; // 180 degrees per second

        FRotator NewRotation = InitialRotation + RotationOffset;
        PhysicsHandle->SetTargetRotation(NewRotation);
    }
    else
    {
        // RMB not held. move object to follow cursor
        PhysicsHandle->SetTargetLocation(Update.WorldTargetPoint);
    }
}

// called when player releases mouse. drops object
void UGrabbableComponent::OnInteractEnd_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("GrabbableComponent::OnInteractEnd called"));

    IsBeingGrabbed = false;

    if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
    {
        PhysicsHandle->ReleaseComponent();
        UE_LOG(LogTemp, Warning, TEXT("PhysicsHandle released component"));
    }
}