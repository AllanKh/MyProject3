// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/DataManager.h"

void UDataManager::AddToNPCStruct(const FNPCDATASTRUCT& data)
{
	NPCDataStructCollection.Add(data);
}

FString UDataManager::SerializeNPCArrayToString() const
{
	FString CSV = TEXT("NPC Name, Delegate Count, Time Active, Tasks Succeeded, Tasks Failed");
	for (const FNPCDATASTRUCT& Entry : NPCDataStructCollection)
	{
		CSV += FString::Printf(TEXT("%s, %d, %f, %g, %h, %h\n"), *Entry.Name, Entry.DelegateCount, Entry.TimeActive, Entry.TasksSucceeded, Entry.TasksFailed);
	}
	return CSV;
}

void UDataManager::PrintToCSV() const
{
	FString NPCCSV = SerializeNPCArrayToString();

	FString CombinedCSV = NPCCSV;

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
