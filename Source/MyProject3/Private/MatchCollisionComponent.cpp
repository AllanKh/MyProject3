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

    // 1) Decide what to bind
    TArray<UPrimitiveComponent*> Targets;

    if (bBindAllPrimitivesForDebug)
    {
        // Use templated getter (portable)
        GetOwner()->GetComponents<UPrimitiveComponent>(Targets);
    }
    else
    {
        if (!HitPrimitive)
        {
            if (USkeletalMeshComponent* Skel = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
            {
                HitPrimitive = Skel;
            }
            else
            {
                HitPrimitive = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
            }
        }
        if (HitPrimitive) Targets.Add(HitPrimitive);
    }

    // 2) Bind to chosen primitives
    if (Targets.Num() > 0)
    {
        for (UPrimitiveComponent* Prim : Targets)
        {
            BindOnePrimitive(Prim);
        }
    }
    else
    {
        // Last resort: bind actor-level hit
        GetOwner()->OnActorHit.AddDynamic(this, &UMatchCollisionComponent::OnActorHit);
        if (bDebugLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("[MatchCollision] %s: No primitive found, using OnActorHit"), *GetOwner()->GetName());
        }
    }
}

void UMatchCollisionComponent::BindOnePrimitive(UPrimitiveComponent* Prim)
{
    if (!Prim) return;

    Prim->SetGenerateOverlapEvents(true);
    Prim->SetNotifyRigidBodyCollision(true);
    if (Prim->BodyInstance.IsValidBodyInstance())
    {
        Prim->BodyInstance.bNotifyRigidBodyCollision = true;
    }

    if (Prim->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    Prim->OnComponentHit.AddDynamic(this, &UMatchCollisionComponent::OnCompHit);
    Prim->OnComponentBeginOverlap.AddDynamic(this, &UMatchCollisionComponent::OnCompBeginOverlap);

    LogComponentSetup(Prim, TEXT("Bind"));
}

void UMatchCollisionComponent::LogComponentSetup(UPrimitiveComponent* Prim, const TCHAR* Tag) const
{
    if (!bDebugLog || !Prim) return;

    const auto CE = Prim->GetCollisionEnabled();
    const bool bGenOverlap = Prim->GetGenerateOverlapEvents();
    const bool bNotifyRigid = Prim->BodyInstance.bNotifyRigidBodyCollision;

    UE_LOG(LogTemp, Warning, TEXT("[MatchCollision][%s] %s -> Prim=%s | CollEnabled=%d Overlap=%d NotifyRigid=%d ObjType=%d"),
        Tag,
        *GetOwner()->GetName(),
        *Prim->GetName(),
        (int32)CE,
        bGenOverlap ? 1 : 0,
        bNotifyRigid ? 1 : 0,
        (int32)Prim->GetCollisionObjectType());
}

UColorMatchComponent* UMatchCollisionComponent::GetMatch(AActor* Act) const
{
    return Act ? Act->FindComponentByClass<UColorMatchComponent>() : nullptr;
}

bool UMatchCollisionComponent::ShouldScoreThisHit(AActor* OtherActor, float& OutRelSpeed,
    bool& bMyLooking, bool& bOtherLooking,
    uint8& MyColor, uint8& OtherColor, bool& bInsideAreaBoth) const
{
    OutRelSpeed = 0.f;
    bMyLooking = false;
    bOtherLooking = false;
    MyColor = 0;
    OtherColor = 0;
    bInsideAreaBoth = true;

    if (!OtherActor || !GetOwner()) return false;

    UColorMatchComponent* Me = GetMatch(GetOwner());
    UColorMatchComponent* Ot = GetMatch(OtherActor);
    if (!Me || !Ot) return false;

    bMyLooking = Me->IsLooking();
    bOtherLooking = Ot->IsLooking();
    MyColor = (uint8)Me->CurrentColor;
    OtherColor = (uint8)Ot->CurrentColor;

    const FVector RelVel = OtherActor->GetVelocity() - GetOwner()->GetVelocity();
    OutRelSpeed = RelVel.Size();

    if (OutRelSpeed < MinHitSpeed) return false;

    if (bRequireInsidePlayArea)
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), Found);
        if (Found.Num())
        {
            AMatchGameManager* M = Cast<AMatchGameManager>(Found[0]);
            if (M)
            {
                const bool AInside = M->IsInsidePlayArea(GetOwner()->GetActorLocation());
                const bool BInside = M->IsInsidePlayArea(OtherActor->GetActorLocation());
                bInsideAreaBoth = (AInside && BInside);
                if (!bInsideAreaBoth) return false;
            }
        }
    }

    if (!bMyLooking && !bOtherLooking) return false;

    return true;
}

