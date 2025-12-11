#pragma once

#include "CoreMinimal.h"
#include "NPCTelemetry.generated.h"

USTRUCT(BlueprintType)
struct FNPCTelemetryEvent
{
    GENERATED_BODY()
    UPROPERTY() FString EventName;
    UPROPERTY() float Timestamp;
    UPROPERTY() FString Details;
};

USTRUCT(BlueprintType)
struct FNPCTelemetryStruct
{
    GENERATED_BODY()
    UPROPERTY() FString Name;
    UPROPERTY() FString Status;
    UPROPERTY() int32 DelegateCount;
    UPROPERTY() int32 DelegateMemoryUsage;
    UPROPERTY() TArray<FNPCTelemetryEvent> EventLog;
};

USTRUCT(BlueprintType)
struct FMinigameTelemetryStruct
{
    GENERATED_BODY()
    UPROPERTY() FString MinigameName;
    UPROPERTY() float CameraActiveTime;
    UPROPERTY() int32 ClickCount;
};