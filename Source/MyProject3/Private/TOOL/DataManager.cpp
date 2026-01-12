// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/DataManager.h"
#include "Kismet/GameplayStatics.h"

void UDataManager::AddToNPCStruct(const FNPCDATASTRUCT& data)
{
    UE_LOG(LogTemp, Log, TEXT("Adding NPC telemetry from: %s"), *data.Name);
    NPCDataStructCollection.Add(data);
    UE_LOG(LogTemp, Log, TEXT("NPC data array size is now: %d"), NPCDataStructCollection.Num());
}

void UDataManager::AddToCameraStruct(const FCAMERADATASTRUCT& data)
{
    CameraDataStructCollection.Add(data);
}

FString UDataManager::SerializeNPCArrayToString() const
{
    FString CSV = TEXT("NPC Name, Delegate Count, Time Active, Tasks Succeeded, Tasks Failed\n");
    for (const FNPCDATASTRUCT& Entry : NPCDataStructCollection)
    {
        CSV += FString::Printf(TEXT("%s, %d, %f, %d, %d\n"), *Entry.Name, Entry.DelegateCount, Entry.TimeActive, Entry.TasksSucceeded, Entry.TasksFailed);
    }
    return CSV;
}

FString UDataManager::SerializeCameraArrayToString() const
{
    FString CSV = TEXT("Camera Name, Time Active, Times Clicked\n");
    for (const FCAMERADATASTRUCT& Entry : CameraDataStructCollection)
    {
        CSV += FString::Printf(TEXT("%s, %.2f, %d\n"),
            *Entry.MinigameName, 
            Entry.TimeActive,
            Entry.TimesClicked);
    }
    return CSV;
}

void UDataManager::GatherTelemetryData()
{

    NPCDataStructCollection.Empty();

    UClass* NPCBlueprintClass = LoadClass<AActor>(
        nullptr,
        TEXT("/Game/AI/Blueprints/NPC/BP_NPC_NEW.BP_NPC_NEW_C")
    );

    if (!NPCBlueprintClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load NPC Blueprint class."));
        return;
    }

    TArray<AActor*> NPCActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), NPCBlueprintClass, NPCActors);
    UE_LOG(LogTemp, Log, TEXT("GatherTelemetryData: Found %d NPC actors."), NPCActors.Num());

    for (AActor* Actor : NPCActors)
    {
        if (!Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("GatherTelemetryData: Found nullptr Actor in NPCActors array."));
            continue;
        }

        UE_LOG(LogTemp, Log, TEXT("GatherTelemetryData: Processing Actor: %s"), *Actor->GetName());

        if (UNPCDataGatherer* Gatherer = Actor->FindComponentByClass<UNPCDataGatherer>())
        {
            UE_LOG(LogTemp, Log, TEXT("GatherTelemetryData: Calling EndOfDay() on %s"), *Actor->GetName());
            Gatherer->EndOfDay();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("GatherTelemetryData: No UNPCDataGatherer component found on %s"), *Actor->GetName());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("GatherTelemetryData: Telemetry collection complete."));

    CameraDataStructCollection.Empty();
    UE_LOG(LogTemp, Log, TEXT("GatherTelemetryData: Cleared CameraDataStructCollection."));

    // Gather Camera telemetry
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;

        if (UCameraDataGatherer* CameraGatherer = Actor->FindComponentByClass<UCameraDataGatherer>())
        {
            UE_LOG(LogTemp, Log, TEXT("GatherTelemetryData: Calling EndOfDay() on Camera Actor: %s"), *Actor->GetName());
            CameraGatherer->EndOfDay();
        }
    }
}

void UDataManager::PrintToCSV()
{
    FString NPCCSV = SerializeNPCArrayToString();
    FString CameraCSV = SerializeCameraArrayToString();

    FString CombinedCSV = NPCCSV + TEXT("\n\n") + CameraCSV;

    FDateTime now = FDateTime::UtcNow();
    FString Timestamp = now.ToString(TEXT("%Y%m%d_%H%M%S"));

    FString FileName = FString::Printf(TEXT("TelemetryExport_%s.csv"), *Timestamp);

    FString FilePath = FPaths::ProjectSavedDir() / FileName;
    bool bSuccess = FFileHelper::SaveStringToFile(CombinedCSV, *FilePath);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("CSV exported successfully to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to export CSV to: %s"), *FilePath);
    }
}