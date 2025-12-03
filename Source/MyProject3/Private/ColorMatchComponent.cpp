#include "ColorMatchComponent.h"
#include "MatchGameManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"



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

        // emoji billboard (for match / mismatch)
        EmojiComponent = NewObject<UBillboardComponent>(GetOwner(), TEXT("MatchEmoji"));
        if (EmojiComponent)
        {
            EmojiComponent->SetupAttachment(GetOwner()->GetRootComponent());
            EmojiComponent->RegisterComponent();

            EmojiComponent->SetRelativeLocation(EmojiOffset);
            EmojiComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            EmojiComponent->SetHiddenInGame(true);
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

    // when NPC starts looking for a match, pick the mesh for this color
    if (State == EMatchState::LookingForMatch)
    {
        CurrentIconMesh = GetIconMeshForColor(CurrentColor);

        if (DebugIconComponent && CurrentIconMesh)
        {
            DebugIconComponent->SetStaticMesh(CurrentIconMesh);
        }
    }

    RefreshDebugLabel();
    OnAssignmentChanged.Broadcast(CurrentColor, bIsLookingForMatch);
}

// map each color to a specific mesh
UStaticMesh* UColorMatchComponent::GetIconMeshForColor(EMatchColor Color) const
{
    switch (Color)
    {
    case EMatchColor::Red:
        return Mesh_RPGHero_Sword01;

    case EMatchColor::Green:
        return Mesh_AnimalHero_Shield01;

    case EMatchColor::Blue:
        return Mesh_TinyHero_Sword02;

    default:
        return nullptr;
    }
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

    // hide emoji when going back to idle
    if (EmojiComponent)
    {
        EmojiComponent->SetHiddenInGame(true);
    }

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

    // freeze this npc for 3 seconds while the emoji shows
    FreezeOwnerForSeconds(3.0f);

    // hide the color icon
    CurrentIconMesh = nullptr;
    RefreshDebugLabel();

    // show the match emoji
    if (EmojiComponent && MatchEmojiTexture)
    {
        EmojiComponent->SetSprite(MatchEmojiTexture);
        EmojiComponent->SetHiddenInGame(false);

        // hide after 3 seconds
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateUObject(this, &UColorMatchComponent::HideEmoji),
            4.0f,
            false
        );
    }

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

    // freeze this npc for 3 seconds as well
    FreezeOwnerForSeconds(3.0f);

    // hide the color icon
    CurrentIconMesh = nullptr;
    RefreshDebugLabel();

    // show the mismatch emoji
    if (EmojiComponent && MismatchEmojiTexture)
    {
        EmojiComponent->SetSprite(MismatchEmojiTexture);
        EmojiComponent->SetHiddenInGame(false);

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateUObject(this, &UColorMatchComponent::HideEmoji),
            3.0f,
            false
        );
    }

    OnMismatchWith.Broadcast(OtherActor);
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

void UColorMatchComponent::HideEmoji()
{
    if (EmojiComponent)
    {
        EmojiComponent->SetHiddenInGame(true);
    }
}

void UColorMatchComponent::FreezeOwnerForSeconds(float Seconds)
{
    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner)
    {
        return;
    }

    if (UCharacterMovementComponent* MoveComp = CharacterOwner->GetCharacterMovement())
    {
        // Disable movement
        MoveComp->DisableMovement();

        // Set a timer to unfreeze 
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle,
            FTimerDelegate::CreateUObject(this, &UColorMatchComponent::UnfreezeOwner),
            Seconds,
            false
        );
    }
}

void UColorMatchComponent::UnfreezeOwner()
{
    ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
    if (!CharacterOwner)
    {
        return;
    }

    if (UCharacterMovementComponent* MoveComp = CharacterOwner->GetCharacterMovement())
    {
        MoveComp->SetMovementMode(MOVE_Walking);
    }
}