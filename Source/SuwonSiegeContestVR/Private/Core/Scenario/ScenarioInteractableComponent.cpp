#include "Core/Scenario/ScenarioInteractableComponent.h"

#include "Core/Scenario/ScenarioManagerComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

UScenarioInteractableComponent::UScenarioInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UScenarioInteractableComponent::SetInteractionEnabled(const bool bEnabled)
{
	bInteractionEnabled = bEnabled;
}

bool UScenarioInteractableComponent::SupportsInteractionType(
	const EScenarioInteractionType InteractionType) const
{
	return SupportedInteractionTypes.Contains(InteractionType);
}

bool UScenarioInteractableComponent::CanReportInteraction(
	const EScenarioInteractionType InteractionType) const
{
	if (!bInteractionEnabled || !SupportsInteractionType(InteractionType) || TargetID.IsNone())
	{
		return false;
	}
	if (bAutoReportToScenarioManager)
	{
		if (const UScenarioManagerComponent* Manager = FindScenarioManager())
		{
			return Manager->CanReportInteractionResult(TargetID, InteractionType);
		}
	}
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionStarted(
	const EScenarioInteractionType InteractionType)
{
	if (!bInteractionEnabled || !SupportsInteractionType(InteractionType) || TargetID.IsNone())
	{
		return false;
	}
	OnInteractionStarted.Broadcast(TargetID, InteractionType);
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionProgress(
	const EScenarioInteractionType InteractionType,
	const float Progress)
{
	if (!bInteractionEnabled || !SupportsInteractionType(InteractionType) || TargetID.IsNone())
	{
		return false;
	}
	OnInteractionProgress.Broadcast(TargetID, InteractionType, FMath::Clamp(Progress, 0.0f, 1.0f));
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionCompleted(
	const EScenarioInteractionType InteractionType)
{
	if (!CanReportInteraction(InteractionType))
	{
		return false;
	}
	if (bAutoReportToScenarioManager)
	{
		if (UScenarioManagerComponent* Manager = FindScenarioManager())
		{
			if (!Manager->ReportInteractionResult(TargetID, InteractionType, true))
			{
				return false;
			}
		}
	}
	OnInteractionCompleted.Broadcast(TargetID, InteractionType);
	return true;
}

bool UScenarioInteractableComponent::ReportInteractionFailed(
	const EScenarioInteractionType InteractionType)
{
	if (!CanReportInteraction(InteractionType))
	{
		return false;
	}
	if (bAutoReportToScenarioManager)
	{
		if (UScenarioManagerComponent* Manager = FindScenarioManager())
		{
			if (!Manager->ReportInteractionResult(TargetID, InteractionType, false))
			{
				return false;
			}
		}
	}
	OnInteractionFailed.Broadcast(TargetID, InteractionType);
	return true;
}

UScenarioManagerComponent* UScenarioInteractableComponent::FindScenarioManager() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		if (UScenarioManagerComponent* Manager = ActorIterator->FindComponentByClass<UScenarioManagerComponent>())
		{
			return Manager;
		}
	}
	return nullptr;
}
