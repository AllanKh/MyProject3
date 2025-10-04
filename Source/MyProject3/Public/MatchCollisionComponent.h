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
    UPrimitiveComponent* HitPrimitive = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision", meta = (ClampMin = "0"))
    float MinHitSpeed = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision")
    bool bRequireInsidePlayArea = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision|Debug")
    bool bDebugLog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Collision|Debug")
    bool bBindAllPrimitivesForDebug = true;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    bool ShouldScoreThisHit(AActor* OtherActor, float& OutRelSpeed, bool& bMyLooking, bool& bOtherLooking,
        uint8& MyColor, uint8& OtherColor, bool& bInsideAreaBoth) const;

    void ForwardToManager(AActor* SelfActor, AActor* OtherActor) const;

    UColorMatchComponent* GetMatch(AActor* Act) const;

    void BindOnePrimitive(UPrimitiveComponent* Prim);
    void LogComponentSetup(UPrimitiveComponent* Prim, const TCHAR* Tag) const;
};
