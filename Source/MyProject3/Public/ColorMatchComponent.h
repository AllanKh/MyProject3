#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EMatchColor.h"
#include "ColorMatchComponent.generated.h"

class AMatchGameManager;
class UStaticMeshComponent;
class UStaticMesh;

// tracks what state the npc is in for matching
UENUM(BlueprintType)
enum class EMatchState : uint8
{
    Idle,
    LookingForMatch,
    Matched,
    Dead
};

// fired when npc gets assigned a color or starts looking
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchAssignmentChanged, EMatchColor, NewColor, bool, bIsLookingForMatch);
// fired when npc successfully matches with another
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchedWith, AActor*, OtherActor);
// fired when npc collides but colors dont match
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMismatchWith, AActor*, OtherActor);

// component that handles color matching behavior for npcs
UCLASS(ClassGroup = (MiniGame), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UColorMatchComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UColorMatchComponent();

    // whether this component is currently active (match logic & events enabled)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Debug")
    bool bIsComponentEnabled = true;

    // what color this npc is currently assigned
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
    EMatchColor CurrentColor = EMatchColor::None;

    // current state of the npc
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
    EMatchState State = EMatchState::Idle;

    // whether to show icon mesh above npc
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Debug")
    bool bShouldUseDebugLabel = true;

    // weapon meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Visual")
    UStaticMesh* Mesh_RPGHero_Sword01 = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Visual")
    UStaticMesh* Mesh_AnimalHero_Shield01 = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Visual")
    UStaticMesh* Mesh_TinyHero_Sword02 = nullptr;

    // Offset of the icon above the NPC’s root
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Visual")
    FVector IconOffset = FVector(0.f, 0.f, 120.f);

    // Scale of the icon mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match|Visual")
    FVector IconScale = FVector(0.6f, 0.6f, 0.6f);

    // events that fire during matching
    UPROPERTY(BlueprintAssignable, Category = "Match|Events")
    FOnMatchAssignmentChanged OnAssignmentChanged;

    UPROPERTY(BlueprintAssignable, Category = "Match|Events")
    FOnMatchedWith OnMatchedWith;

    UPROPERTY(BlueprintAssignable, Category = "Match|Events")
    FOnMismatchWith OnMismatchWith;

    // gives this npc a color and makes it look for matches
    UFUNCTION(BlueprintCallable, Category = "Match")
    void Assign(EMatchColor NewColor, bool bIsLookingForMatch);

    // removes color assignment and returns to idle
    UFUNCTION(BlueprintCallable, Category = "Match")
    void ClearAssignment();

    UFUNCTION(BlueprintCallable, Category = "Match") // added by liz
    void Reset();

    // checks if this npc is currently looking for a match
    UFUNCTION(BlueprintCallable, Category = "Match")
    bool IsLooking() const { return State == EMatchState::LookingForMatch; }

    // called by manager when match succeeds
    void HandleMatched(AActor* OtherActor);
    // called by manager when match fails
    void HandleMismatch(AActor* OtherActor);

protected:
    virtual void BeginPlay() override;

private:
    // updates the floating icon above npc
    void RefreshDebugLabel();

    // registers this npc with game manager on start
    void TryAutoRegisterWithManager();

    // finds the game manager in the world
    AMatchGameManager* FindGameManager() const;

    // mesh component that shows icon above npc
    UPROPERTY()
    UStaticMeshComponent* DebugIconComponent = nullptr;

    // mesh currently chosen for this npc
    UPROPERTY()
    UStaticMesh* CurrentIconMesh = nullptr;

    // picks a random mesh from the three configured options
    UStaticMesh* GetRandomIconMesh() const;
};
