#include "MatchGameManager.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ColorMatchComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AMatchGameManager::AMatchGameManager()
{
    PrimaryActorTick.bCanEverTick = false;
    PlayArea = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayArea"));
    RootComponent = PlayArea;
    PlayArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMatchGameManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("[Manager] BeginPlay. AssignInterval [%.2f, %.2f]"), AssignIntervalMin, AssignIntervalMax);
    ScheduleNextAssign();
}

void AMatchGameManager::RegisterNPC(AActor* NPC)
{
    if (!NPC) return;
    NPCs.AddUnique(NPC);
    UE_LOG(LogTemp, Warning, TEXT("[Manager] Registered NPC: %s (Total=%d)"), *NPC->GetName(), NPCs.Num());
}

void AMatchGameManager::UnregisterNPC(AActor* NPC)
{
    NPCs.Remove(NPC);
    UE_LOG(LogTemp, Warning, TEXT("[Manager] Unregistered NPC: %s (Total=%d)"), *GetNameSafe(NPC), NPCs.Num());
}

UColorMatchComponent* AMatchGameManager::GetMatchComp(AActor* Actor)
{
    return Actor ? Actor->FindComponentByClass<UColorMatchComponent>() : nullptr;
}

bool AMatchGameManager::ColorsMatchAndBothLooking(UColorMatchComponent* A, UColorMatchComponent* B, EMatchColor& Out)
{
    if (!A || !B) return false;
    if (!A->IsLooking() || !B->IsLooking()) return false;
    if (A->CurrentColor == EMatchColor::None || B->CurrentColor == EMatchColor::None) return false;
    const bool bSame = (A->CurrentColor == B->CurrentColor);
    if (bSame) Out = A->CurrentColor;
    return bSame;
}

bool AMatchGameManager::IsInsidePlayArea(const FVector& WorldPos) const
{
    return UKismetMathLibrary::IsPointInBoxWithTransform(WorldPos, PlayArea->GetComponentTransform(), PlayArea->GetScaledBoxExtent());
}

void AMatchGameManager::HandleNPCVsNPCCollision(AActor* A, AActor* B)
{
    if (!A || !B) return;

    UColorMatchComponent* CA = GetMatchComp(A);
    UColorMatchComponent* CB = GetMatchComp(B);

    UE_LOG(LogTemp, Warning, TEXT("[Manager] HandleCollision %s vs %s | CA=%s CB=%s"),
        *A->GetName(), *B->GetName(),
        CA ? TEXT("YES") : TEXT("NO"),
        CB ? TEXT("YES") : TEXT("NO"));

    if (!CA || !CB) return;

    UE_LOG(LogTemp, Warning, TEXT("[Manager]   States: A(L:%d C:%d) B(L:%d C:%d)"),
        CA->IsLooking() ? 1 : 0, (uint8)CA->CurrentColor,
        CB->IsLooking() ? 1 : 0, (uint8)CB->CurrentColor);

    EMatchColor Color;
    if (ColorsMatchAndBothLooking(CA, CB, Color))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Manager]   MATCH! +1"));
        AddScore(+1);
        ToastDelta(+1);
        CA->HandleMatched(B);
        CB->HandleMatched(A);
        CA->ClearAssignment();
        CB->ClearAssignment();
        Despawn(A);
        Despawn(B);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Manager]   MISMATCH/INVALID! -1 (Despawn both)"));
        AddScore(-1);
        ToastDelta(-1);
        CA->HandleMismatch(B);
        CB->HandleMismatch(A);
        Despawn(A);
        Despawn(B);
    }
}

void AMatchGameManager::AddScore(int32 Delta)
{
    Score += Delta;
    UE_LOG(LogTemp, Warning, TEXT("[Manager] Score = %d (Delta %d)"), Score, Delta);
}

void AMatchGameManager::ToastDelta(int32 Delta)
{
    const FLinearColor Col = (Delta > 0) ? FLinearColor::Green : FLinearColor::Red;
    const FString Msg = (Delta > 0) ? TEXT("+1") : TEXT("-1");
    UKismetSystemLibrary::PrintString(GetWorld(), Msg, true, true, Col, 1.5f);
}

void AMatchGameManager::Despawn(AActor* NPC)
{
    if (!NPC) return;
    UE_LOG(LogTemp, Warning, TEXT("[Manager] Despawn: %s"), *NPC->GetName());
    NPCs.Remove(NPC);
    NPC->Destroy();
}

void AMatchGameManager::ScheduleNextAssign()
{
    const float Delay = FMath::FRandRange(AssignIntervalMin, AssignIntervalMax);
    UE_LOG(LogTemp, Warning, TEXT("[Manager] ScheduleNextAssign in %.2fs"), Delay);
    GetWorldTimerManager().SetTimer(AssignTimer, this, &AMatchGameManager::AssignRandomNPC, Delay, false);
}

void AMatchGameManager::AssignRandomNPC()
{
    TArray<AActor*> IdleInside;
    for (auto& W : NPCs)
    {
        if (AActor* A = W.Get())
        {
            if (!IsInsidePlayArea(A->GetActorLocation())) continue;
            if (UColorMatchComponent* C = GetMatchComp(A))
            {
                if (C->State == EMatchState::Idle)
                {
                    IdleInside.Add(A);
                }
            }
        }
    }

    if (IdleInside.Num() > 0)
    {
        AActor* Pick = IdleInside[FMath::RandRange(0, IdleInside.Num() - 1)];
        if (UColorMatchComponent* C = GetMatchComp(Pick))
        {
            static const EMatchColor COLORS[] = { EMatchColor::Red, EMatchColor::Green, EMatchColor::Blue, EMatchColor::Yellow };
            const EMatchColor Chosen = COLORS[FMath::RandHelper(4)];
            UE_LOG(LogTemp, Warning, TEXT("[Manager] Assign %s -> Color %d Looking=1"), *Pick->GetName(), (uint8)Chosen);
            C->Assign(Chosen, true);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Manager] No idle NPCs inside area to assign."));
    }

    ScheduleNextAssign();
}
