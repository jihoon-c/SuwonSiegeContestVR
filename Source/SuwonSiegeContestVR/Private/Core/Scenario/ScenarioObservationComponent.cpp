#include "Core/Scenario/ScenarioObservationComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

UScenarioObservationComponent::UScenarioObservationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SupportedInteractionTypes = {EScenarioInteractionType::Observe};
	bInteractionEnabled = false;
}

void UScenarioObservationComponent::StartObservation()
{
	AccumulatedViewTime = 0.0f;
	bObservationActive = true;
	SetInteractionEnabled(true);
	SetComponentTickEnabled(true);
	ReportInteractionStarted(EScenarioInteractionType::Observe);
}

void UScenarioObservationComponent::StopObservation(const bool bReportFailure)
{
	if (bReportFailure && bObservationActive)
	{
		ReportInteractionFailed(EScenarioInteractionType::Observe);
	}
	bObservationActive = false;
	AccumulatedViewTime = 0.0f;
	SetComponentTickEnabled(false);
	SetInteractionEnabled(false);
}

void UScenarioObservationComponent::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bObservationActive)
	{
		return;
	}

	if (IsTargetObserved())
	{
		AccumulatedViewTime += DeltaTime;
		ReportInteractionProgress(EScenarioInteractionType::Observe, GetObservationProgress());
		if (GetObservationProgress() >= 1.0f)
		{
			ReportInteractionCompleted(EScenarioInteractionType::Observe);
			StopObservation(false);
		}
	}
	else
	{
		AccumulatedViewTime = 0.0f;
		ReportInteractionProgress(EScenarioInteractionType::Observe, 0.0f);
	}
}

float UScenarioObservationComponent::GetObservationProgress() const
{
	return RequiredViewTime <= KINDA_SMALL_NUMBER
		? (bObservationActive ? 1.0f : 0.0f)
		: FMath::Clamp(AccumulatedViewTime / RequiredViewTime, 0.0f, 1.0f);
}

bool UScenarioObservationComponent::IsTargetObserved() const
{
	const UWorld* World = GetWorld();
	const APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	const APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
	if (!CameraManager)
	{
		return false;
	}

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector TargetLocation = GetTargetLocation();
	const FVector ToTarget = TargetLocation - CameraLocation;
	if (MaxDistance > 0.0f && ToTarget.SizeSquared() > FMath::Square(MaxDistance))
	{
		return false;
	}

	const float RequiredDot = FMath::Cos(FMath::DegreesToRadians(RequiredViewAngle));
	if (FVector::DotProduct(CameraManager->GetCameraRotation().Vector(), ToTarget.GetSafeNormal()) < RequiredDot)
	{
		return false;
	}

	if (bRequireLineOfSight)
	{
		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ScenarioObservation), true);
		if (const AActor* Owner = GetOwner())
		{
			QueryParams.AddIgnoredActor(Owner);
		}
		if (World->LineTraceSingleByChannel(Hit, CameraLocation, TargetLocation, LineOfSightChannel, QueryParams))
		{
			const AActor* TargetOwner = ObservationTarget ? ObservationTarget->GetOwner() : GetOwner();
			if (Hit.GetActor() != TargetOwner)
			{
				return false;
			}
		}
	}
	return true;
}

FVector UScenarioObservationComponent::GetTargetLocation() const
{
	if (ObservationTarget)
	{
		return ObservationTarget->GetComponentLocation();
	}
	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}
