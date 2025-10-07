#include "ColorMatchComponent.h"
#include "MatchGameManager.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

UColorMatchComponent::UColorMatchComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UColorMatchComponent::BeginPlay()
{
    Super::BeginPlay();

    // create debug text component if enabled
    if (bShouldUseDebugLabel)
    {
        DebugTextComponent = NewObject<UTextRenderComponent>(GetOwner(), TEXT("MatchDebugText"));
        if (DebugTextComponent)
        {
            DebugTextComponent->SetupAttachment(GetOwner()->GetRootComponent());
            DebugTextComponent->RegisterComponent();
            DebugTextComponent->SetHorizontalAlignment(EHTA_Center);
            DebugTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
            DebugTextComponent->SetWorldSize(48.f);
            DebugTextComponent->SetTextRenderColor(FColor::White);
            DebugTextComponent->SetRelativeLocation(FVector(0, 0, 120));
            DebugTextComponent->SetHiddenInGame(true);
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
    CurrentColor = NewColor;
    State = bIsLookingForMatch ? EMatchState::LookingForMatch : EMatchState::Idle;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, bIsLookingForMatch);
}

// clears color assignment and returns to idle state
void UColorMatchComponent::ClearAssignment()
{
    CurrentColor = EMatchColor::None;
    State = EMatchState::Idle;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, false);
}

// called when this npc successfully matched with another
void UColorMatchComponent::HandleMatched(AActor* OtherActor)
{
    State = EMatchState::Matched;
    RefreshDebugLabel();
    OnMatchedWith.Broadcast(OtherActor);
}

// called when this npc collided but colors didnt match
void UColorMatchComponent::HandleMismatch(AActor* OtherActor)
{
    State = EMatchState::Dead;
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
    if (!DebugTextComponent)
    {
        return;
    }

    // only show label when npc has color and is looking for match
    const bool bShouldShowLabel = (CurrentColor != EMatchColor::None) && (State == EMatchState::LookingForMatch);
    DebugTextComponent->SetHiddenInGame(!bShouldShowLabel);

    if (bShouldShowLabel)
    {
        DebugTextComponent->SetText(ConvertMatchColorToText(CurrentColor));
        DebugTextComponent->SetTextRenderColor(ConvertMatchColorToFColor(CurrentColor));
    }
}