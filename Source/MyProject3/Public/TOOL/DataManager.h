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

USTRUCT(BlueprintType)
struct FEventRecord {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event")
	FString ObjectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventMessage;
};


UCLASS()
class MYPROJECT3_API UDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TArray<FNPCDATASTRUCT> NPCDataStructCollection;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TArray<FCAMERADATASTRUCT> CameraDataStructCollection;

	TMap<FString, FEventRecord> EventRecords;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void LogEvent(const FString& ObjectName, const FString& EventMessage);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ExportEventLog();


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
