#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Interactable.h"
#include "GrabbableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrabbedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleasedEvent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UGrabbableComponent : public UActorComponent, public IInteractable
{
    GENERATED_BODY()

public:
    UGrabbableComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    UPrimitiveComponent* TargetComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float LinearDamping = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float LinearStiffness = 750.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float AngularDamping = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Physics")
    float AngularStiffness = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Rotation")
    float RotationSpeed = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Grab")
    bool IsBeingGrabbed = false;

    UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
    FOnGrabbedEvent OnGrabbed;

    UPROPERTY(BlueprintAssignable, Category = "Grab|Events")
    FOnReleasedEvent OnReleased;

    virtual EInteractCaps GetInteractionCapabilities_Implementation() const override;
    virtual void OnInteractStart_Implementation(const FHitResult& Hit) override;
    virtual void OnInteractUpdate_Implementation(const FInteractUpdate& Update) override;
    virtual void OnInteractEnd_Implementation() override;

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    UPhysicsHandleComponent* PhysicsHandle = nullptr;

    FRotator InitialRotation;
    FRotator RotationOffset;

    void EnsurePhysicsHandle();
    UPrimitiveComponent* GetTargetPrimitive(const FHitResult& Hit);
};