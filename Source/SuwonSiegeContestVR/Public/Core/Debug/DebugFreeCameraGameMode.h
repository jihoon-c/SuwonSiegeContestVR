#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DebugFreeCameraGameMode.generated.h"

/**
 * Non-VR test GameMode. Spawns only an observer camera so combat, spawning and siege logic can be
 * verified on a desktop without an HMD or the VR player pawn.
 *
 * This class is Core: it must never reference a Game Feature. Experience-specific setup belongs in
 * a Blueprint child owned by that feature.
 */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API ADebugFreeCameraGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADebugFreeCameraGameMode();

	virtual void BeginPlay() override;

protected:
	/** Stereo rendering is pointless for this mode and hides the desktop view when an HMD is attached. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug|TestMode")
	bool bDisableHMDOnBeginPlay = true;

	/** Keeps the mouse captured by the viewport so the observer camera can look around immediately. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug|TestMode")
	bool bCaptureMouseOnBeginPlay = true;
};
