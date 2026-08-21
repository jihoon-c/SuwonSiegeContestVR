#include "Scenario/GongsimdonScenarioDirectorActor.h"

#include "Components/SceneComponent.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Enemy/GongsimdonEnemyGroupActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Interaction/GongsimdonCombatTargetActor.h"
#include "Interaction/GongsimdonObservationTargetActor.h"
#include "Interaction/GongsimdonReportActor.h"
#include "TimerManager.h"

AGongsimdonScenarioDirectorActor::AGongsimdonScenarioDirectorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AGongsimdonScenarioDirectorActor::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		InitializeDirector();
	}));
}

void AGongsimdonScenarioDirectorActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ScenarioManager)
	{
		ScenarioManager->OnInteractionRequested.RemoveDynamic(this, &ThisClass::HandleInteractionRequested);
		ScenarioManager->OnInteractionStateChanged.RemoveDynamic(this, &ThisClass::HandleInteractionStateChanged);
	}
	DeactivateAllTargets();
	Super::EndPlay(EndPlayReason);
}

bool AGongsimdonScenarioDirectorActor::InitializeDirector()
{
	if (!GetWorld())
	{
		return false;
	}

	if (!ScenarioManager)
	{
		for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
		{
			if (UScenarioManagerComponent* Candidate =
				ActorIterator->FindComponentByClass<UScenarioManagerComponent>())
			{
				ScenarioManager = Candidate;
				break;
			}
		}
	}
	if (!ScenarioManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("Gongsimdon director could not find a Scenario Manager."));
		return false;
	}

	ScenarioManager->OnInteractionRequested.AddUniqueDynamic(this, &ThisClass::HandleInteractionRequested);
	ScenarioManager->OnInteractionStateChanged.AddUniqueDynamic(this, &ThisClass::HandleInteractionStateChanged);

	const FScenarioInteraction CurrentInteraction = ScenarioManager->GetCurrentInteraction();
	if (!CurrentInteraction.InteractionID.IsNone() &&
		ScenarioManager->GetInteractionState(CurrentInteraction.InteractionID) == EScenarioInteractionState::Running)
	{
		HandleInteractionRequested(CurrentInteraction);
	}
	return true;
}

void AGongsimdonScenarioDirectorActor::HandleInteractionRequested(FScenarioInteraction Interaction)
{
	DeactivateAllTargets();
	switch (Interaction.InteractionType)
	{
	case EScenarioInteractionType::Observe:
		if (!ActivateObservationTarget(Interaction.TargetID))
		{
			UE_LOG(LogTemp, Warning, TEXT("No Gongsimdon observation target found for %s."),
				*Interaction.TargetID.ToString());
		}
		break;
	case EScenarioInteractionType::Sequence:
	case EScenarioInteractionType::Spawn:
		OnCueRequested.Broadcast(Interaction.InteractionID, Interaction.TargetID);
		break;
	case EScenarioInteractionType::Custom:
		ArmReportTarget(Interaction.TargetID);
		OnActionRequested.Broadcast(Interaction.InteractionID, Interaction.TargetID);
		break;
	case EScenarioInteractionType::Combat:
		ArmCombatTarget(Interaction.TargetID);
		OnActionRequested.Broadcast(Interaction.InteractionID, Interaction.TargetID);
		break;
	default:
		OnActionRequested.Broadcast(Interaction.InteractionID, Interaction.TargetID);
		break;
	}
}

void AGongsimdonScenarioDirectorActor::HandleInteractionStateChanged(
	FScenarioInteraction Interaction,
	const EScenarioInteractionState State)
{
	if (State == EScenarioInteractionState::Completed ||
		State == EScenarioInteractionState::Failed ||
		State == EScenarioInteractionState::Skipped)
	{
		DeactivateAllTargets();
	}
}

void AGongsimdonScenarioDirectorActor::DeactivateAllTargets()
{
	if (!GetWorld())
	{
		return;
	}
	for (TActorIterator<AGongsimdonObservationTargetActor> It(GetWorld()); It; ++It)
	{
		It->DeactivateObservation(false);
	}
	for (TActorIterator<AGongsimdonReportActor> It(GetWorld()); It; ++It)
	{
		It->SetReportArmed(false);
	}
	for (TActorIterator<AGongsimdonCombatTargetActor> It(GetWorld()); It; ++It)
	{
		It->SetCombatArmed(false);
	}
	for (TActorIterator<AGongsimdonEnemyGroupActor> It(GetWorld()); It; ++It)
	{
		It->DeactivateObservation(false);
		It->SetCombatArmed(false);
	}
}

bool AGongsimdonScenarioDirectorActor::ActivateObservationTarget(const FName TargetID)
{
	for (TActorIterator<AGongsimdonEnemyGroupActor> It(GetWorld()); It; ++It)
	{
		if (It->ObservationTargetID == TargetID)
		{
			It->ActivateObservation();
			return true;
		}
	}
	for (TActorIterator<AGongsimdonObservationTargetActor> It(GetWorld()); It; ++It)
	{
		if (It->GetTargetID() == TargetID)
		{
			It->ActivateObservation();
			return true;
		}
	}
	return false;
}

bool AGongsimdonScenarioDirectorActor::ArmReportTarget(const FName TargetID)
{
	for (TActorIterator<AGongsimdonReportActor> It(GetWorld()); It; ++It)
	{
		if (It->GetTargetID() == TargetID)
		{
			It->SetReportArmed(true);
			return true;
		}
	}
	return false;
}

bool AGongsimdonScenarioDirectorActor::ArmCombatTarget(const FName TargetID)
{
	for (TActorIterator<AGongsimdonEnemyGroupActor> It(GetWorld()); It; ++It)
	{
		if (It->CombatTargetID == TargetID)
		{
			It->SetCombatArmed(true);
			return true;
		}
	}
	for (TActorIterator<AGongsimdonCombatTargetActor> It(GetWorld()); It; ++It)
	{
		if (It->GetTargetID() == TargetID)
		{
			It->SetCombatArmed(true);
			return true;
		}
	}
	return false;
}
