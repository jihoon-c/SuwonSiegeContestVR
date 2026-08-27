#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ChongtongAimGripComponent.generated.h"

class UMotionControllerComponent;
class USceneComponent;

/** Two-hand cannon grip. Hand separation controls yaw; authored elevation remains fixed. */
UCLASS(ClassGroup=(VRInteraction), meta=(BlueprintSpawnableComponent))
class GF_ONGSEONGCROSSBOW_API UChongtongAimGripComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UChongtongAimGripComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Aim")
	bool TryGrab(UMotionControllerComponent* MotionController);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Aim")
	void TryRelease(UMotionControllerComponent* MotionController);

	void SetAimTarget(USceneComponent* InAimTarget) { AimTarget = InAimTarget; }

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Aim")
	void TriggerPressed(UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintCallable, Category="Ongseong|Chongtong|Aim")
	void TriggerReleased(UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintPure, Category="Ongseong|Chongtong|Aim")
	bool IsTwoHandAiming() const { return LeftController.IsValid() && RightController.IsValid(); }

	/** AVRPlayerPawn reflection contract; the cannon grip intentionally allows both hands. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Chongtong|Aim")
	bool bIsHeld = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Aim")
	bool bAllowTwoHandedGrab = true;

protected:
	void CaptureBaseline();
	bool IsLeftController(const UMotionControllerComponent* Controller) const;

	UPROPERTY()
	TWeakObjectPtr<UMotionControllerComponent> LeftController;
	UPROPERTY()
	TWeakObjectPtr<UMotionControllerComponent> RightController;
	UPROPERTY()
	TObjectPtr<USceneComponent> AimTarget;

	UPROPERTY(EditAnywhere, Category="Ongseong|Chongtong|Aim")
	float MaxYaw = 70.0f;
	UPROPERTY(EditAnywhere, Category="Ongseong|Chongtong|Aim")
	float RotationSpeed = 70.0f;
	/** Both triggers reach full power after this many seconds; additional hold time is ignored. */
	UPROPERTY(EditAnywhere, Category="Ongseong|Chongtong|Charge", meta=(ClampMin="0.1", Units="s"))
	float MaxChargeDuration = 2.0f;

	FVector BaselineHandDirection = FVector::ForwardVector;
	FRotator BaselineRelativeRotation = FRotator::ZeroRotator;
	bool bHasBaseline = false;
	bool bLeftTrigger = false;
	bool bRightTrigger = false;
	bool bFireLatched = false;
	bool bCharging = false;
	float ChargeElapsed = 0.0f;
};
