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

    if (bShouldBindAllPrimitivesForDebug)
    {
        GetOwner()->GetComponents<UPrimitiveComponent>(PrimitivesToBind);
    }
    else
    {
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

    if (PrimitivesToBind.Num() > 0)
    {
        for (UPrimitiveComponent* Primitive : PrimitivesToBind)
        {
            BindEventsForOnePrimitive(Primitive);
        }
    }
    else
    {
        GetOwner()->OnActorHit.AddDynamic(this, &UMatchCollisionComponent::OnActorHit);
    }
}

void UMatchCollisionComponent::BindEventsForOnePrimitive(UPrimitiveComponent* Primitive)
{
    if (!Primitive)
    {
        return;
    }

    Primitive->SetGenerateOverlapEvents(true);
    Primitive->SetNotifyRigidBodyCollision(true);
    if (Primitive->BodyInstance.IsValidBodyInstance())
    {
        Primitive->BodyInstance.bNotifyRigidBodyCollision = true;
    }

    if (Primitive->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        Primitive->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    Primitive->OnComponentHit.AddDynamic(this, &UMatchCollisionComponent::OnComponentHit);
    Primitive->OnComponentBeginOverlap.AddDynamic(this, &UMatchCollisionComponent::OnComponentBeginOverlap);

    LogComponentSetup(Primitive, TEXT("Bind"));
}

void UMatchCollisionComponent::LogComponentSetup(UPrimitiveComponent* Primitive, const TCHAR* LogTag) const
{
    if (!bShouldDebugLog || !Primitive) return;

    const auto CollisionEnabled = Primitive->GetCollisionEnabled();
    const bool bGeneratesOverlap = Primitive->GetGenerateOverlapEvents();
    const bool bNotifiesRigidBody = Primitive->BodyInstance.bNotifyRigidBodyCollision;
}

UColorMatchComponent* UMatchCollisionComponent::GetColorMatchComponent(AActor* Actor) const
{
    return Actor ? Actor->FindComponentByClass<UColorMatchComponent>() : nullptr;
}

bool UMatchCollisionComponent::ShouldScoreThisHit(AActor* OtherActor, float& OutRelativeSpeed,
    bool& bIsMyActorLooking, bool& bIsOtherActorLooking,
    uint8& MyActorColor, uint8& OtherActorColor, bool& bAreBothActorsInsideArea) const
{
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

    UColorMatchComponent* MyMatchComponent = GetColorMatchComponent(GetOwner());
    UColorMatchComponent* OtherMatchComponent = GetColorMatchComponent(OtherActor);
    if (!MyMatchComponent || !OtherMatchComponent) return false;

    bIsMyActorLooking = MyMatchComponent->IsLooking();
    bIsOtherActorLooking = OtherMatchComponent->IsLooking();
    MyActorColor = (uint8)MyMatchComponent->CurrentColor;
    OtherActorColor = (uint8)OtherMatchComponent->CurrentColor;

    const FVector RelativeVelocity = OtherActor->GetVelocity() - GetOwner()->GetVelocity();
    OutRelativeSpeed = RelativeVelocity.Size();

    if (OutRelativeSpeed < MinimumHitSpeed) 
    {
        return false;
    }

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

    if (!bIsMyActorLooking && !bIsOtherActorLooking) 
    {
        return false;
    }

    return true;
}

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