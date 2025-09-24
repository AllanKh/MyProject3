#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// types of interactions an object supports
UENUM(BlueprintType)
enum class EInteractCaps : uint8
{
    None = 0,  // cant be interacted with
    Push = 1,  // can be pushed
    Grab = 2   // can be grabbed
};

// all the data needed during interaction updates. passed to objects every frame while being interacted with
USTRUCT(BlueprintType)
struct FInteractUpdate
{
    GENERATED_BODY()

    // point in 3d space where the raycast hits the object
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    FVector WorldHitLocation = FVector::ZeroVector;

    // direction the surface is facing at the hit point
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    FVector WorldHitNormal = FVector::UpVector;

    // cursor position in world space
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    FVector WorldTargetPoint = FVector::ZeroVector;

    // how many pixel mouse moved on screen during current frame
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    FVector2D ScreenDelta = FVector2D::ZeroVector;

    // time since last frame
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    float DeltaSeconds = 0.f;

    // check if player holding RMB to activate rotation
    UPROPERTY(BlueprintReadWrite, Category = "Interaction")
    bool bAltRotate = false;
};

// UE stuff
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
    GENERATED_BODY()
};

// interface for objects that can be interacted with using the mouse cursor
// implement these functions to make grabbable/pushable/interactive objects
class MYPROJECT3_API IInteractable
{
    GENERATED_BODY()

public:
    // returns what type of interaction object supports
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    EInteractCaps GetInteractionCapabilities() const;

    // called when mouse starts hovering over object
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnHoverBegin();

    // called when mouse stops hovering
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnHoverEnd();

    // called when player clicks on object. hit tells where raycast hit
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractStart(const FHitResult& Hit);

    // called every frame while interacting. do grab/push logic here
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractUpdate(const FInteractUpdate& Update);

    // called when player releases mouse. cleanup physics handles/forces
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void OnInteractEnd();
};