// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseNPCController.generated.h"

/**
 *  THIS CLASS WILL NOT BE IN USE YET - LMK IF IT BUGS OUT ANYONES PROGRAM
 */
UCLASS()
class MYPROJECT3_API ABaseNPCController : public AAIController
{
	GENERATED_BODY()

public:
	ABaseNPCController();
protected:
	virtual void BeginPlay() override;
	
};
