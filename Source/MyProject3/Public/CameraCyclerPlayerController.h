#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CameraCyclerPlayerController.generated.h"

class ACameraActor;
class UInputMappingContext;
class UInputAction;
class UUserWidget;

UCLASS()
class MYPROJECT3_API ACameraCyclerPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ACameraCyclerPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

protected:
    // Cameras found in the level with tag "GameCamera"
    UPROPERTY(VisibleInstanceOnly)
    TArray<ACameraActor*> GameCameras;

    UPROPERTY(VisibleInstanceOnly)
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

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
    TSubclassOf<UUserWidget> TutorialWidgetClass;

    UPROPERTY()
    UUserWidget* TutorialWidgetInstance = nullptr;

    UPROPERTY(VisibleInstanceOnly, Category = "Tutorial")
    bool bIsTutorialActive = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Tutorial")
    int32 TutorialInputCooldownFrames = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Input")
    UInputAction* ToggleTutorialAction = nullptr;

    // Camera cycling
    void NextCamera();
    void PreviousCamera();
    void BuildCameraList();
    void SwitchToCamera(int32 NewIndex, float CustomBlendTime = -1.f);

    // Tutorial
    void ToggleTutorial();

    // Sync current view target -> index (prevents weird jumping)
    UPROPERTY()
    bool bHasSyncedToViewTarget = false;

    int32 FindIndexOfCurrentViewTarget() const;
    void SyncIndexToCurrentCamera();
};
