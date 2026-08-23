#include "Nokro/NokroControlGripComponent.h"

#include "MotionControllerComponent.h"
#include "Nokro/NokroCraneActor.h"

UNokroControlGripComponent::UNokroControlGripComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UNokroControlGripComponent::IsLeftController(const UMotionControllerComponent* Controller) const
{
	return Controller && Controller->GetTrackingSource() == EControllerHand::Left;
}

bool UNokroControlGripComponent::TryGrab(UMotionControllerComponent* MotionController)
{
	if (!IsValid(MotionController)) return false;
	TWeakObjectPtr<UMotionControllerComponent>& Slot = IsLeftController(MotionController) ? LeftController : RightController;
	Slot = MotionController;
	if (!DrivingController.IsValid()) DrivingController = MotionController;
	bIsHeld = true;
	CaptureHandleAngle();
	SetComponentTickEnabled(true);
	if (ANokroCraneActor* Crane = Cast<ANokroCraneActor>(GetOwner()))
	{
		Crane->SetJoystickInputCapture(true);
		Crane->NotifyHandleGrabbed();
	}
	return true;
}

void UNokroControlGripComponent::TryRelease(UMotionControllerComponent* MotionController)
{
	if (!MotionController || LeftController.Get() == MotionController)
	{
		LeftController.Reset();
		bLeftTriggerPressed = false;
	}
	if (!MotionController || RightController.Get() == MotionController)
	{
		RightController.Reset();
		bRightTriggerPressed = false;
	}
	if (!MotionController || DrivingController.Get() == MotionController)
	{
		DrivingController = LeftController.IsValid() ? LeftController : RightController;
		CaptureHandleAngle();
	}
	bIsHeld = LeftController.IsValid() || RightController.IsValid();
	bPlacementLatched = false;
	if (!bIsHeld)
	{
		bHasHandleAngle = false;
		SetComponentTickEnabled(false);
	}
	if (ANokroCraneActor* Crane = Cast<ANokroCraneActor>(GetOwner())) Crane->SetJoystickInputCapture(bIsHeld);
}

void UNokroControlGripComponent::TriggerPressed(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTriggerPressed = true;
	if (RightController.Get() == MotionController) bRightTriggerPressed = true;
	if (bLeftTriggerPressed && bRightTriggerPressed && !bPlacementLatched && IsHeldByBothHands())
	{
		bPlacementLatched = true;
		if (ANokroCraneActor* Crane = Cast<ANokroCraneActor>(GetOwner())) Crane->RequestStonePlacement();
	}
}

void UNokroControlGripComponent::TriggerReleased(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTriggerPressed = false;
	if (RightController.Get() == MotionController) bRightTriggerPressed = false;
	if (!bLeftTriggerPressed || !bRightTriggerPressed) bPlacementLatched = false;
}

float UNokroControlGripComponent::CalculateHandleAngle() const
{
	if (!DrivingController.IsValid()) return 0.0f;
	const FVector LocalHand = GetComponentTransform().InverseTransformPosition(DrivingController->GetComponentLocation());
	return FMath::RadiansToDegrees(FMath::Atan2(LocalHand.Z, LocalHand.X));
}

void UNokroControlGripComponent::CaptureHandleAngle()
{
	bHasHandleAngle = DrivingController.IsValid();
	if (bHasHandleAngle) PreviousHandleAngle = CalculateHandleAngle();
}

void UNokroControlGripComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!DrivingController.IsValid()) return;
	const float CurrentAngle = CalculateHandleAngle();
	if (bHasHandleAngle)
	{
		const float DeltaAngle = FMath::FindDeltaAngleDegrees(PreviousHandleAngle, CurrentAngle);
		if (FMath::Abs(DeltaAngle) <= 80.0f && !FMath::IsNearlyZero(DeltaAngle, 0.05f))
		{
			if (ANokroCraneActor* Crane = Cast<ANokroCraneActor>(GetOwner())) Crane->ApplyHandleRotation(DeltaAngle);
		}
	}
	PreviousHandleAngle = CurrentAngle;
	bHasHandleAngle = true;
}
