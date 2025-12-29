// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NPCDataGatherer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "DataManager.generated.h"

UNPCDataGatherer;
UCLASS()
class MYPROJECT3_API UDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TArray<FNPCDATASTRUCT> NPCDataStructCollection;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void AddToNPCStruct(const FNPCDATASTRUCT &data);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	FString SerializeNPCArrayToString() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void PrintToCSV() const;

	
};
