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

AMatchGameManager* UColorMatchComponent::FindGameManager() const
{
    TArray<AActor*> FoundManagers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), FoundManagers);
    return FoundManagers.Num() ? Cast<AMatchGameManager>(FoundManagers[0]) : nullptr;
}

void UColorMatchComponent::TryAutoRegisterWithManager()
{
    if (AMatchGameManager* GameManager = FindGameManager())
    {
        GameManager->RegisterNPC(GetOwner());
    }
}

void UColorMatchComponent::Assign(EMatchColor NewColor, bool bIsLookingForMatch)
{
    CurrentColor = NewColor;
    State = bIsLookingForMatch ? EMatchState::LookingForMatch : EMatchState::Idle;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, bIsLookingForMatch);
}

void UColorMatchComponent::ClearAssignment()
{
    CurrentColor = EMatchColor::None;
    State = EMatchState::Idle;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, false);
}

void UColorMatchComponent::HandleMatched(AActor* OtherActor)
{
    State = EMatchState::Matched;
    RefreshDebugLabel();
    OnMatchedWith.Broadcast(OtherActor);
}

void UColorMatchComponent::HandleMismatch(AActor* OtherActor)
{
    State = EMatchState::Dead;
    RefreshDebugLabel();
    OnMismatchWith.Broadcast(OtherActor);
}

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

void UColorMatchComponent::RefreshDebugLabel()
{
    if (!DebugTextComponent) 
    {
        return;
    }

    const bool bShouldShowLabel = (CurrentColor != EMatchColor::None) && (State == EMatchState::LookingForMatch);
    DebugTextComponent->SetHiddenInGame(!bShouldShowLabel);

    if (bShouldShowLabel)
    {
        DebugTextComponent->SetText(ConvertMatchColorToText(CurrentColor));
        DebugTextComponent->SetTextRenderColor(ConvertMatchColorToFColor(CurrentColor));
    }
}