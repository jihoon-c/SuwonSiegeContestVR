#include "Interaction/GongsimdonReportActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "UObject/ConstructorHelpers.h"

AGongsimdonReportActor::AGongsimdonReportActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	ReportButton = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReportButton"));
	ReportButton->SetupAttachment(SceneRoot);
	ReportButton->ComponentTags.Add(TEXT("VRGrab"));
	ReportButton->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ReportButton->SetCollisionResponseToAllChannels(ECR_Ignore);
	ReportButton->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.12f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ButtonMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (ButtonMesh.Succeeded())
	{
		ReportButton->SetStaticMesh(ButtonMesh.Object);
	}
	ScenarioInteraction = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("ScenarioInteraction"));
	ScenarioInteraction->SupportedInteractionTypes = {EScenarioInteractionType::Custom};
}

bool AGongsimdonReportActor::HandleVRGrabbed(
	USceneComponent*, UMotionControllerComponent*)
{
	return SubmitReport(ExpectedDirection, MinimumEnemyCount);
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
