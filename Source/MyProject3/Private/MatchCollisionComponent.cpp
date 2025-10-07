#include "MatchCollisionComponent.h"
#include "ColorMatchComponent.h"
#include "MatchGameManager.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

UMatchCollisionComponent::UMatchCollisionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UMatchCollisionComponent::BeginPlay()
{
    Super::BeginPlay();

    TArray<UPrimitiveComponent*> PrimitivesToBind;

    // either bind all primitives for debug or just the one specified
    if (bShouldBindAllPrimitivesForDebug)
    {
        GetOwner()->GetComponents<UPrimitiveComponent>(PrimitivesToBind);
    }
    else
    {
        // if no collision primitive set, try to find one automatically
        if (!CollisionPrimitive)
        {
            if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
            {
                CollisionPrimitive = SkeletalMesh;
            }
            else
            {
                CollisionPrimitive = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
            }
        }
        if (CollisionPrimitive) PrimitivesToBind.Add(CollisionPrimitive);
    }

    // bind collision events to all primitives we found
    if (PrimitivesToBind.Num() > 0)
    {
        for (UPrimitiveComponent* Primitive : PrimitivesToBind)
        {
            BindEventsForOnePrimitive(Primitive);
        }
    }
    else
    {
        // fallback to actor level hit events if no primitives found
        GetOwner()->OnActorHit.AddDynamic(this, &UMatchCollisionComponent::OnActorHit);
    }
}

// sets up collision settings and binds events for one primitive component
void UMatchCollisionComponent::BindEventsForOnePrimitive(UPrimitiveComponent* Primitive)
{
    if (!Primitive)
    {
        return;
    }

    // enable overlap and hit events
    Primitive->SetGenerateOverlapEvents(true);
    Primitive->SetNotifyRigidBodyCollision(true);
    if (Primitive->BodyInstance.IsValidBodyInstance())
    {
        Primitive->BodyInstance.bNotifyRigidBodyCollision = true;
    }

    // if collision disabled, enable it for queries
    if (Primitive->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        Primitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    Primitive->OnComponentHit.AddDynamic(this, &UMatchCollisionComponent::OnComponentHit);
    Primitive->OnComponentBeginOverlap.AddDynamic(this, &UMatchCollisionComponent::OnComponentBeginOverlap);

    LogComponentSetup(Primitive, TEXT("Bind"));
}

// logs collision setup info for debugging
void UMatchCollisionComponent::LogComponentSetup(UPrimitiveComponent* Primitive, const TCHAR* LogTag) const
{
    if (!bShouldDebugLog || !Primitive) return;

    const auto CollisionEnabled = Primitive->GetCollisionEnabled();
    const bool bGeneratesOverlap = Primitive->GetGenerateOverlapEvents();
    const bool bNotifiesRigidBody = Primitive->BodyInstance.bNotifyRigidBodyCollision;
}

// finds color match component on an actor
UColorMatchComponent* UMatchCollisionComponent::GetColorMatchComponent(AActor* Actor) const
{
    return Actor ? Actor->FindComponentByClass<UColorMatchComponent>() : nullptr;
}

// checks if this collision should be scored based on speed, colors, and play area
bool UMatchCollisionComponent::ShouldScoreThisHit(AActor* OtherActor, float& OutRelativeSpeed,
    bool& bIsMyActorLooking, bool& bIsOtherActorLooking,
    uint8& MyActorColor, uint8& OtherActorColor, bool& bAreBothActorsInsideArea) const
{
    // initialize all output parameters
    OutRelativeSpeed = 0.f;
    bIsMyActorLooking = false;
    bIsOtherActorLooking = false;
    MyActorColor = 0;
    OtherActorColor = 0;
    bAreBothActorsInsideArea = true;

    if (!OtherActor || !GetOwner())
    {
        return false;
    }

    // both actors need color match components
    UColorMatchComponent* MyMatchComponent = GetColorMatchComponent(GetOwner());
    UColorMatchComponent* OtherMatchComponent = GetColorMatchComponent(OtherActor);
    if (!MyMatchComponent || !OtherMatchComponent) return false;

    // gather info about both actors
    bIsMyActorLooking = MyMatchComponent->IsLooking();
    bIsOtherActorLooking = OtherMatchComponent->IsLooking();
    MyActorColor = (uint8)MyMatchComponent->CurrentColor;
    OtherActorColor = (uint8)OtherMatchComponent->CurrentColor;

    // check if collision was hard enough
    const FVector RelativeVelocity = OtherActor->GetVelocity() - GetOwner()->GetVelocity();
    OutRelativeSpeed = RelativeVelocity.Size();

    if (OutRelativeSpeed < MinimumHitSpeed)
    {
        return false;
    }

    // if required, check if both actors are inside play area
    if (bMustBeInsidePlayArea)
    {
        TArray<AActor*> FoundManagers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), FoundManagers);
        if (FoundManagers.Num())
        {
            AMatchGameManager* GameManager = Cast<AMatchGameManager>(FoundManagers[0]);
            if (GameManager)
            {
                const bool bIsMyActorInside = GameManager->IsInsidePlayArea(GetOwner()->GetActorLocation());
                const bool bIsOtherActorInside = GameManager->IsInsidePlayArea(OtherActor->GetActorLocation());
                bAreBothActorsInsideArea = (bIsMyActorInside && bIsOtherActorInside);
                if (!bAreBothActorsInsideArea) return false;
            }
        }
    }

    // at least one actor needs to be looking for match
    if (!bIsMyActorLooking && !bIsOtherActorLooking)
    {
        return false;
    }

    return true;
}

