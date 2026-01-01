// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/DataManager.h"
#include "Kismet/GameplayStatics.h"

void UDataManager::AddToNPCStruct(const FNPCDATASTRUCT& data)
{
	NPCDataStructCollection.Add(data);
}

void UDataManager::AddToCameraStruct(const FCAMERADATASTRUCT& data)
{
	CameraDataStructCollection.Add(data);
}

FString UDataManager::SerializeNPCArrayToString() const
{
	FString CSV = TEXT("NPC Name, Delegate Count, Time Active, Tasks Succeeded, Tasks Failed");
	for (const FNPCDATASTRUCT& Entry : NPCDataStructCollection)
	{
		CSV += FString::Printf(TEXT("%s, %d, %f, %g, %h\n"), *Entry.Name, Entry.DelegateCount, Entry.TimeActive, Entry.TasksSucceeded, Entry.TasksFailed);
	}
	return CSV;
}

FString UDataManager::SerializeCameraArrayToString() const
{
	FString CSV = TEXT("Camera Name, Minigame Name, Time Active, Times Clicked\n");
	for (const FCAMERADATASTRUCT& Entry : CameraDataStructCollection)
	{
		CSV += FString::Printf(TEXT("%s, %.2f, %d\n"), *Entry.MinigameName, Entry.TimeActive, Entry.TimesClicked);
	}
	return CSV;
}

void UDataManager::GatherTelemetryData()
{
	NPCDataStructCollection.Empty(); // Clear old data

	UClass* NPCBlueprintClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/Game/PathTo/BP_NPC_NEW.BP_NPC_NEW_C"));

	// Get all NPC actors
	TArray<AActor*> NPCActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), NPCBlueprintClass, NPCActors);


	for (AActor* Actor : NPCActors)
	{
		if (UNPCDataGatherer* Gatherer = Actor->FindComponentByClass<UNPCDataGatherer>())
		{
			Gatherer->EndOfDay();
		}
	}

	// Get all Camera Components
}

void UDataManager::PrintToCSV()
{
	GatherTelemetryData();

	FString NPCCSV = SerializeNPCArrayToString();
	FString CameraCSV = SerializeCameraArrayToString();

	FString CombinedCSV = NPCCSV + TEXT("\n\n") + CameraCSV;

	FString FilePath = FPaths::ProjectSavedDir() / TEXT("TelemetryExport.csv");
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
