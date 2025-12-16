// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/UTelemetryManager.h"
#include "Engine/Engine.h"
#include "Logging/LogMacros.h"

void UUTelemetryManager::LogNPCEvent(const FString& NPCName, const FString& EventName, const FString& Details, float Timestamp)
{


}


void UUTelemetryManager::UpdateDelegateAndMemoryCount(const FString& NPCName, int32 Count, int32 MemoryUsage)
{

	DelegateCounts.FindOrAdd(NPCName) = Count;
    MemoryUsages.FindOrAdd(NPCName) = MemoryUsage;

#if !UE_BUILD_SHIPPING 
	UE_LOG(LogTemp, Log, TEXT("Telemetry: Updated DelegateCount for NPC '%s': Count=%d, MemoryUsage=%d"), *NPCName, Count, MemoryUsage);
#endif

}

void UUTelemetryManager::UpdateCameraActiveTime(const FString& MinigameName, float DeltaTime)
{


}

void UUTelemetryManager::IncrementClickCount(const FString& MinigameName)
{


}

void UUTelemetryManager::ProcessEndOfDayData()
{
    ExportData();

    for (FNPCTelemetryStruct& Data : NPCTelemetryArray)
    {
        UE_LOG(LogTemp, Log, TEXT("NPC '%s' - DelegateCount: %d, MemoryUsage: %d bytes, Events: %d"), *Data.Name, Data.DelegateCount, Data.DelegateMemoryUsage, Data.EventLog.Num());
        Data.DelegateCount = 0;
        Data.DelegateMemoryUsage = 0;
        Data.EventLog.Empty();
    }
}

void UUTelemetryManager::ExportData()
{
    TArray<TSharedPtr<FJsonValue>> TelemetryJsonArray;

    for (const FNPCTelemetryStruct& Data : NPCTelemetryArray)
    {
        TSharedPtr<FJsonObject> NPCJson = MakeShared<FJsonObject>();

        NPCJson->SetStringField(TEXT("Name"), Data.Name);
        NPCJson->SetStringField(TEXT("Status"), Data.Status);
        NPCJson->SetNumberField(TEXT("DelegateCount"), Data.DelegateCount);
        NPCJson->SetNumberField(TEXT("DelegateMemoryUsage"), Data.DelegateMemoryUsage);

        // Serialize EventLog (array of FNPCTelemetryEvent)
        TArray<TSharedPtr<FJsonValue>> EventLogJsonArray;
        for (const FNPCTelemetryEvent& Event : Data.EventLog)
        {
            TSharedPtr<FJsonObject> EventJson = MakeShared<FJsonObject>();
            EventJson->SetStringField(TEXT("EventName"), Event.EventName);
            EventJson->SetNumberField(TEXT("Timestamp"), Event.Timestamp);
            EventJson->SetStringField(TEXT("Details"), Event.Details);

            EventLogJsonArray.Add(MakeShared<FJsonValueObject>(EventJson));
        }

        NPCJson->SetArrayField(TEXT("EventLog"), EventLogJsonArray);

        TelemetryJsonArray.Add(MakeShared<FJsonValueObject>(NPCJson));
    }

    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetArrayField(TEXT("Telemetry"), TelemetryJsonArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
    {
        FString FilePath = FPaths::ProjectSavedDir() / TEXT("TelemetryData.json");
        if (FFileHelper::SaveStringToFile(OutputString, *FilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Telemetry exported successfully to %s"), *FilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save telemetry to %s"), *FilePath);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to serialize telemetry data to JSON"));
    }
}

void UUTelemetryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // Example: initialize telemetry storage
    NPCTelemetryMap.Empty();

    // Bind any needed delegates or events here

    UE_LOG(LogTemp, Log, TEXT("TelemetryManagerSubsystem Initialized"));
}




