#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EMatchColor.h"
#include "ColorMatchComponent.generated.h"

class UTextRenderComponent;
class AMatchGameManager;

UENUM(BlueprintType)
enum class EMatchState : uint8
{
    Idle,
    LookingForMatch,
    Matched,
    Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchAssignmentChanged, EMatchColor, NewColor, bool, bIsLookingForMatch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchedWith, AActor*, OtherActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMismatchWith, AActor*, OtherActor);

UCLASS(ClassGroup = (MiniGame), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UColorMatchComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UColorMatchComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
    EMatchColor CurrentColor = EMatchColor::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
    EMatchState State = EMatchState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Debug")
    bool bShouldUseDebugLabel = true;

    UPROPERTY(BlueprintAssignable, Category = "Match|Events")
    FOnMatchAssignmentChanged OnAssignmentChanged;

    UPROPERTY(BlueprintAssignable, Category = "Match|Events")
    FOnMatchedWith OnMatchedWith;

    UPROPERTY(BlueprintAssignable, Category = "Match|Events")
    FOnMismatchWith OnMismatchWith;

    UFUNCTION(BlueprintCallable, Category = "Match")
    void Assign(EMatchColor NewColor, bool bIsLookingForMatch);

    UFUNCTION(BlueprintCallable, Category = "Match")
    void ClearAssignment();

    UFUNCTION(BlueprintCallable, Category = "Match")
    bool IsLooking() const { return State == EMatchState::LookingForMatch; }

    void HandleMatched(AActor* OtherActor);
    void HandleMismatch(AActor* OtherActor);

protected:
    virtual void BeginPlay() override;

private:
    void RefreshDebugLabel();
    void TryAutoRegisterWithManager();
    AMatchGameManager* FindGameManager() const;

    UPROPERTY()
    UTextRenderComponent* DebugTextComponent = nullptr;
};