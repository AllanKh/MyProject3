
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCDataGatherer.generated.h"

USTRUCT(BlueprintType)
struct FNPCDATASTRUCT
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	int32 DelegateCount = 0;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	int32 DelegateMemoryUsage = 0;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
	float TimeActive = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
    int32 TasksSucceeded = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
    int32 TasksFailed = 0;

};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT3_API UNPCDataGatherer : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDataGatherer();

    // Struct holding telemetry data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Telemetry")
    FNPCDATASTRUCT NPCData;

    // Blueprint callable update functions
    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void IncreaseDelegateCount();

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void DecreaseDelegateCount();

    /*UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void UpdateMemoryUsage(int32 NewMemoryUsage);*/

    void AddTimeActive(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void IncreaseTasksSuccessful();

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void IncreaseTasksFailed();

    UFUNCTION(BlueprintCallable, Category = "Telemetry")
    void TaskClear();

    UFUNCTION(Blueprintcallable, Category = "Telemetry")
    void EndOfDay();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};