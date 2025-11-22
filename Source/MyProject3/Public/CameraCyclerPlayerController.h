#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CameraCyclerPlayerController.generated.h"

class ACameraActor;
class UInputMappingContext;
class UInputAction;

UCLASS()
class MYPROJECT3_API ACameraCyclerPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ACameraCyclerPlayerController();
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

protected:
    UPROPERTY()
    TArray<ACameraActor*> GameCameras;

    int32 CurrentCameraIndex = INDEX_NONE;

    UPROPERTY(EditAnywhere, Category = "Camera Cycling")
    float BlendTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Camera Cycling")
    TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Cubic;

    UPROPERTY(EditAnywhere, Category = "Camera Cycling")
    float BlendExp = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera Cycling|Input")
    UInputMappingContext* CameraMappingContext = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Camera Cycling|Input")
    UInputAction* NextCameraAction = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Camera Cycling|Input")
    UInputAction* PrevCameraAction = nullptr;

    void NextCamera();
    void PreviousCamera();
    void BuildCameraList();
    void SwitchToCamera(int32 NewIndex, float CustomBlendTime = -1.f);
};
