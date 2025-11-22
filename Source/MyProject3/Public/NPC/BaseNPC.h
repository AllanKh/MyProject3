#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "MyProject3/GrabbableComponent.h"
#include "BaseNPC.generated.h"

/*
THIS CLASS IS DEPRICATED FORN OW DO NOT USE IT !!! ! ! ! ! ! ! ! !!!! ! ! ! ! ! 
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnterRagdoll);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitRagdoll);

UCLASS()
class MYPROJECT3_API ABaseNPC : public APawn
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	

	ABaseNPC();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintAssignable, Category="Ragdoll")
	FOnEnterRagdoll OnEnterRagdoll;

	UPROPERTY(BlueprintAssignable, Category = "Ragdoll")
	FOnExitRagdoll OnExitRagdoll;

	UFUNCTION(BlueprintCallable, Category="Ragdoll")
	virtual void EnterRagdoll();

	UFUNCTION(BlueprintCallable, Category="Ragdoll")
	virtual void ExitRagdoll();

	UFUNCTION(BlueprintCallable, Category="Ragdoll")
	bool GetRagdollState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float Speed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool ragdollState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	bool hasLanded = false;

	FTimerHandle RagdollRecoveryTimer;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;     

	UPROPERTY(VisibleAnywhere)
	UFloatingPawnMovement* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UGrabbableComponent* GrabbableComponent;

};
