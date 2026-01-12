// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NPCDataGatherer.h"
#include "CameraDataGatherer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "DataManager.generated.h"

UNPCDataGatherer;
UCameraDataGatherer;
UCLASS()
class MYPROJECT3_API UDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TArray<FNPCDATASTRUCT> NPCDataStructCollection;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TArray<FCAMERADATASTRUCT> CameraDataStructCollection;


	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void AddToNPCStruct(const FNPCDATASTRUCT &data);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void AddToCameraStruct(const FCAMERADATASTRUCT& data);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	FString SerializeNPCArrayToString() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	FString SerializeCameraArrayToString() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void GatherTelemetryData();

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void PrintToCSV();



	
};
