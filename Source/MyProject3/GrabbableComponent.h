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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab Settings")
    UPrimitiveComponent* TargetComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab Settings")
    float LinearStiffness = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab Settings")
    float LinearDamping = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab Settings")
    float AngularStiffness = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab Settings")
    float AngularDamping = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab Settings")
    float RotationSpeed = 100.f;

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

    FRotator InitialRotation = FRotator::ZeroRotator;
    FRotator RotationOffset = FRotator::ZeroRotator;

    void EnsurePhysicsHandle();
    UPrimitiveComponent* GetTargetPrimitive(const FHitResult& Hit);
};