// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/CameraDataGatherer.h"
#include "TOOL/DataManager.h"

// Sets default values for this component's properties
UCameraDataGatherer::UCameraDataGatherer()
{

	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UCameraDataGatherer::BeginPlay()
{
    Super::BeginPlay();

	CameraData.MinigameName = LinkedCameraName;
}


// Called every frame
void UCameraDataGatherer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsCameraMatched)
	{

		CameraData.TimeActive += DeltaTime;
	}
}


void UCameraDataGatherer::RegisterClick()
{
    if (IsCameraMatched)
    {
        CameraData.TimesClicked++;
        UE_LOG(LogTemp, Warning, TEXT("Click registered; TimesClicked is now %d."), CameraData.TimesClicked);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Click registered; TimesClicked is now %d."), CameraData.TimesClicked);
    }
}

void UCameraDataGatherer::VerifyActiveCamera(FString ActiveCameraName)
{
    if (ActiveCameraName != CameraData.MinigameName)
    {
        IsCameraMatched = false;
        UE_LOG(LogTemp, Warning, TEXT("VerifyActiveCamera: ActiveCameraActor nullptr. MinigameName: [%s]"), *CameraData.MinigameName);
        return;
    }

    IsCameraMatched = true;

}

void UCameraDataGatherer::EndOfDay() 
{
    if (UDataManager* DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UDataManager>())
    {
        DataManager->AddToCameraStruct(CameraData);
        UE_LOG(LogTemp, Log, TEXT("Camera telemetry submitted to DataManager."));
    }
}


