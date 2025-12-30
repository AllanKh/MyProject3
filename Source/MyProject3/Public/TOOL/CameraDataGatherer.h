#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraDataGatherer.generated.h"


class UDatamanager;
USTRUCT(BlueprintType)
struct FCAMERADATASTRUCT
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	FString MinigameName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float TimeActive = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	int32 TimesClicked = 0.0f;


};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT3_API UCameraDataGatherer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCameraDataGatherer();

	FCAMERADATASTRUCT CameraData;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RegisterClick();

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void EndOfDay();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FString GetActiveCameraName() const;

	bool DoesCameraMatchMinigame(const FString& ActiveCameraName) const {
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

		
};
