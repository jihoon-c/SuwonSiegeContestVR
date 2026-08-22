#include "Ongseong/OngseongCrossbowGripComponent.h"

#include "MotionControllerComponent.h"
#include "Ongseong/OngseongCrossbowActor.h"

UOngseongCrossbowGripComponent::UOngseongCrossbowGripComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UOngseongCrossbowGripComponent::IsLeftController(const UMotionControllerComponent* Controller) const
{
	return Controller && Controller->GetTrackingSource() == EControllerHand::Left;
}

bool UOngseongCrossbowGripComponent::TryGrab(UMotionControllerComponent* MotionController)
{
	if (!MotionController) return false;
	TWeakObjectPtr<UMotionControllerComponent>& Slot = IsLeftController(MotionController) ? LeftController : RightController;
	if (Slot.IsValid() && Slot.Get() != MotionController) return false;
	Slot = MotionController;
	bIsHeld = LeftController.IsValid() || RightController.IsValid();
	if (AOngseongCrossbowActor* Crossbow = Cast<AOngseongCrossbowActor>(GetOwner())) Crossbow->HandleGripStateChanged(IsTwoHandAiming());
	return true;
}

void UOngseongCrossbowGripComponent::TryRelease(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) { LeftController.Reset(); bLeftTriggerPressed = false; }
	if (RightController.Get() == MotionController) { RightController.Reset(); bRightTriggerPressed = false; }
	bIsHeld = LeftController.IsValid() || RightController.IsValid();
	if (AOngseongCrossbowActor* Crossbow = Cast<AOngseongCrossbowActor>(GetOwner())) Crossbow->HandleGripStateChanged(IsTwoHandAiming());
}

void UOngseongCrossbowGripComponent::TriggerPressed(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTriggerPressed = true;
	if (RightController.Get() == MotionController) bRightTriggerPressed = true;
	if (IsTwoHandAiming() && (bLeftTriggerPressed || bRightTriggerPressed))
	{
		if (AOngseongCrossbowActor* Crossbow = Cast<AOngseongCrossbowActor>(GetOwner())) Crossbow->TryFire();
	}
}

void UOngseongCrossbowGripComponent::TriggerReleased(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTriggerPressed = false;
	if (RightController.Get() == MotionController) bRightTriggerPressed = false;
}

void UOngseongCrossbowGripComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateAim();
}

void UOngseongCrossbowGripComponent::UpdateAim()
{
	if (!IsTwoHandAiming() || !AimTarget) return;
	const FVector Midpoint = (LeftController->GetComponentLocation() + RightController->GetComponentLocation()) * 0.5f;
	FVector AimDirection = Midpoint - GetComponentLocation();
	if (AimDirection.IsNearlyZero()) return;
	const FRotator DesiredWorldRotation = AimDirection.Rotation();
	AimTarget->SetWorldRotation(FRotator(
		FMath::ClampAngle(DesiredWorldRotation.Pitch, -20.0f, 55.0f),
		DesiredWorldRotation.Yaw,
		0.0f));
}
