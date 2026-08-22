#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "OngseongCrossbowGripComponent.generated.h"

class UMotionControllerComponent;
class USceneComponent;

/** Reflection-compatible, two-hand mounted grip consumed by AVRPlayerPawn. */
UCLASS(ClassGroup=(Ongseong), meta=(BlueprintSpawnableComponent))
class GF_ONGSEONGCROSSBOW_API UOngseongCrossbowGripComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UOngseongCrossbowGripComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow|Grip")
	bool TryGrab(UMotionControllerComponent* MotionController);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow|Grip")
	void TryRelease(UMotionControllerComponent* MotionController);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow|Grip")
	void TriggerPressed(UMotionControllerComponent* MotionController);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Crossbow|Grip")
	void TriggerReleased(UMotionControllerComponent* MotionController);

	void SetAimTarget(USceneComponent* NewAimTarget) { AimTarget = NewAimTarget; }
	bool IsTwoHandAiming() const { return LeftController.IsValid() && RightController.IsValid(); }

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Crossbow|Grip")
	bool bIsHeld = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Crossbow|Grip")
	bool bAllowTwoHandedGrab = true;

protected:
	bool IsLeftController(const UMotionControllerComponent* Controller) const;
	void UpdateAim();

	UPROPERTY(Transient)
	TWeakObjectPtr<UMotionControllerComponent> LeftController;
	UPROPERTY(Transient)
	TWeakObjectPtr<UMotionControllerComponent> RightController;
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> AimTarget;
	bool bLeftTriggerPressed = false;
	bool bRightTriggerPressed = false;
};
