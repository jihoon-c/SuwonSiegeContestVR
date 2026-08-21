#include "Interaction/GongsimdonObservationTargetActor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Core/Scenario/ScenarioObservationComponent.h"

AGongsimdonObservationTargetActor::AGongsimdonObservationTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	EditorMarker = CreateDefaultSubobject<UArrowComponent>(TEXT("EditorMarker"));
	EditorMarker->SetupAttachment(SceneRoot);
	EditorMarker->SetHiddenInGame(true);
	EditorMarker->ArrowColor = FColor::Cyan;
	EditorMarker->ArrowSize = 1.5f;

	Observation = CreateDefaultSubobject<UScenarioObservationComponent>(TEXT("Observation"));
	Observation->ObservationTarget = SceneRoot;
}

void AGongsimdonObservationTargetActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (Observation)
	{
		Observation->TargetID = TargetID;
		Observation->RequiredViewTime = RequiredViewTime;
		Observation->RequiredViewAngle = RequiredViewAngle;
		Observation->MaxDistance = MaxDistance;
		Observation->bRequireLineOfSight = bRequireLineOfSight;
		Observation->SetInteractionEnabled(false);
	}
}

void AGongsimdonObservationTargetActor::ActivateObservation()
{
	if (Observation)
	{
		Observation->SetInteractionEnabled(true);
		Observation->StartObservation();
	}
}

void AGongsimdonObservationTargetActor::DeactivateObservation(const bool bReportFailure)
{
	if (Observation)
	{
		Observation->StopObservation(bReportFailure);
		Observation->SetInteractionEnabled(false);
	}
}
