// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TOOL/NPCTelemetry.h"
#include "UTelemetryManager.generated.h"


UCLASS()
class MYPROJECT3_API UUTelemetryManager : public UObject
{
	GENERATED_BODY()

    public:

    UPROPERTY() TArray<FNPCTelemetryStruct> NPCsTelemetry;
    UPROPERTY() TArray<FMinigameTelemetryStruct> MinigamesTelemetry;

    void LogNPCEvent(const FString& NPCName, const FString& EventName, const FString& Details, float Timestamp);
    void UpdateDelegateCount(const FString& NPCName, int32 Count, int32 MemoryUsage);
    void UpdateCameraActiveTime(const FString& MinigameName, float DeltaTime);
    void IncrementClickCount(const FString& MinigameName);
    void ProcessEndOfDayData();
};
