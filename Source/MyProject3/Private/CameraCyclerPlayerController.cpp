#include "CameraCyclerPlayerController.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"

ACameraCyclerPlayerController::ACameraCyclerPlayerController()
{
    bAutoManageActiveCameraTarget = false;

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;

    CurrentCameraIndex = 0;
    bIsTutorialActive = false;
    bHasSyncedToViewTarget = false;

    TutorialWidgetInstance = nullptr;
}

void ACameraCyclerPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("CameraCyclerPlayerController::BeginPlay"));

    // Enhanced Input mapping context
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsys =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (CameraMappingContext)
            {
                Subsys->AddMappingContext(CameraMappingContext, 1);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("CameraMappingContext is NOT set!"));
            }
        }
    }

    // Build & force a deterministic start camera
    BuildCameraList();

    if (GameCameras.Num() > 0)
    {
        CurrentCameraIndex = 0;
        // Hard set to camera 0 so we never "random start"
        SetViewTarget(GameCameras[0]);
        UE_LOG(LogTemp, Warning, TEXT("BeginPlay: Forced start camera to %s"), *GameCameras[0]->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BeginPlay: No GameCameras found"));
    }

    // Tutorial widget class auto-load (your existing logic)
    if (!TutorialWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("TutorialWidgetClass not set, attempting to load..."));

        TutorialWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Tutorial/WBP_TutorialScreen.WBP_TutorialScreen_C"));

        if (TutorialWidgetClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("TutorialWidgetClass loaded successfully!"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load TutorialWidgetClass!"));
        }
    }
}

void ACameraCyclerPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EIC)
    {
        UE_LOG(LogTemp, Error, TEXT("InputComponent is not enhanced input"));
        return;
    }

    if (NextCameraAction)
    {
        EIC->BindAction(NextCameraAction, ETriggerEvent::Started,
            this, &ACameraCyclerPlayerController::NextCamera);
    }

    if (PrevCameraAction)
    {
        EIC->BindAction(PrevCameraAction, ETriggerEvent::Started,
            this, &ACameraCyclerPlayerController::PreviousCamera);
    }

    // Legacy input bind for tutorial toggle (as you had)
    InputComponent->BindAction("ToggleTutorial", IE_Pressed, this, &ACameraCyclerPlayerController::ToggleTutorial);
    UE_LOG(LogTemp, Warning, TEXT("Tutorial input bound using legacy input"));
}

void ACameraCyclerPlayerController::BuildCameraList()
{
    GameCameras.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundActors);

    UE_LOG(LogTemp, Warning, TEXT("BuildCameraList: Found %d camera in world"), FoundActors.Num());

    const FName TagName(TEXT("GameCamera"));

    for (AActor* Actor : FoundActors)
    {
        if (ACameraActor* Cam = Cast<ACameraActor>(Actor))
        {
            if (Cam->ActorHasTag(TagName))
            {
                UE_LOG(LogTemp, Warning, TEXT("Added %s as GameCamera"), *Cam->GetName());
                GameCameras.Add(Cam);
            }
        }
    }

    // Stable sort (still depends on camera names being stable)
    GameCameras.Sort([](const ACameraActor& A, const ACameraActor& B)
        {
            return A.GetName() < B.GetName();
        });

    UE_LOG(LogTemp, Warning, TEXT("BuildCameraList: Total GameCameras: %d"), GameCameras.Num());
}

void ACameraCyclerPlayerController::SwitchToCamera(int32 NewIndex, float CustomBlendTime)
{
    if (!GameCameras.IsValidIndex(NewIndex))
        return;

    if (ACameraActor* TargetCam = GameCameras[NewIndex])
    {
        const float UseBlendTime = (CustomBlendTime >= 0.f) ? CustomBlendTime : BlendTime;

        FViewTargetTransitionParams Params;
        Params.BlendTime = UseBlendTime;
        Params.BlendFunction = BlendFunction;
        Params.BlendExp = BlendExp;

        UE_LOG(LogTemp, Warning, TEXT("SwitchToCamera: %d -> %s"), NewIndex, *TargetCam->GetName());

        SetViewTarget(TargetCam, Params);
    }
}

int32 ACameraCyclerPlayerController::FindIndexOfCurrentViewTarget() const
{
    AActor* VT = GetViewTarget();
    if (!VT)
        return INDEX_NONE;

    if (ACameraActor* AsCam = Cast<ACameraActor>(VT))
    {
        return GameCameras.IndexOfByKey(AsCam);
    }

    return INDEX_NONE;
}

void ACameraCyclerPlayerController::SyncIndexToCurrentCamera()
{
    if (GameCameras.Num() == 0)
        BuildCameraList();

    const int32 Found = FindIndexOfCurrentViewTarget();
    if (Found != INDEX_NONE)
    {
        CurrentCameraIndex = Found;
        UE_LOG(LogTemp, Warning, TEXT("Synced CurrentCameraIndex to %d (%s)"),
            CurrentCameraIndex, *GameCameras[CurrentCameraIndex]->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SyncIndexToCurrentCamera: ViewTarget is not a CameraActor in GameCameras"));
    }
}

void ACameraCyclerPlayerController::NextCamera()
{
    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NextCamera: camera list empty, rebuilding"));
        BuildCameraList();
    }

    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NextCamera: still no cameras after rebuild???"));
        return;
    }

    // Sync only once so we don't fight blends / other systems
    if (!bHasSyncedToViewTarget)
    {
        SyncIndexToCurrentCamera();
        bHasSyncedToViewTarget = true;
    }

    CurrentCameraIndex = (CurrentCameraIndex + 1) % GameCameras.Num();
    SwitchToCamera(CurrentCameraIndex, -1.f);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("NextCamera key pressed"));
    }
}

void ACameraCyclerPlayerController::PreviousCamera()
{
    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PreviousCamera: camera list empty, rebuilding"));
        BuildCameraList();
    }

    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PreviousCamera: still no cameras after rebuild???"));
        return;
    }

    // Sync only once so we don't fight blends / other systems
    if (!bHasSyncedToViewTarget)
    {
        SyncIndexToCurrentCamera();
        bHasSyncedToViewTarget = true;
    }

    CurrentCameraIndex = (CurrentCameraIndex - 1 + GameCameras.Num()) % GameCameras.Num();
    SwitchToCamera(CurrentCameraIndex, -1.f);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("PrevCamera key pressed"));
    }
}

void ACameraCyclerPlayerController::ToggleTutorial()
{
    if (!bIsTutorialActive)
    {
        // Open tutorial
        if (TutorialWidgetClass)
        {
            TutorialWidgetInstance = CreateWidget<UUserWidget>(this, TutorialWidgetClass);

            if (TutorialWidgetInstance)
            {
                TutorialWidgetInstance->AddToViewport(100);

                SetIgnoreMoveInput(true);
                SetIgnoreLookInput(true);
                bShowMouseCursor = true;

                bIsTutorialActive = true;

                UE_LOG(LogTemp, Warning, TEXT("Tutorial opened"));
            }
        }
    }
    else
    {
        // Close tutorial
        if (TutorialWidgetInstance)
        {
            TutorialWidgetInstance->RemoveFromParent();
            TutorialWidgetInstance = nullptr;

            SetIgnoreMoveInput(false);
            SetIgnoreLookInput(false);
            bShowMouseCursor = false;

            bIsTutorialActive = false;

            UE_LOG(LogTemp, Warning, TEXT("Tutorial closed"));
        }
    }
}
