#include "PushableComponent.h"

UPushableComponent::UPushableComponent()
{
    // dont need to tick every frame, only update when being interacted with
    PrimaryComponentTick.bCanEverTick = false;
}

void UPushableComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPushableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// tells the system thyis component supports pushing
EInteractCaps UPushableComponent::GetInteractionCapabilities_Implementation() const
{
    return EInteractCaps::Push;
}

// called every frame while player is clicking and dragging on object
void UPushableComponent::OnInteractUpdate_Implementation(const FInteractUpdate& Update)
{
    // find the physics object to push. use TargetComponent if set, otherwise use root
    UPrimitiveComponent* Primitive = TargetComponent;
    if (!Primitive)
    {
        Primitive = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    }

    if (!Primitive) return;  // no valid object found

    // make sure physics is enabled so we can apply forces (probably remove this ?)
    if (!Primitive->IsSimulatingPhysics())
    {
        Primitive->SetSimulatePhysics(true);
    }

    // calculate spring gdamper force to push object toward cursor
    FVector ContactPoint = Update.WorldHitLocation;    // where pushing from
    FVector TargetPoint = Update.WorldTargetPoint;     // where cursor is in 3d space
    FVector Displacement = TargetPoint - ContactPoint; // direction and distance to push

    // get current velocity at contact point
    FVector Velocity = Primitive->GetPhysicsLinearVelocityAtPoint(ContactPoint);

    // spring force pulls toward target damper resists velocity
    FVector Force = (Displacement * SpringStrength) - (Velocity * DamperStrength);

    // limit max force to prevent objects flying away
    Force = Force.GetClampedToMaxSize(MaxForceLimit);

    // apply the force at the contact point
    Primitive->AddForceAtLocation(Force, ContactPoint);
}