void UMatchCollisionComponent::ForwardToManager(AActor* SelfActor, AActor* OtherActor) const
{
    if (!SelfActor || !OtherActor) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), Found);
    if (Found.Num())
    {
        if (AMatchGameManager* M = Cast<AMatchGameManager>(Found[0]))
        {
            if (bDebugLog)
            {
                UE_LOG(LogTemp, Warning, TEXT("[MatchCollision] ForwardToManager %s vs %s"),
                    *SelfActor->GetName(), *OtherActor->GetName());
            }
            M->HandleNPCVsNPCCollision(SelfActor, OtherActor);
        }
    }
}

void UMatchCollisionComponent::OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    float Rel = 0.f; bool MyL = false, OtherL = false; uint8 MyC = 0, OtherC = 0; bool Inside = true;
    const bool bOK = ShouldScoreThisHit(OtherActor, Rel, MyL, OtherL, MyC, OtherC, Inside);

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MatchCollision] OnActorHit %s vs %s | Rel=%.1f Min=%.1f | My(L:%d C:%d) Other(L:%d C:%d) Inside:%d -> Pass:%d"),
            *SelfActor->GetName(), *GetNameSafe(OtherActor),
            Rel, MinHitSpeed, MyL, MyC, OtherL, OtherC, Inside, bOK);
    }

    if (bOK) ForwardToManager(SelfActor, OtherActor);
}

void UMatchCollisionComponent::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    FVector NormalImpulse, const FHitResult& Hit)
{
    float Rel = 0.f; bool MyL = false, OtherL = false; uint8 MyC = 0, OtherC = 0; bool Inside = true;
    const bool bOK = ShouldScoreThisHit(OtherActor, Rel, MyL, OtherL, MyC, OtherC, Inside);

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MatchCollision] OnCompHit %s[%s] vs %s[%s] | Rel=%.1f Min=%.1f | My(L:%d C:%d) Other(L:%d C:%d) Inside:%d -> Pass:%d"),
            *GetOwner()->GetName(), *GetNameSafe(HitComp),
            *GetNameSafe(OtherActor), *GetNameSafe(OtherComp),
            Rel, MinHitSpeed, MyL, MyC, OtherL, OtherC, Inside, bOK);
    }

    if (bOK) ForwardToManager(GetOwner(), OtherActor);
}

void UMatchCollisionComponent::OnCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    float Rel = 0.f; bool MyL = false, OtherL = false; uint8 MyC = 0, OtherC = 0; bool Inside = true;
    const bool bOK = ShouldScoreThisHit(OtherActor, Rel, MyL, OtherL, MyC, OtherC, Inside);

    if (bDebugLog)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MatchCollision] Overlap %s[%s] with %s[%s] | Rel=%.1f Min=%.1f | My(L:%d C:%d) Other(L:%d C:%d) Inside:%d -> Pass:%d"),
            *GetOwner()->GetName(), *GetNameSafe(OverlappedComp),
            *GetNameSafe(OtherActor), *GetNameSafe(OtherComp),
            Rel, MinHitSpeed, MyL, MyC, OtherL, OtherC, Inside, bOK);
    }

    if (bOK) ForwardToManager(GetOwner(), OtherActor);
}
