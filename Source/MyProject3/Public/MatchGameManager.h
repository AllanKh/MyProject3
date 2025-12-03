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
    float MinimumAssignInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Rules")
    float MaximumAssignInterval = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Match|NPC")
    void RegisterNPC(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "Match|NPC")
    void UnregisterNPC(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "Match|NPC")
    void HandleNPCVsNPCCollision(AActor* FirstActor, AActor* SecondActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Match|NPC")
    void OnNPCMatch(AActor* FirstActor, AActor* SecondActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Match|NPC")
    void OnNPCMismatch(AActor* FirstActor, AActor* SecondActor);

    bool IsInsidePlayArea(const FVector& WorldPosition) const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredNPCs;

    FTimerHandle AssignColorTimer;

    void ScheduleNextColorAssignment();
    void AssignColorToRandomNPC();

    static UColorMatchComponent* GetColorMatchComponent(AActor* Actor);
    static bool DoColorsMatchAndAreBothLooking(UColorMatchComponent* FirstComponent, UColorMatchComponent* SecondComponent, EMatchColor& OutMatchedColor);
    void AddToScore(int32 ScoreDelta);
    void ShowScoreToast(int32 ScoreDelta);
    void DespawnNPC(AActor* NPCActor);
};