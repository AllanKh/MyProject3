#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GrabbableComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UGrabbableComponent : public UActorComponent, public IInteractable
{
    GENERATED_BODY()

public:
    UGrabbableComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    UPrimitiveComponent* TargetComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    float LinearDamping = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    float LinearStiffness = 750.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    float AngularDamping = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    float AngularStiffness = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    float RotationSpeed = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Grab")
    bool IsBeingGrabbed = false;

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    virtual EInteractCaps GetInteractionCapabilities_Implementation() const override;
    virtual void OnInteractStart_Implementation(const FHitResult& Hit) override;
    virtual void OnInteractUpdate_Implementation(const FInteractUpdate& Update) override;
    virtual void OnInteractEnd_Implementation() override;

private:
    UPROPERTY()
    UPhysicsHandleComponent* PhysicsHandle = nullptr;

    FRotator InitialRotation;
    FRotator RotationOffset;

    void EnsurePhysicsHandle();
    UPrimitiveComponent* GetTargetPrimitive(const FHitResult& Hit);
};