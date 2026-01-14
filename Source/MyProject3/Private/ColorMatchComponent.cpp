#include "ColorMatchComponent.h"
#include "MatchGameManager.h"
#include "../GrabbableComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

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

            // purely visual - no collision or interaction
            DebugIconComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            DebugIconComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
            DebugIconComponent->SetCastShadow(false);
            DebugIconComponent->SetGenerateOverlapEvents(false);
            DebugIconComponent->CanCharacterStepUpOn = ECB_No;
            DebugIconComponent->SetVisibility(true);
            DebugIconComponent->bRenderCustomDepth = false;

            DebugIconComponent->SetHiddenInGame(true);
        }
    }

    RefreshDebugLabel();
    TryAutoRegisterWithManager();
    BindToGrabbableComponent();
}

void UColorMatchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // make the icon mesh face the camera
    if (DebugIconComponent && !DebugIconComponent->bHiddenInGame)
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
                {
                    FVector CameraLocation = CameraManager->GetCameraLocation();
                    FVector IconLocation = DebugIconComponent->GetComponentLocation();

                    // calculate rotation to face camera
                    FVector DirectionToCamera = CameraLocation - IconLocation;
                    DirectionToCamera.Z = 0; // keep upright, only rotate on Z axis

                    if (!DirectionToCamera.IsNearlyZero())
                    {
                        FRotator LookAtRotation = DirectionToCamera.Rotation();
                        // apply the rotation offset
                        LookAtRotation += IconRotationOffset;
                        DebugIconComponent->SetWorldRotation(LookAtRotation);
                    }
                }
            }
        }
    }
}

// binds to GrabbableComponent events if one exists on the same actor
void UColorMatchComponent::BindToGrabbableComponent()
{
    if (AActor* Owner = GetOwner())
    {
        CachedGrabbableComponent = Owner->FindComponentByClass<UGrabbableComponent>();
        if (CachedGrabbableComponent)
        {
            CachedGrabbableComponent->OnGrabbed.AddDynamic(this, &UColorMatchComponent::OnGrabbedCallback);
            CachedGrabbableComponent->OnReleased.AddDynamic(this, &UColorMatchComponent::OnReleasedCallback);
        }
    }
}

// callback when GrabbableComponent fires OnGrabbed
void UColorMatchComponent::OnGrabbedCallback()
{
    SetGrabbed(true);
}

// callback when GrabbableComponent fires OnReleased
void UColorMatchComponent::OnReleasedCallback()
{
    SetGrabbed(false);
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

// sets whether this NPC is currently being grabbed
void UColorMatchComponent::SetGrabbed(bool bGrabbed)
{
    bIsCurrentlyGrabbed = bGrabbed;
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

    // when NPC starts looking for a match, get the mesh for this color
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

void UColorMatchComponent::Reset() {
    State = EMatchState::Idle;
    bIsCurrentlyGrabbed = false;
}