#include "Ongseong/ChongtongAimGripComponent.h"

#include "MotionControllerComponent.h"
#include "Ongseong/ChongtongCannonActor.h"

UChongtongAimGripComponent::UChongtongAimGripComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UChongtongAimGripComponent::IsLeftController(const UMotionControllerComponent* Controller) const
{
	return Controller && Controller->GetTrackingSource() == EControllerHand::Left;
}

bool UChongtongAimGripComponent::TryGrab(UMotionControllerComponent* MotionController)
{
	if (!IsValid(MotionController)) return false;
	(IsLeftController(MotionController) ? LeftController : RightController) = MotionController;
	bIsHeld = LeftController.IsValid() || RightController.IsValid();
	if (IsTwoHandAiming())
	{
		CaptureBaseline();
		SetComponentTickEnabled(true);
		if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner())) Cannon->BeginPlayerAim();
	}
	return true;
}

void UChongtongAimGripComponent::TryRelease(UMotionControllerComponent* MotionController)
{
	if (!MotionController || LeftController.Get() == MotionController) LeftController.Reset();
	if (!MotionController || RightController.Get() == MotionController) RightController.Reset();
	bIsHeld = LeftController.IsValid() || RightController.IsValid();
	bHasBaseline = false;
	bFireLatched = false;
	if (!IsTwoHandAiming())
	{
		SetComponentTickEnabled(false);
		if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner())) Cannon->EndPlayerAim();
	}
}

void UChongtongAimGripComponent::CaptureBaseline()
{
	if (!IsTwoHandAiming() || !AimTarget) return;
	BaselineHandDirection = (RightController->GetComponentLocation() - LeftController->GetComponentLocation()).GetSafeNormal();
	BaselineRelativeRotation = AimTarget->GetRelativeRotation();
	bHasBaseline = !BaselineHandDirection.IsNearlyZero();
}

void UChongtongAimGripComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsTwoHandAiming() || !bHasBaseline || !AimTarget) return;
	const FVector Current = (RightController->GetComponentLocation() - LeftController->GetComponentLocation()).GetSafeNormal();
	if (Current.IsNearlyZero()) return;
	const FQuat Delta = FQuat::FindBetweenNormals(BaselineHandDirection, Current);
	FRotator Desired = (Delta * BaselineRelativeRotation.Quaternion()).Rotator();
	Desired.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Desired.Pitch), MinPitch, MaxPitch);
	Desired.Yaw = FMath::Clamp(FRotator::NormalizeAxis(Desired.Yaw), -MaxYaw, MaxYaw);
	Desired.Roll = 0.0f;
	AimTarget->SetRelativeRotation(FMath::RInterpConstantTo(AimTarget->GetRelativeRotation(), Desired, DeltaTime, RotationSpeed));
}

void UChongtongAimGripComponent::TriggerPressed(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTrigger = true;
	if (RightController.Get() == MotionController) bRightTrigger = true;
	if (bLeftTrigger && bRightTrigger && !bFireLatched && IsTwoHandAiming())
	{
		bFireLatched = true;
		if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner())) Cannon->TryFirePlayer();
	}
}

void UChongtongAimGripComponent::TriggerReleased(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTrigger = false;
	if (RightController.Get() == MotionController) bRightTrigger = false;
	if (!bLeftTrigger || !bRightTrigger) bFireLatched = false;
}
