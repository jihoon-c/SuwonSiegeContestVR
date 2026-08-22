#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NokroControlGripComponent.generated.h"

class UMotionControllerComponent;

/** VRPawn reflection-compatible grip used to crank the handle and detect a two-trigger placement gesture. */
UCLASS(ClassGroup=(VRInteraction), meta=(BlueprintSpawnableComponent))
class GF_GEOJUNGGI_API UNokroControlGripComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UNokroControlGripComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Nokro|Interaction")
	bool TryGrab(UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintCallable, Category="Nokro|Interaction")
	void TryRelease(UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintCallable, Category="Nokro|Interaction")
	void TriggerPressed(UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintCallable, Category="Nokro|Interaction")
	void TriggerReleased(UMotionControllerComponent* MotionController);

	UFUNCTION(BlueprintPure, Category="Nokro|Interaction")
	bool IsHeld() const { return bIsHeld; }

	UFUNCTION(BlueprintPure, Category="Nokro|Interaction")
	bool IsHeldByBothHands() const { return LeftController.IsValid() && RightController.IsValid(); }

	/** Reflection contract consumed by AVRPlayerPawn. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nokro|Interaction")
	bool bIsHeld = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Interaction")
	bool bAllowTwoHandedGrab = true;

	/** Generic AVRPlayerPawn reflection contract: the right stick belongs to the held machine. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Interaction")
	bool bConsumeTurnInput = true;

private:
	bool IsLeftController(const UMotionControllerComponent* Controller) const;
	void CaptureHandleAngle();
	float CalculateHandleAngle() const;

	UPROPERTY()
	TWeakObjectPtr<UMotionControllerComponent> LeftController;
	UPROPERTY()
	TWeakObjectPtr<UMotionControllerComponent> RightController;
	UPROPERTY()
	TWeakObjectPtr<UMotionControllerComponent> DrivingController;

	float PreviousHandleAngle = 0.0f;
	bool bHasHandleAngle = false;
	bool bLeftTriggerPressed = false;
	bool bRightTriggerPressed = false;
	bool bPlacementLatched = false;
};
