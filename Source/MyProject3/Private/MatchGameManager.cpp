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
    ScheduleNextColorAssignment();
}

void AMatchGameManager::RegisterNPC(AActor* NPCActor)
{
    if (!NPCActor) 
    {
        return;
    }
    RegisteredNPCs.AddUnique(NPCActor);
}

void AMatchGameManager::UnregisterNPC(AActor* NPCActor)
{
    RegisteredNPCs.Remove(NPCActor);
}

UColorMatchComponent* AMatchGameManager::GetColorMatchComponent(AActor* Actor)
{
    return Actor ? Actor->FindComponentByClass<UColorMatchComponent>() : nullptr;
}

bool AMatchGameManager::DoColorsMatchAndAreBothLooking(UColorMatchComponent* FirstComponent, UColorMatchComponent* SecondComponent, EMatchColor& OutMatchedColor)
{
    if (!FirstComponent || !SecondComponent) 
    {
        return false;
    }
    if (!FirstComponent->IsLooking() || !SecondComponent->IsLooking()) 
    {
        return false;
    }
    if (FirstComponent->CurrentColor == EMatchColor::None || SecondComponent->CurrentColor == EMatchColor::None)
    {
        return false;
    }
    const bool bColorsAreSame = (FirstComponent->CurrentColor == SecondComponent->CurrentColor);
    if (bColorsAreSame) 
    {
        OutMatchedColor = FirstComponent->CurrentColor;
    }
    return bColorsAreSame;
}

bool AMatchGameManager::IsInsidePlayArea(const FVector& WorldPosition) const
{
    return UKismetMathLibrary::IsPointInBoxWithTransform(WorldPosition, PlayArea->GetComponentTransform(), PlayArea->GetScaledBoxExtent());
}

void AMatchGameManager::HandleNPCVsNPCCollision(AActor* FirstActor, AActor* SecondActor)
{
    if (!FirstActor || !SecondActor) 
    {
        return;
    }

    UColorMatchComponent* FirstMatchComponent = GetColorMatchComponent(FirstActor);
    UColorMatchComponent* SecondMatchComponent = GetColorMatchComponent(SecondActor);

    if (!FirstMatchComponent || !SecondMatchComponent) 
    {
        return;
    }


    EMatchColor MatchedColor;
    if (DoColorsMatchAndAreBothLooking(FirstMatchComponent, SecondMatchComponent, MatchedColor))
    {
        UE_LOG(LogTemp, Warning, TEXT("MATCH +1"));
        AddToScore(+1);
        ShowScoreToast(+1);
        FirstMatchComponent->HandleMatched(SecondActor);
        SecondMatchComponent->HandleMatched(FirstActor);
        FirstMatchComponent->ClearAssignment();
        SecondMatchComponent->ClearAssignment();
        DespawnNPC(FirstActor);
        DespawnNPC(SecondActor);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MISMATCH -1"));
        AddToScore(-1);
        ShowScoreToast(-1);
        FirstMatchComponent->HandleMismatch(SecondActor);
        SecondMatchComponent->HandleMismatch(FirstActor);
        DespawnNPC(FirstActor);
        DespawnNPC(SecondActor);
    }
}

void AMatchGameManager::AddToScore(int32 ScoreDelta)
{
    Score += ScoreDelta;
}

void AMatchGameManager::ShowScoreToast(int32 ScoreDelta)
{
    const FLinearColor ToastColor = (ScoreDelta > 0) ? FLinearColor::Green : FLinearColor::Red;
    const FString ToastMessage = (ScoreDelta > 0) ? TEXT("+1") : TEXT("-1");
    UKismetSystemLibrary::PrintString(GetWorld(), ToastMessage, true, true, ToastColor, 1.5f);
}

void AMatchGameManager::DespawnNPC(AActor* NPCActor)
{
    if (!NPCActor) 
    {
        return;
    }
    RegisteredNPCs.Remove(NPCActor);
    NPCActor->Destroy();
}

void AMatchGameManager::ScheduleNextColorAssignment()
{
    const float RandomDelay = FMath::FRandRange(MinimumAssignInterval, MaximumAssignInterval);
    GetWorldTimerManager().SetTimer(AssignColorTimer, this, &AMatchGameManager::AssignColorToRandomNPC, RandomDelay, false);
}

void AMatchGameManager::AssignColorToRandomNPC()
{
    TArray<AActor*> IdleNPCsInsideArea;
    for (auto& WeakNPC : RegisteredNPCs)
    {
        if (AActor* NPCActor = WeakNPC.Get())
        {
            if (!IsInsidePlayArea(NPCActor->GetActorLocation())) 
            {
                continue;
            }
            if (UColorMatchComponent* MatchComponent = GetColorMatchComponent(NPCActor))
            {
                if (MatchComponent->State == EMatchState::Idle)
                {
                    IdleNPCsInsideArea.Add(NPCActor);
                }
            }
        }
    }

    if (IdleNPCsInsideArea.Num() > 0)
    {
        AActor* SelectedNPC = IdleNPCsInsideArea[FMath::RandRange(0, IdleNPCsInsideArea.Num() - 1)];
        if (UColorMatchComponent* MatchComponent = GetColorMatchComponent(SelectedNPC))
        {
            static const EMatchColor AVAILABLE_COLORS[] = { EMatchColor::Red, EMatchColor::Green, EMatchColor::Blue, EMatchColor::Yellow };
            const EMatchColor RandomColor = AVAILABLE_COLORS[FMath::RandHelper(4)];
            MatchComponent->Assign(RandomColor, true);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No npcs inside area"));
    }

    ScheduleNextColorAssignment();
}