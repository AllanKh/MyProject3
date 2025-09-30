#include "InteractorComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UInteractorComponent::UInteractorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInteractorComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentDepth = InitialCursorDepth;
    UE_LOG(LogTemp, Warning, TEXT("InteractorComponent BeginPlay - Component initialized"));
}

void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FHitResult Hit;
    if (TraceFromMouse(Hit))
    {
        CachedHit = Hit;
        UpdateHover(Hit);
    }
    else
    {
        if (CurrentHover.GetObject())
        {
            IInteractable::Execute_OnHoverEnd(CurrentHover.GetObject());
            CurrentHover.SetObject(nullptr);
            CurrentHover.SetInterface(nullptr);
        }
        CachedHit = FHitResult();
    }

    if (ActiveInteract.GetObject())
    {
        FVector RayOrigin, RayDir;
        if (GetMouseRay(RayOrigin, RayDir))
        {
            APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
            if (PC)
            {
                FVector2D MousePos;
                PC->GetMousePosition(MousePos.X, MousePos.Y);

                FInteractUpdate Update;
                Update.WorldHitLocation = CachedHit.ImpactPoint;
                Update.WorldHitNormal = CachedHit.ImpactNormal;
                Update.WorldTargetPoint = GetTargetPointInWorld(RayOrigin, RayDir);
                Update.ScreenDelta = MousePos - LastMousePos;
                Update.DeltaSeconds = DeltaTime;
                Update.bAltRotate = bAltRotateHeld;

                IInteractable::Execute_OnInteractUpdate(ActiveInteract.GetObject(), Update);
                LastMousePos = MousePos;
            }
        }
    }
}

bool UInteractorComponent::GetMouseRay(FVector& OutOrigin, FVector& OutDirection) const
{
    APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
    if (!PC) return false;

    FVector2D MousePos;
    if (!PC->GetMousePosition(MousePos.X, MousePos.Y)) return false;

    FVector WorldPos, WorldDir;
    if (PC->DeprojectScreenPositionToWorld(MousePos.X, MousePos.Y, WorldPos, WorldDir))
    {
        OutOrigin = WorldPos;
        OutDirection = WorldDir.GetSafeNormal();
        return true;
    }
    return false;
}

// UPDATED: try your channel first, then fallback to object-type trace that includes ECC_Pawn
bool UInteractorComponent::TraceFromMouse(FHitResult& OutHit)
{
    FVector Origin, Direction;
    if (!GetMouseRay(Origin, Direction)) return false;

    const FVector End = Origin + Direction * TraceDistance;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    // 1) Primary: project-configured trace channel (e.g., Visibility)
    bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Origin, End, TraceChannel, Params);

    // 2) Fallback: object-type trace that includes Pawns (Character capsules)
    if (!bHit)
    {
        FCollisionObjectQueryParams ObjParams;
        ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
        ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
        ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);
        ObjParams.AddObjectTypesToQuery(ECC_Pawn); // key for Characters

        bHit = GetWorld()->LineTraceSingleByObjectType(OutHit, Origin, End, ObjParams, Params);
    }

    DrawDebugLine(GetWorld(), Origin, End, bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 0.2f);
    if (bHit)
    {
        DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 5.f, 8, FColor::Yellow, false, 0.1f);
    }

    return bHit;
}

FVector UInteractorComponent::GetTargetPointInWorld(const FVector& RayOrigin, const FVector& RayDirection) const
{
    return RayOrigin + RayDirection * CurrentDepth;
}

void UInteractorComponent::UpdateHover(const FHitResult& Hit)
{
    UObject* NewHoverObject = nullptr;

    if (Hit.GetComponent() && Hit.GetComponent()->Implements<UInteractable>())
    {
        NewHoverObject = Hit.GetComponent();
        UE_LOG(LogTemp, Warning, TEXT("Found interactable COMPONENT: %s"), *NewHoverObject->GetName());
    }
    else if (Hit.GetActor() && Hit.GetActor()->Implements<UInteractable>())
    {
        NewHoverObject = Hit.GetActor();
        UE_LOG(LogTemp, Warning, TEXT("Found interactable ACTOR: %s"), *NewHoverObject->GetName());
    }
    else if (Hit.GetActor())
    {
        TArray<UActorComponent*> Components = Hit.GetActor()->GetComponentsByInterface(UInteractable::StaticClass());
        if (Components.Num() > 0)
        {
            NewHoverObject = Components[0];
        }
    }

    if (NewHoverObject != CurrentHover.GetObject())
    {
        if (CurrentHover.GetObject())
        {
            IInteractable::Execute_OnHoverEnd(CurrentHover.GetObject());
        }

        CurrentHover.SetObject(NewHoverObject);
        CurrentHover.SetInterface(NewHoverObject ? Cast<IInteractable>(NewHoverObject) : nullptr);

        if (CurrentHover.GetObject())
        {
            IInteractable::Execute_OnHoverBegin(CurrentHover.GetObject());
        }
    }
}

void UInteractorComponent::InputInteractPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("InputInteractPressed called"));

    if (ActiveInteract.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("Already have active interact, ignoring"));
        return;
    }

    if (CurrentHover.GetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("Starting interact on: %s"), *CurrentHover.GetObject()->GetName());
        ActiveInteract = CurrentHover;

        APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
        if (PC)
        {
            PC->GetMousePosition(LastMousePos.X, LastMousePos.Y);
        }

        FVector RayOrigin, RayDir;
        if (GetMouseRay(RayOrigin, RayDir) && CachedHit.bBlockingHit)
        {
            CurrentDepth = FVector::Dist(RayOrigin, CachedHit.ImpactPoint);
            UE_LOG(LogTemp, Warning, TEXT("Set grab depth to: %f"), CurrentDepth);
        }

        IInteractable::Execute_OnInteractStart(ActiveInteract.GetObject(), CachedHit);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No hover object to interact with"));
    }
}

void UInteractorComponent::InputInteractReleased()
{
    UE_LOG(LogTemp, Warning, TEXT("InputInteractReleased called"));

    if (ActiveInteract.GetObject())
    {
        IInteractable::Execute_OnInteractEnd(ActiveInteract.GetObject());
        ActiveInteract.SetObject(nullptr);
        ActiveInteract.SetInterface(nullptr);
    }
}

void UInteractorComponent::InputAltRotatePressed()
{
    bAltRotateHeld = true;
    UE_LOG(LogTemp, Warning, TEXT("Alt Rotate Pressed"));
}

void UInteractorComponent::InputAltRotateReleased()
{
    bAltRotateHeld = false;
    UE_LOG(LogTemp, Warning, TEXT("Alt Rotate Released"));
}

void UInteractorComponent::InputScrollUp()
{
    CurrentDepth = FMath::Clamp(CurrentDepth + 50.f, 50.f, 3000.f);
    UE_LOG(LogTemp, Warning, TEXT("Scroll UP: depth=%f (closer)"), CurrentDepth);
}

void UInteractorComponent::InputScrollDown()
{
    CurrentDepth = FMath::Clamp(CurrentDepth - 50.f, 50.f, 3000.f);
    UE_LOG(LogTemp, Warning, TEXT("Scroll DOWN: depth=%f (farther)"), CurrentDepth);
}
