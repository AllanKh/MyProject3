// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/NavMovementComponent.h"
#include "BaseNPC.generated.h"

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

	UFUNCTION(BlueprintCallable, Category="Movement")
	virtual void MoveTo(const FVector& Target, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category="Behavior")
	virtual void EnterRagdoll();

	UFUNCTION(BlueprintCallable, Category="Behavior")
	virtual void ExitRagdoll();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float Speed = 300.f;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;     

	/*UPROPERTY(VisibleAnywhere)
	UNavMovementComponent* MovementComponent;*/

};
