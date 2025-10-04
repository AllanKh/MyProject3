#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EMatchColor.h"
#include "MatchGameManager.generated.h"

class UBoxComponent;
class UUserWidget;
class UColorMatchComponent;

UCLASS()
class MYPROJECT3_API AMatchGameManager : public AActor
{
    GENERATED_BODY()

public:
    AMatchGameManager();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match|Area")
    UBoxComponent* PlayArea;

    UPROPERTY(BlueprintReadOnly, Category = "Match|Score")
    int32 Score = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match|UI")
    TSubclassOf<UUserWidget> ScoreToastClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Rules")
    float AssignIntervalMin = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Rules")
    float AssignIntervalMax = 4.0f;

    UFUNCTION(BlueprintCallable, Category = "Match|NPC")
    void RegisterNPC(AActor* NPC);

    UFUNCTION(BlueprintCallable, Category = "Match|NPC")
    void UnregisterNPC(AActor* NPC);

    UFUNCTION(BlueprintCallable, Category = "Match|Rules")
    void HandleNPCVsNPCCollision(AActor* A, AActor* B);

    // MADE PUBLIC so collision component can query it
    bool IsInsidePlayArea(const FVector& WorldPos) const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> NPCs;

    FTimerHandle AssignTimer;

    void ScheduleNextAssign();
    void AssignRandomNPC();

    static UColorMatchComponent* GetMatchComp(AActor* Actor);
    static bool ColorsMatchAndBothLooking(UColorMatchComponent* A, UColorMatchComponent* B, EMatchColor& Out);
    void AddScore(int32 Delta);
    void ToastDelta(int32 Delta);
    void Despawn(AActor* NPC);
};
