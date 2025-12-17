// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/NPCDataGatherer.h"

// Sets default values for this component's properties
UNPCDataGatherer::UNPCDataGatherer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UNPCDataGatherer::IncreaseDelegateCount()
{
	NPCData.DelegateCount++;
}

void UNPCDataGatherer::DecreaseDelegateCount()
{
	NPCData.DelegateCount--;
}

//void UNPCDataGatherer::UpdateMemoryUsage(int32 NewMemoryUsage)
//{
//
//}

void UNPCDataGatherer::AddTimeActive(float DeltaTime)
{
	NPCData.TimeActive += DeltaTime;

}

void UNPCDataGatherer::IncreaseTasksSuccessful()
{
	NPCData.TasksSucceeded++;
}


void UNPCDataGatherer::IncreaseTasksFailed()
{
	NPCData.TasksFailed++;
}

void UNPCDataGatherer::TaskClear()
{
	NPCData.TasksSucceeded = 0;
	NPCData.TasksFailed = 0;
}

void UNPCDataGatherer::EndOfDay()
{

}



// Called when the game starts
void UNPCDataGatherer::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		NPCData.Name = Owner->GetName();
	}
	
}


// Called every frame
void UNPCDataGatherer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	AddTimeActive(DeltaTime);

	
}

