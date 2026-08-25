#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "DebugFreeCameraPawn.generated.h"

/**
 * Observer camera for non-VR testing. It never possesses or drives gameplay actors; it only flies
 * around so combat can be watched on a desktop without an HMD.
 *
 * Input is polled from the player controller instead of using axis mappings or an input mapping
 * context, so this pawn works in any level regardless of the project's Enhanced Input setup.
 */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API ADebugFreeCameraPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	ADebugFreeCameraPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category = "Debug|FreeCamera")
	float GetCurrentFlySpeed() const { return CurrentFlySpeed; }

protected:
	void TickMovement(APlayerController* PlayerController, float DeltaSeconds);
	void TickLook(APlayerController* PlayerController, float DeltaSeconds);

	/** Base speed in cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|FreeCamera", meta = (ClampMin = "1.0"))
	float BaseFlySpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|FreeCamera", meta = (ClampMin = "1.0"))
	float FastSpeedMultiplier = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|FreeCamera", meta = (ClampMin = "0.01"))
	float SlowSpeedMultiplier = 0.25f;

	/** Degrees of rotation per unit of mouse delta. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|FreeCamera", meta = (ClampMin = "0.01"))
	float MouseSensitivity = 2.0f;

	/** When true the camera only rotates while the right mouse button is held. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|FreeCamera")
	bool bRequireRightMouseButtonToLook = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|FreeCamera", meta = (ClampMin = "1.0", ClampMax = "89.0"))
	float MaxPitchDegrees = 88.0f;

private:
	float CurrentFlySpeed = 0.0f;
};