// sends collision info to game manager for scoring
void UMatchCollisionComponent::ForwardCollisionToManager(AActor* SelfActor, AActor* OtherActor) const
{
    if (!SelfActor || !OtherActor)
    {
        return;
    }

    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), FoundManagers);
    if (FoundManagers.Num())
    {
        if (AMatchGameManager* GameManager = Cast<AMatchGameManager>(FoundManagers[0]))
        {
            GameManager->HandleNPCVsNPCCollision(SelfActor, OtherActor);
        }
    }
}

// called when actor hit event fires
void UMatchCollisionComponent::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& HitResult)
{
    float RelativeSpeed = 0.f;
    bool bIsMyActorLooking = false, bIsOtherActorLooking = false;
    uint8 MyActorColor = 0, OtherActorColor = 0;
    bool bAreBothInside = true;
    const bool bShouldScore = ShouldScoreThisHit(OtherActor, RelativeSpeed, bIsMyActorLooking, bIsOtherActorLooking, MyActorColor, OtherActorColor, bAreBothInside);

    if (bShouldScore)
    {
        ForwardCollisionToManager(SelfActor, OtherActor);
    }
}

// called when component hit event fires
void UMatchCollisionComponent::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& HitResult)
{
    float RelativeSpeed = 0.f;
    bool bIsMyActorLooking = false, bIsOtherActorLooking = false;
    uint8 MyActorColor = 0, OtherActorColor = 0;
    bool bAreBothInside = true;
    const bool bShouldScore = ShouldScoreThisHit(OtherActor, RelativeSpeed, bIsMyActorLooking, bIsOtherActorLooking, MyActorColor, OtherActorColor, bAreBothInside);

    if (bShouldScore)
    {
        ForwardCollisionToManager(GetOwner(), OtherActor);
    }
}

// called when component overlap event fires
void UMatchCollisionComponent::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    float RelativeSpeed = 0.f;
    bool bIsMyActorLooking = false, bIsOtherActorLooking = false;
    uint8 MyActorColor = 0, OtherActorColor = 0;
    bool bAreBothInside = true;
    const bool bShouldScore = ShouldScoreThisHit(OtherActor, RelativeSpeed, bIsMyActorLooking, bIsOtherActorLooking, MyActorColor, OtherActorColor, bAreBothInside);

    if (bShouldScore)
    {
        ForwardCollisionToManager(GetOwner(), OtherActor);
    }
}