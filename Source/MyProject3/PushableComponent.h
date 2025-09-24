#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interactable.h"
#include "PushableComponent.generated.h"

// component that lets objects be pushed around with mouse drag using spring physics
// add this on object to make it pushable
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UPushableComponent : public UActorComponent, public IInteractable
{
    GENERATED_BODY()

public:
    UPushableComponent();

    // specific component to push. if null uses root component
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Push Settings")
    UPrimitiveComponent* TargetComponent = nullptr;

    // how hard the spring pulls object toward cursor. higher gives stiffer/faster response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Push Settings")
    float SpringStrength = 50000.f;

    // how much the damper resists movement. higher gives less bouncy/more stable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Push Settings")
    float DamperStrength = 5000.f;

    // max force. stop objects from going crazy
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Push Settings")
    float MaxForceLimit = 500000.f;

    // interface implementation. tells system this supports pushing
    virtual EInteractCaps GetInteractionCapabilities_Implementation() const override;

    // interface implementation. applies spring force every frame while being pushed
    virtual void OnInteractUpdate_Implementation(const FInteractUpdate& Update) override;

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};