// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TOOL/NPCTelemetry.h"
#include "UTelemetryManager.generated.h"


UCLASS()
class MYPROJECT3_API UUTelemetryManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

    public:

    UPROPERTY() TArray<FNPCTelemetryStruct> NPCsTelemetry;
    UPROPERTY() TArray<FMinigameTelemetryStruct> MinigamesTelemetry;

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void LogNPCEvent(const FString& NPCName, const FString& EventName, const FString& Details, float Timestamp);

    UFUNCTION(BlueprintCallable, Category="Telemetry")
    void UpdateDelegateAndMemoryCount(const FString& NPCName, int32 Count, int32 MemoryUsage);

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void UpdateCameraActiveTime(const FString& MinigameName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void IncrementClickCount(const FString& MinigameName);

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void ProcessEndOfDayData();

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void ExportData();

    UPROPERTY()
    TArray<FNPCTelemetryStruct> NPCTelemetryArray;

    UPROPERTY()
    UUTelemetryManager* TelemetryManager;

    UFUNCTION(BlueprintCallable)
    UUTelemetryManager* GetTelemetryManager() const { return TelemetryManager; }

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void Deinitialize() override;


    private:
        UPROPERTY()
        TMap<FString,int32> DelegateCounts;

        UPROPERTY()
        TMap<FString, int32> MemoryUsages;
};
