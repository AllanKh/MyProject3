#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MatchCollisionComponent.generated.h"

class UPrimitiveComponent;
class UColorMatchComponent;

UCLASS(ClassGroup = (MiniGame), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UMatchCollisionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMatchCollisionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision")
    UPrimitiveComponent* CollisionPrimitive = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision", meta = (ClampMin = "0"))
    float MinimumHitSpeed = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision")
    bool bMustBeInsidePlayArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision|Debug")
    bool bShouldDebugLog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision|Debug")
    bool bShouldBindAllPrimitivesForDebug = true;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& HitResult);

    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
        FVector NormalImpulse, const FHitResult& HitResult);

    UFUNCTION()
    void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    bool ShouldScoreThisHit(AActor* OtherActor, float& OutRelativeSpeed, bool& bIsMyActorLooking, bool& bIsOtherActorLooking,
        uint8& MyActorColor, uint8& OtherActorColor, bool& bAreBothActorsInsideArea) const;

    void ForwardCollisionToManager(AActor* SelfActor, AActor* OtherActor) const;

    UColorMatchComponent* GetColorMatchComponent(AActor* Actor) const;

    void BindEventsForOnePrimitive(UPrimitiveComponent* Primitive);
    void LogComponentSetup(UPrimitiveComponent* Primitive, const TCHAR* LogTag) const;
};