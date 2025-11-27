#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable.h"
#include "InteractorComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UInteractorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractorComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float TraceDistance = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InitialCursorDepth = 400.f;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InputInteractPressed();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InputInteractReleased();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InputAltRotatePressed();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InputAltRotateReleased();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InputScrollUp();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InputScrollDown();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    TScriptInterface<IInteractable> CurrentHover;
    TScriptInterface<IInteractable> ActiveInteract;
    FHitResult CachedHit;
    FVector2D LastMousePos = FVector2D::ZeroVector;
    bool bAltRotateHeld = false;

    float CurrentDepth = 400.f;

    bool GetMouseRay(FVector& OutOrigin, FVector& OutDirection) const;

    bool TraceFromMouse(FHitResult& OutHit);

    // calculates where in 3D space the cursor should place objects based on scroll depth
    FVector GetTargetPointInWorld(const FVector& RayOrigin, const FVector& RayDirection) const;

    // handles hover state changes. checks if mouse is over interactable object
    void UpdateHover(const FHitResult& Hit);
};
