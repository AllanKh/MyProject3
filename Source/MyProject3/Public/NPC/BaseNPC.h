// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "BaseNPC.generated.h"

/*
THIS CLASS IS THE BASE FOR ALL OF THE NPCS IN THIS GAME, AT THE MOMENT OF WRITING THIS SECION, THIS CLASS IS NOT A COMPLETE BASE CLASS.
WILL BE UPDATED LATER ON TO SUB CLASSES ONCE BASE STRUCTURE IS COMPLETE.
*/

UCLASS()
class MYPROJECT3_API ABaseNPC : public APawn
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//

public:	

	ABaseNPC();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION(BlueprintCallable, Category="Movement")
	//virtual void MoveTo(const FVector& Target, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category="Behavior")
	virtual void EnterRagdoll();

	UFUNCTION(BlueprintCallable, Category="Behavior")
	virtual void ExitRagdoll();

	UFUNCTION(BlueprintCallable, Category="Behavior")
	bool GetRagdollState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float Speed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool ragdollState = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;     

	UPROPERTY(VisibleAnywhere)
	UFloatingPawnMovement* MovementComponent;

};
