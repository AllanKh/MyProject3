#include "ColorMatchComponent.h"
#include "MatchGameManager.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UColorMatchComponent::UColorMatchComponent()
{
    // allow ticking if we ever implement TickComponent
    PrimaryComponentTick.bCanEverTick = true;
}

void UColorMatchComponent::BeginPlay()
{
    Super::BeginPlay();

    // respect enabled flag for ticking
    SetComponentTickEnabled(bIsComponentEnabled);

    // create debug icon mesh component if enabled in settings
    if (bShouldUseDebugLabel)
    {
        DebugIconComponent = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("MatchDebugIcon"));
        if (DebugIconComponent)
        {
            DebugIconComponent->SetupAttachment(GetOwner()->GetRootComponent());
            DebugIconComponent->RegisterComponent();

            // Position above the NPC
            DebugIconComponent->SetRelativeLocation(IconOffset);
            DebugIconComponent->SetRelativeScale3D(IconScale);

            // purely visual
            DebugIconComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            DebugIconComponent->SetCastShadow(false);

            DebugIconComponent->SetHiddenInGame(true);
        }
    }

    RefreshDebugLabel();
    TryAutoRegisterWithManager();
}


// finds the game manager in the world
AMatchGameManager* UColorMatchComponent::FindGameManager() const
{
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), FoundManagers);
    return FoundManagers.Num() ? Cast<AMatchGameManager>(FoundManagers[0]) : nullptr;
}

// automatically registers this npc with game manager on start
void UColorMatchComponent::TryAutoRegisterWithManager()
{
    if (AMatchGameManager* GameManager = FindGameManager())
    {
        GameManager->RegisterNPC(GetOwner());
    }
}

// assigns color and sets state to looking or idle
void UColorMatchComponent::Assign(EMatchColor NewColor, bool bIsLookingForMatch)
{
    if (!bIsComponentEnabled)
    {
        return;
    }

    CurrentColor = NewColor;
    State = bIsLookingForMatch ? EMatchState::LookingForMatch : EMatchState::Idle;

    // when NPC starts looking for a match, pick a random icon mesh
    if (State == EMatchState::LookingForMatch)
    {
        CurrentIconMesh = GetRandomIconMesh();

        if (DebugIconComponent && CurrentIconMesh)
        {
            DebugIconComponent->SetStaticMesh(CurrentIconMesh);
        }
    }

    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, bIsLookingForMatch);
}


UStaticMesh* UColorMatchComponent::GetRandomIconMesh() const
{
    TArray<UStaticMesh*> Candidates;

    if (Mesh_RPGHero_Sword01)
    {
        Candidates.Add(Mesh_RPGHero_Sword01);
    }
    if (Mesh_AnimalHero_Shield01)
    {
        Candidates.Add(Mesh_AnimalHero_Shield01);
    }
    if (Mesh_TinyHero_Sword02)
    {
        Candidates.Add(Mesh_TinyHero_Sword02);
    }

    if (Candidates.Num() == 0)
    {
        // nothing configured
        return nullptr;
    }

    const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
    return Candidates[Index];
}


// clears color assignment and returns to idle state
void UColorMatchComponent::ClearAssignment()
{
    if (!bIsComponentEnabled)
    {
        return;
    }

    CurrentColor = EMatchColor::None;
    State = EMatchState::Idle;

    CurrentIconMesh = nullptr;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, false);
}


// called when this npc successfully matched with another
void UColorMatchComponent::HandleMatched(AActor* OtherActor)
{
    if (!bIsComponentEnabled)
    {
        return;
    }

    State = EMatchState::Matched;
    CurrentIconMesh = nullptr;
    RefreshDebugLabel();
    OnMatchedWith.Broadcast(OtherActor);
}


// called when this npc collided but colors didnt match
void UColorMatchComponent::HandleMismatch(AActor* OtherActor)
{
    if (!bIsComponentEnabled)
    {
        return;
    }

    State = EMatchState::Dead;
    CurrentIconMesh = nullptr;
    RefreshDebugLabel();
    OnMismatchWith.Broadcast(OtherActor);
}

// converts match color enum to unreal color for debug display
static FColor ConvertMatchColorToFColor(EMatchColor MatchColor)
{
    switch (MatchColor)
    {
    default:
    case EMatchColor::None:
        return FColor::White;
    case EMatchColor::Red:
        return FColor::Red;
    case EMatchColor::Green:
        return FColor::Green;
    case EMatchColor::Blue:
        return FColor::Blue;
    case EMatchColor::Yellow:
        return FColor::Yellow;
    }
}

// converts match color enum to text for debug display
static FText ConvertMatchColorToText(EMatchColor MatchColor)
{
    switch (MatchColor)
    {
    default:
    case EMatchColor::None:
        return FText::FromString(TEXT("NONE"));
    case EMatchColor::Red:
        return FText::FromString(TEXT("RED"));
    case EMatchColor::Green:
        return FText::FromString(TEXT("GREEN"));
    case EMatchColor::Blue:
        return FText::FromString(TEXT("BLUE"));
    case EMatchColor::Yellow:
        return FText::FromString(TEXT("YELLOW"));
    }
}

// updates debug label to show current color if looking for match
void UColorMatchComponent::RefreshDebugLabel()
{
    if (!DebugIconComponent)
    {
        return;
    }

    // only show icon when npc has color and is looking for match
    const bool bShouldShowIcon =
        bIsComponentEnabled &&
        (CurrentColor != EMatchColor::None) &&
        (State == EMatchState::LookingForMatch) &&
        (CurrentIconMesh != nullptr);

    DebugIconComponent->SetHiddenInGame(!bShouldShowIcon);

    if (bShouldShowIcon)
    {
        if (DebugIconComponent->GetStaticMesh() != CurrentIconMesh)
        {
            DebugIconComponent->SetStaticMesh(CurrentIconMesh);
        }
    }
}

