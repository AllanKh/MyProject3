#include "CameraCyclerPlayerController.h"

#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"

void ACameraCyclerPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("CameraCyclerPlayerController::BeginPlay"));

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

    BuildCameraList();

    if (GameCameras.Num() > 0)
    {
        CurrentCameraIndex = 0;

        SwitchToCamera(CurrentCameraIndex, 0.0f);
    }
}

void ACameraCyclerPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UE_LOG(LogTemp, Warning, TEXT("CameraCyclerPlayerController::SetupInputComponent"));

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EIC)
    {
        UE_LOG(LogTemp, Error, TEXT("InputComponent is NOT an EnhancedInputComponent!"));
        return;
    }

    if (NextCameraAction)
    {
        EIC->BindAction(NextCameraAction, ETriggerEvent::Started,
            this, &ACameraCyclerPlayerController::NextCamera);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NextCameraAction is NOT set!"));
    }

    if (PrevCameraAction)
    {
        EIC->BindAction(PrevCameraAction, ETriggerEvent::Started,
            this, &ACameraCyclerPlayerController::PreviousCamera);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PrevCameraAction is NOT set!"));
    }
}


void ACameraCyclerPlayerController::BuildCameraList()
{
    GameCameras.Empty();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundActors);

    UE_LOG(LogTemp, Warning, TEXT("BuildCameraList: Found %d ACameraActor in world"), FoundActors.Num());

    const FName TagName(TEXT("GameCamera"));

    for (AActor* Actor : FoundActors)
    {
        if (ACameraActor* Cam = Cast<ACameraActor>(Actor))
        {
            if (Cam->ActorHasTag(TagName))
            {
                UE_LOG(LogTemp, Warning, TEXT("  -> Added %s as GameCamera"), *Cam->GetName());
                GameCameras.Add(Cam);
            }
        }
    }

    GameCameras.Sort([](const ACameraActor& A, const ACameraActor& B)
        {
            return A.GetName() < B.GetName();
        });

    UE_LOG(LogTemp, Warning, TEXT("BuildCameraList: Final GameCameras.Num = %d"), GameCameras.Num());
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

        UE_LOG(LogTemp, Warning, TEXT("SwitchToCamera: Index %d -> %s"),
            NewIndex, *TargetCam->GetName());

        SetViewTarget(TargetCam, Params);
    }
}

void ACameraCyclerPlayerController::NextCamera()
{
    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NextCamera: camera list empty, rebuilding..."));
        BuildCameraList();
    }

    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NextCamera: still no cameras after rebuild"));
        return;
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
        UE_LOG(LogTemp, Warning, TEXT("PreviousCamera: camera list empty, rebuilding..."));
        BuildCameraList();
    }

    if (GameCameras.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PreviousCamera: still no cameras after rebuild"));
        return;
    }

    CurrentCameraIndex = (CurrentCameraIndex - 1 + GameCameras.Num()) % GameCameras.Num();
    SwitchToCamera(CurrentCameraIndex, -1.f);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("PrevCamera key pressed"));
    }
}

ACameraCyclerPlayerController::ACameraCyclerPlayerController()
{
    bAutoManageActiveCameraTarget = false;
}
