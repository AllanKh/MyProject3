// Fill out your copyright notice in the Description page of Project Settings.


#include "TOOL/CameraDataGatherer.h"
#include "TOOL/DataManager.h"

// Sets default values for this component's properties
UCameraDataGatherer::UCameraDataGatherer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCameraDataGatherer::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        ULevel* OwnerLevel = Owner->GetLevel();
        UWorld* World = GetWorld();

        if (OwnerLevel && World)
        {
            if (OwnerLevel == World->PersistentLevel)
            {
                CameraData.MinigameName = TEXT("Trading");
            }
            else
            {
                if (OwnerLevel->GetOuter())
                {
                    if (ULevelStreaming* LevelStreaming = Cast<ULevelStreaming>(OwnerLevel->GetOuter()))
                    {
                        FString LevelPackageName = LevelStreaming->GetWorldAssetPackageName();
                        CameraData.MinigameName = FPackageName::GetShortName(LevelPackageName);
                    }
                    else
                    {
						CameraData.MinigameName = OwnerLevel->GetName();
                    }
                }
            }
        }
    }
}


// Called every frame
void UCameraDataGatherer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FString ActiveCameraName = GetActiveCameraName();

	if (DoesCameraMatchMinigame(ActiveCameraName))
	{
		CameraData.TimeActive += DeltaTime;
	}
}

FString UCameraDataGatherer::GetActiveCameraName() const
{
	if (GetWorld())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->PlayerCameraManager)
		{
			AActor* ViewTarget = PC->PlayerCameraManager->GetViewTarget();
			if (ViewTarget)
			{
				return ViewTarget->GetName();
			}
		}
	}
	return FString();
}

bool UCameraDataGatherer::DoesCameraMatchMinigame(const FString& ActiveCameraName) const
{
		if (CameraData.MinigameName == TEXT("Trading"))
		{
			return ActiveCameraName == TEXT("TradeCamera");
		}
		else if (CameraData.MinigameName == TEXT("DungeonMinigame"))
		{
			return ActiveCameraName == TEXT("DungeonCamera");
		}
		else if (CameraData.MinigameName == TEXT("ResourceMinigame"))
		{
			return ActiveCameraName == TEXT("ResourceCamera");
		}
		else if (CameraData.MinigameName == TEXT("LootPackerMinigame"))
		{
			return ActiveCameraName == TEXT("LootCamera");
		}
		return false;
}

void UCameraDataGatherer::RegisterClick() {
	FString ActiveCameraName = GetActiveCameraName();

	if (DoesCameraMatchMinigame(ActiveCameraName))
	{
		CameraData.TimesClicked++;
	}
}

void UCameraDataGatherer::EndOfDay() 
{
    if (UDataManager* DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UDataManager>())
    {
        DataManager->AddToCameraStruct(CameraData);
        UE_LOG(LogTemp, Log, TEXT("Camera telemetry submitted to DataManager."));
    }
}


