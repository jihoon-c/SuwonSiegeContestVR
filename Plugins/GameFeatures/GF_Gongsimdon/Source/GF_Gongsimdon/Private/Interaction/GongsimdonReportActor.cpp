#include "Interaction/GongsimdonReportActor.h"

#include "Components/SceneComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"

AGongsimdonReportActor::AGongsimdonReportActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	ScenarioInteraction = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("ScenarioInteraction"));
	ScenarioInteraction->SupportedInteractionTypes = {EScenarioInteractionType::Custom};
}

void AGongsimdonReportActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ScenarioInteraction)
	{
		ScenarioInteraction->TargetID = TargetID;
		ScenarioInteraction->SetInteractionEnabled(bReportArmed);
	}
}

bool AGongsimdonReportActor::SubmitReport(
	const EGongsimdonReportDirection Direction,
	const int32 EnemyCount)
{
	const bool bCountAccepted = EnemyCount >= MinimumEnemyCount &&
		(MaximumEnemyCount <= 0 || EnemyCount <= MaximumEnemyCount);
	const bool bAccepted = bReportArmed && Direction == ExpectedDirection && bCountAccepted;
	OnReportEvaluated.Broadcast(bAccepted);
	if (!bAccepted || !ScenarioInteraction)
	{
		return false;
	}

	const bool bReported = ScenarioInteraction->ReportInteractionCompleted(EScenarioInteractionType::Custom);
	if (bReported)
	{
		SetReportArmed(false);
	}
	return bReported;
}

void AGongsimdonReportActor::SetReportArmed(const bool bArmed)
{
	bReportArmed = bArmed;
	if (ScenarioInteraction)
	{
		ScenarioInteraction->SetInteractionEnabled(bArmed);
	}
}
