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

    if (bUseDebugLabel)
    {
        DebugText = NewObject<UTextRenderComponent>(GetOwner(), TEXT("MatchDebugText"));
        if (DebugText)
        {
            DebugText->SetupAttachment(GetOwner()->GetRootComponent());
            DebugText->RegisterComponent();
            DebugText->SetHorizontalAlignment(EHTA_Center);
            DebugText->SetVerticalAlignment(EVRTA_TextCenter);
            DebugText->SetWorldSize(48.f);
            DebugText->SetTextRenderColor(FColor::White);
            DebugText->SetRelativeLocation(FVector(0, 0, 120));
            DebugText->SetHiddenInGame(true);
        }
    }

    RefreshDebugLabel();
    TryAutoRegister();
}

AMatchGameManager* UColorMatchComponent::FindManager() const
{
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMatchGameManager::StaticClass(), Found);
    return Found.Num() ? Cast<AMatchGameManager>(Found[0]) : nullptr;
}

void UColorMatchComponent::TryAutoRegister()
{
    if (AMatchGameManager* M = FindManager())
    {
        M->RegisterNPC(GetOwner());
    }
}

void UColorMatchComponent::Assign(EMatchColor NewColor, bool bLookingForMatch)
{
    CurrentColor = NewColor;
    State = bLookingForMatch ? EMatchState::LookingForMatch : EMatchState::Idle;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, bLookingForMatch);
}

void UColorMatchComponent::ClearAssignment()
{
    CurrentColor = EMatchColor::None;
    State = EMatchState::Idle;
    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, false);
}

void UColorMatchComponent::HandleMatched(AActor* Other)
{
    State = EMatchState::Matched;
    RefreshDebugLabel();
    OnMatchedWith.Broadcast(Other);
}

void UColorMatchComponent::HandleMismatch(AActor* Other)
{
    State = EMatchState::Dead;
    RefreshDebugLabel();
    OnMismatchWith.Broadcast(Other);
}

static FColor ToFColor(EMatchColor C)
{
    switch (C)
    {
    default:
    case EMatchColor::None:   return FColor::White;
    case EMatchColor::Red:    return FColor::Red;
    case EMatchColor::Green:  return FColor::Green;
    case EMatchColor::Blue:   return FColor::Blue;
    case EMatchColor::Yellow: return FColor::Yellow;
    }
}

static FText ToText(EMatchColor C)
{
    switch (C)
    {
    default:
    case EMatchColor::None:   return FText::FromString(TEXT("NONE"));
    case EMatchColor::Red:    return FText::FromString(TEXT("RED"));
    case EMatchColor::Green:  return FText::FromString(TEXT("GREEN"));
    case EMatchColor::Blue:   return FText::FromString(TEXT("BLUE"));
    case EMatchColor::Yellow: return FText::FromString(TEXT("YELLOW"));
    }
}

void UColorMatchComponent::RefreshDebugLabel()
{
    if (!DebugText) return;

    const bool bShow = (CurrentColor != EMatchColor::None) && (State == EMatchState::LookingForMatch);
    DebugText->SetHiddenInGame(!bShow);

    if (bShow)
    {
        DebugText->SetText(ToText(CurrentColor));
        DebugText->SetTextRenderColor(ToFColor(CurrentColor));
    }
}
