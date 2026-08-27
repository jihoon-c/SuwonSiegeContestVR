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
	if (!MotionController || LeftController.Get() == MotionController)
	{
		LeftController.Reset();
		bLeftTrigger = false;
	}
	if (!MotionController || RightController.Get() == MotionController)
	{
		RightController.Reset();
		bRightTrigger = false;
	}
	bIsHeld = LeftController.IsValid() || RightController.IsValid();
	bHasBaseline = false;
	bCharging = false;
	ChargeElapsed = 0.0f;
	if (!bLeftTrigger && !bRightTrigger) bFireLatched = false;
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
	if (bCharging)
	{
		ChargeElapsed = FMath::Min(ChargeElapsed + DeltaTime, FMath::Max(0.1f, MaxChargeDuration));
		if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner()))
		{
			Cannon->SetPlayerChargePercent(ChargeElapsed / FMath::Max(0.1f, MaxChargeDuration));
		}
	}
	const FVector Current = (RightController->GetComponentLocation() - LeftController->GetComponentLocation()).GetSafeNormal();
	if (Current.IsNearlyZero()) return;
	FVector BaselineHorizontal = BaselineHandDirection;
	FVector CurrentHorizontal = Current;
	BaselineHorizontal.Z = 0.0f;
	CurrentHorizontal.Z = 0.0f;
	if (!BaselineHorizontal.Normalize() || !CurrentHorizontal.Normalize()) return;
	const float DeltaYaw = FMath::FindDeltaAngleDegrees(BaselineHorizontal.Rotation().Yaw, CurrentHorizontal.Rotation().Yaw);
	FRotator Desired = BaselineRelativeRotation;
	Desired.Yaw = BaselineRelativeRotation.Yaw + FMath::Clamp(DeltaYaw, -MaxYaw, MaxYaw);
	AimTarget->SetRelativeRotation(FMath::RInterpConstantTo(AimTarget->GetRelativeRotation(), Desired, DeltaTime, RotationSpeed));
}

void UChongtongAimGripComponent::TriggerPressed(UMotionControllerComponent* MotionController)
{
	if (LeftController.Get() == MotionController) bLeftTrigger = true;
	if (RightController.Get() == MotionController) bRightTrigger = true;
	if (bLeftTrigger && bRightTrigger && !bFireLatched && IsTwoHandAiming())
	{
		bFireLatched = true;
		bCharging = true;
		ChargeElapsed = 0.0f;
		if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner()))
		{
			Cannon->SetPlayerChargePercent(0.0f);
			Cannon->SetPlayerChargeVisible(true);
		}
	}
}

void UChongtongAimGripComponent::TriggerReleased(UMotionControllerComponent* MotionController)
{
	const bool bWasCharging = bCharging;
	if (LeftController.Get() == MotionController) bLeftTrigger = false;
	if (RightController.Get() == MotionController) bRightTrigger = false;
	if (bWasCharging)
	{
		bCharging = false;
		const float ChargeAlpha = ChargeElapsed / FMath::Max(0.1f, MaxChargeDuration);
		if (AChongtongCannonActor* Cannon = Cast<AChongtongCannonActor>(GetOwner()))
		{
			Cannon->TryFirePlayerCharged(ChargeAlpha);
			Cannon->SetPlayerChargeVisible(false);
		}
	}
	if (!bLeftTrigger && !bRightTrigger) bFireLatched = false;
}
