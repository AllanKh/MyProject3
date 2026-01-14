#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MatchCollisionComponent.generated.h"

class UPrimitiveComponent;
class UColorMatchComponent;

// component that detects collisions between npcs and forwards them to game manager for scoring
UCLASS(ClassGroup = (MiniGame), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UMatchCollisionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMatchCollisionComponent();

    // whether this component is currently active (collision & processing enabled)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision")
    bool bIsComponentEnabled = true;

    // which primitive component to use for collision detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision")
    UPrimitiveComponent* CollisionPrimitive = nullptr;

    // how fast npcs must be moving to count as valid collision
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision", meta = (ClampMin = "0"))
    float MinimumHitSpeed = 20.f;

    // whether both npcs must be inside play area for collision to count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision")
    bool bMustBeInsidePlayArea = false;

    // whether to log collision setup info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision|Debug")
    bool bShouldDebugLog = true;

    // whether to bind all primitives on actor or just the specified one
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision|Debug")
    bool bShouldBindAllPrimitivesForDebug = true;

protected:
    virtual void BeginPlay() override;

private:
    // collision event handlers
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& HitResult);

    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
        FVector NormalImpulse, const FHitResult& HitResult);

    UFUNCTION()
    void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    // checks if collision meets requirements to be scored
    bool ShouldScoreThisHit(AActor* OtherActor, float& OutRelativeSpeed, bool& bIsMyActorLooking, bool& bIsOtherActorLooking,
        uint8& MyActorColor, uint8& OtherActorColor, bool& bAreBothActorsInsideArea) const;

    // sends valid collision to game manager
    void ForwardCollisionToManager(AActor* SelfActor, AActor* OtherActor) const;

    // finds color match component on actor
    UColorMatchComponent* GetColorMatchComponent(AActor* Actor) const;

    // sets up collision events for one primitive
    void BindEventsForOnePrimitive(UPrimitiveComponent* Primitive);

    // logs collision settings for debugging
    void LogComponentSetup(UPrimitiveComponent* Primitive, const TCHAR* LogTag) const;
};
