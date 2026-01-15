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

// adds npc to list of managed npcs
void AMatchGameManager::RegisterNPC(AActor* NPCActor)
{
    if (!NPCActor)
    {
        return;
    }
    RegisteredNPCs.AddUnique(NPCActor);
}

// removes npc from list of managed npcs
void AMatchGameManager::UnregisterNPC(AActor* NPCActor)
{
    RegisteredNPCs.Remove(NPCActor);
}

// finds color match component on an actor
UColorMatchComponent* AMatchGameManager::GetColorMatchComponent(AActor* Actor)
{
    return Actor ? Actor->FindComponentByClass<UColorMatchComponent>() : nullptr;
}

// checks if two npcs have matching colors and are both looking for matches
bool AMatchGameManager::DoColorsMatchAndAreBothLooking(UColorMatchComponent* FirstComponent, UColorMatchComponent* SecondComponent, EMatchColor& OutMatchedColor)
{
    OutMatchedColor = EMatchColor::None;

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

// checks if position is inside play area box
bool AMatchGameManager::IsInsidePlayArea(const FVector& WorldPosition) const
{
    FVector BoxCenter = PlayArea->GetComponentLocation();
    FVector BoxExtent = PlayArea->GetScaledBoxExtent();

    // Simple AABB check
    FVector Min = BoxCenter - BoxExtent;
    FVector Max = BoxCenter + BoxExtent;

    return (WorldPosition.X >= Min.X && WorldPosition.X <= Max.X &&
        WorldPosition.Y >= Min.Y && WorldPosition.Y <= Max.Y &&
        WorldPosition.Z >= Min.Z && WorldPosition.Z <= Max.Z);
}

// called when two npcs collide. checks if colors match and awards points accordingly
void AMatchGameManager::HandleNPCVsNPCCollision(AActor* FirstActor, AActor* SecondActor)
{
    if (!FirstActor || !SecondActor)
    {
        return;
    }

    // prevent processing the same actor colliding with itself
    if (FirstActor == SecondActor)
    {
        return;
    }

    UColorMatchComponent* FirstMatchComponent = GetColorMatchComponent(FirstActor);
    UColorMatchComponent* SecondMatchComponent = GetColorMatchComponent(SecondActor);

    if (!FirstMatchComponent || !SecondMatchComponent)
    {
        return;
    }

    // early out if neither is looking
    if (!FirstMatchComponent->IsLooking() && !SecondMatchComponent->IsLooking())
    {
        return;
    }

    // both must be looking for a match/mismatch to occur
    // this prevents the case where one NPC without a color bumps into one with a color
    if (!FirstMatchComponent->IsLooking() || !SecondMatchComponent->IsLooking())
    {
        return;
    }

    // debug logging
    UE_LOG(LogTemp, Warning, TEXT("HandleNPCVsNPCCollision: %s (Color=%d, State=%d) vs %s (Color=%d, State=%d)"),
        *FirstActor->GetName(), (int)FirstMatchComponent->CurrentColor, (int)FirstMatchComponent->State,
        *SecondActor->GetName(), (int)SecondMatchComponent->CurrentColor, (int)SecondMatchComponent->State);

    // log the actual color names for clarity
    auto GetColorName = [](EMatchColor Color) -> FString {
        switch (Color) {
        case EMatchColor::None: return TEXT("None");
        case EMatchColor::Red: return TEXT("Red");
        case EMatchColor::Green: return TEXT("Green");
        case EMatchColor::Blue: return TEXT("Blue");
        case EMatchColor::Yellow: return TEXT("Yellow");
        default: return TEXT("Unknown");
        }
        };

    UE_LOG(LogTemp, Warning, TEXT("  First NPC color: %s, Second NPC color: %s, Same? %s"),
        *GetColorName(FirstMatchComponent->CurrentColor),
        *GetColorName(SecondMatchComponent->CurrentColor),
        (FirstMatchComponent->CurrentColor == SecondMatchComponent->CurrentColor) ? TEXT("YES") : TEXT("NO"));

    EMatchColor MatchedColor;
    if (DoColorsMatchAndAreBothLooking(FirstMatchComponent, SecondMatchComponent, MatchedColor))
    {
        // colors match and both are looking
        UE_LOG(LogTemp, Warning, TEXT("MATCH +1"));
        AddToScore(+1);
        ShowScoreToast(+1);

        FirstMatchComponent->HandleMatched(SecondActor);
        SecondMatchComponent->HandleMatched(FirstActor);
        FirstMatchComponent->ClearAssignment();
        SecondMatchComponent->ClearAssignment();

        // call Blueprint event for successful match
        OnNPCMatch(FirstActor, SecondActor);
    }
    else
    {
        // both are looking but colors mismatch
        UE_LOG(LogTemp, Warning, TEXT("MISMATCH -1"));
        AddToScore(-1);
        ShowScoreToast(-1);

        FirstMatchComponent->HandleMismatch(SecondActor);
        SecondMatchComponent->HandleMismatch(FirstActor);

        // call blueprint event to trigger death
        OnNPCMismatch(FirstActor, SecondActor);
    }
}

// adds or subtracts from score
void AMatchGameManager::AddToScore(int32 ScoreDelta)
{
    Score += ScoreDelta;
}

// shows score change on screen
void AMatchGameManager::ShowScoreToast(int32 ScoreDelta)
{
    const FLinearColor ToastColor = (ScoreDelta > 0) ? FLinearColor::Green : FLinearColor::Red;
    const FString ToastMessage = (ScoreDelta > 0) ? TEXT("+1") : TEXT("-1");
    UKismetSystemLibrary::PrintString(GetWorld(), ToastMessage, true, true, ToastColor, 1.5f);
}

// removes npc from game
//void AMatchGameManager::DespawnNPC(AActor* NPCActor)
//{
//    if (!NPCActor)
//    {
//        return;
//    }
//    RegisteredNPCs.Remove(NPCActor);
//    NPCActor->Destroy();
//}

// sets up timer for next color assignment
void AMatchGameManager::ScheduleNextColorAssignment()
{
    const float RandomDelay = FMath::FRandRange(MinimumAssignInterval, MaximumAssignInterval);
    GetWorldTimerManager().SetTimer(AssignColorTimer, this, &AMatchGameManager::AssignColorToRandomNPC, RandomDelay, false);
}

// picks random idle npc inside play area and gives it a color to look for
void AMatchGameManager::AssignColorToRandomNPC()
{
    // Debug: log play area info
    FVector PlayAreaLocation = PlayArea->GetComponentLocation();
    FVector PlayAreaExtent = PlayArea->GetScaledBoxExtent();
    UE_LOG(LogTemp, Warning, TEXT("Play Area Center: %s, Extent: %s"),
        *PlayAreaLocation.ToString(),
        *PlayAreaExtent.ToString());

    TArray<AActor*> IdleNPCsInsideArea;
    for (auto& WeakNPC : RegisteredNPCs)
    {
        if (AActor* NPCActor = WeakNPC.Get())
        {
            const bool bIsInside = IsInsidePlayArea(NPCActor->GetActorLocation());
            UE_LOG(LogTemp, Warning, TEXT("NPC %s at %s - Inside play area: %s"),
                *NPCActor->GetName(),
                *NPCActor->GetActorLocation().ToString(),
                bIsInside ? TEXT("YES") : TEXT("NO"));

            if (!bIsInside)
            {
                continue;
            }
            if (UColorMatchComponent* MatchComponent = GetColorMatchComponent(NPCActor))
            {
                if (MatchComponent->State == EMatchState::Idle && !MatchComponent->IsOnCooldown())
                {
                    IdleNPCsInsideArea.Add(NPCActor);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Found %d idle NPCs inside play area"), IdleNPCsInsideArea.Num());

    if (IdleNPCsInsideArea.Num() > 0)
    {
        AActor* SelectedNPC = IdleNPCsInsideArea[FMath::RandRange(0, IdleNPCsInsideArea.Num() - 1)];
        if (UColorMatchComponent* MatchComponent = GetColorMatchComponent(SelectedNPC))
        {
            static const EMatchColor AVAILABLE_COLORS[] = { EMatchColor::Red, EMatchColor::Green, EMatchColor::Blue };
            const EMatchColor RandomColor = AVAILABLE_COLORS[FMath::RandHelper(3)];
            UE_LOG(LogTemp, Warning, TEXT("Assigning color %d to NPC %s"), (int)RandomColor, *SelectedNPC->GetName());
            MatchComponent->Assign(RandomColor, true);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No npcs inside area"));
    }

    ScheduleNextColorAssignment();
}