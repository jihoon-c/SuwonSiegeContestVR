#include "Core/Debug/DebugFreeCameraGameMode.h"

#include "Core/Debug/DebugFreeCameraPawn.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "StereoRendering.h"

ADebugFreeCameraGameMode::ADebugFreeCameraGameMode()
{
	DefaultPawnClass = ADebugFreeCameraPawn::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
	HUDClass = nullptr;
	bStartPlayersAsSpectators = false;
}

void ADebugFreeCameraGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Uses the stereo device directly so Core keeps depending only on Engine, not on an XR plugin module.
	if (bDisableHMDOnBeginPlay && GEngine && GEngine->StereoRenderingDevice.IsValid() && GEngine->StereoRenderingDevice->IsStereoEnabled())
	{
		GEngine->StereoRenderingDevice->EnableStereo(false);
	}

	if (bCaptureMouseOnBeginPlay)
	{
		if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
}
