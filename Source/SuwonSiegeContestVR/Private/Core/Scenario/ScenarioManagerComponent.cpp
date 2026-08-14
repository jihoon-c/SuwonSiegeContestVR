#include "Core/Scenario/ScenarioManagerComponent.h"

#include "Core/Scenario/ScenarioDefinition.h"
#include "Core/Scenario/ScenarioSceneData.h"
#include "Engine/World.h"
#include "TimerManager.h"

UScenarioManagerComponent::UScenarioManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UScenarioManagerComponent::StartScenario(UScenarioDefinition* Scenario)
{
	if (Scenario)
	{
		ScenarioDefinition = Scenario;
	}

	FString ValidationError;
	if (!IsValid(ScenarioDefinition) || !ScenarioDefinition->ValidateScenario(ValidationError))
	{
		FailScenario(IsValid(ScenarioDefinition) ? ValidationError : TEXT("ScenarioDefinition is not assigned."));
		return false;
	}

	ClearTimers();
	CurrentScene = nullptr;
	CurrentSceneID = NAME_None;
	CurrentInteractionID = NAME_None;
	InteractionStates.Reset();
	SetScenarioState(EScenarioState::Running);
	return StartScene(ScenarioDefinition->StartSceneID);
}

void UScenarioManagerComponent::EndScenario()
{
	ClearTimers();
	CurrentInteractionID = NAME_None;
	SetSceneState(EScenarioSceneState::Completed);
	SetScenarioState(EScenarioState::Completed);
	OnScenarioFinished.Broadcast();
}

bool UScenarioManagerComponent::StartScene(const FName SceneID)
{
	if (ScenarioState != EScenarioState::Running || !IsValid(ScenarioDefinition))
	{
		return false;
	}

	UScenarioSceneData* Scene = ScenarioDefinition->FindScene(SceneID);
	FString ValidationError;
	if (!IsValid(Scene) || !Scene->ValidateScene(ValidationError))
	{
		FailScenario(IsValid(Scene) ? ValidationError : FString::Printf(TEXT("Scene %s was not found."), *SceneID.ToString()));
		return false;
	}

	ClearTimers();
	if (SceneState != EScenarioSceneState::Inactive)
	{
		SetSceneState(EScenarioSceneState::Inactive);
	}
	CurrentScene = Scene;
	CurrentSceneID = SceneID;
	CurrentInteractionID = NAME_None;
	InteractionStates.Reset();
	for (const FScenarioInteraction& Interaction : Scene->Interactions)
	{
		InteractionStates.Add(Interaction.InteractionID, EScenarioInteractionState::Inactive);
	}
	SetSceneState(EScenarioSceneState::Running);
	return StartInteraction(Scene->StartInteractionID);
}

bool UScenarioManagerComponent::CompleteScene()
{
	if (SceneState != EScenarioSceneState::Running || !AreRequiredInteractionsComplete())
	{
		return false;
	}

	SetSceneState(EScenarioSceneState::Completed);
	return MoveToNextScene();
}

bool UScenarioManagerComponent::MoveToNextScene()
{
	if (!IsValid(CurrentScene))
	{
		return false;
	}

	if (CurrentScene->NextSceneID.IsNone())
	{
		EndScenario();
		return true;
	}
	return StartScene(CurrentScene->NextSceneID);
}

bool UScenarioManagerComponent::StartInteraction(const FName InteractionID)
{
	if (SceneState != EScenarioSceneState::Running || !IsValid(CurrentScene))
	{
		return false;
	}

	const FScenarioInteraction* Interaction = CurrentScene->FindInteraction(InteractionID);
	if (!Interaction)
	{
		FailScenario(FString::Printf(TEXT("Interaction %s was not found in scene %s."),
			*InteractionID.ToString(), *CurrentSceneID.ToString()));
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(StartDelayTimer);
	PendingInteractionID = InteractionID;
	SetInteractionState(InteractionID, EScenarioInteractionState::Ready);
	if (Interaction->DelayBeforeStart > KINDA_SMALL_NUMBER)
	{
		GetWorld()->GetTimerManager().SetTimer(StartDelayTimer, this,
			&ThisClass::BeginPendingInteraction, Interaction->DelayBeforeStart, false);
	}
	else
	{
		BeginPendingInteraction();
	}
	return true;
}

void UScenarioManagerComponent::BeginPendingInteraction()
{
	CurrentInteractionID = PendingInteractionID;
	PendingInteractionID = NAME_None;
	const FScenarioInteraction* Interaction = FindCurrentInteraction();
	if (!Interaction)
	{
		FailScenario(TEXT("Pending interaction became invalid."));
		return;
	}

	SetInteractionState(CurrentInteractionID, EScenarioInteractionState::Running);
	OnInteractionRequested.Broadcast(*Interaction);
	if (Interaction->InteractionType == EScenarioInteractionType::Narration)
	{
		OnNarrationRequested.Broadcast(Interaction->NarrationID, Interaction->InteractionID);
	}
	if (Interaction->InteractionType == EScenarioInteractionType::Objective)
	{
		OnObjectiveRequested.Broadcast(Interaction->ObjectiveText, Interaction->InteractionID);
	}
	if (GetInteractionState(Interaction->InteractionID) != EScenarioInteractionState::Running)
	{
		return;
	}

	if (Interaction->InteractionType == EScenarioInteractionType::Wait)
	{
		if (Interaction->Duration <= KINDA_SMALL_NUMBER)
		{
			CompleteInteraction(Interaction->InteractionID);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(WaitTimer, this,
				&ThisClass::HandleWaitCompleted, Interaction->Duration, false);
		}
	}
	else if (Interaction->bCompleteOnStart)
	{
		CompleteInteraction(Interaction->InteractionID);
	}
}

bool UScenarioManagerComponent::CompleteInteraction(const FName InteractionID)
{
	if (InteractionID != CurrentInteractionID ||
		GetInteractionState(InteractionID) != EScenarioInteractionState::Running)
	{
		return false;
	}

	const FScenarioInteraction* Interaction = FindCurrentInteraction();
	if (!Interaction)
	{
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(WaitTimer);
	PendingNextInteractionID = !Interaction->SuccessInteractionID.IsNone()
		? Interaction->SuccessInteractionID : Interaction->NextInteractionID;
	const float Delay = Interaction->DelayAfterComplete;
	SetInteractionState(InteractionID, EScenarioInteractionState::Completed);

	if (Delay > KINDA_SMALL_NUMBER)
	{
		GetWorld()->GetTimerManager().SetTimer(CompletionDelayTimer, this,
			&ThisClass::AdvanceAfterInteraction, Delay, false);
	}
	else
	{
		AdvanceAfterInteraction();
	}
	return true;
}

bool UScenarioManagerComponent::FailInteraction(const FName InteractionID)
{
	if (InteractionID != CurrentInteractionID ||
		GetInteractionState(InteractionID) != EScenarioInteractionState::Running)
	{
		return false;
	}

	const FScenarioInteraction* Interaction = FindCurrentInteraction();
	if (!Interaction)
	{
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(WaitTimer);
	SetInteractionState(InteractionID, EScenarioInteractionState::Failed);
	if (!Interaction->FailInteractionID.IsNone())
	{
		return StartInteraction(Interaction->FailInteractionID);
	}

	SetSceneState(EScenarioSceneState::Failed);
	FailScenario(FString::Printf(TEXT("Interaction %s failed without a FailInteractionID."),
		*InteractionID.ToString()));
	return true;
}

bool UScenarioManagerComponent::ReportInteractionResult(
	const FName TargetID,
	const EScenarioInteractionType InteractionType,
	const bool bSuccess)
{
	const FScenarioInteraction* Interaction = FindCurrentInteraction();
	if (!Interaction || GetInteractionState(CurrentInteractionID) != EScenarioInteractionState::Running ||
		Interaction->InteractionType != InteractionType ||
		(!Interaction->TargetID.IsNone() && Interaction->TargetID != TargetID))
	{
		return false;
	}
	return bSuccess ? CompleteInteraction(CurrentInteractionID) : FailInteraction(CurrentInteractionID);
}

void UScenarioManagerComponent::AdvanceAfterInteraction()
{
	if (!PendingNextInteractionID.IsNone())
	{
		const FName NextID = PendingNextInteractionID;
		PendingNextInteractionID = NAME_None;
		StartInteraction(NextID);
		return;
	}

	if (!CompleteScene())
	{
		FailScenario(FString::Printf(TEXT("Scene %s reached the end of its flow before all required interactions completed."),
			*CurrentSceneID.ToString()));
	}
}

void UScenarioManagerComponent::HandleWaitCompleted()
{
	CompleteInteraction(CurrentInteractionID);
}

bool UScenarioManagerComponent::RestartScenario()
{
	return StartScenario(ScenarioDefinition);
}

bool UScenarioManagerComponent::RestartScene()
{
	return StartScene(CurrentSceneID);
}

bool UScenarioManagerComponent::RestartInteraction()
{
	GetWorld()->GetTimerManager().ClearTimer(WaitTimer);
	return StartInteraction(CurrentInteractionID);
}

bool UScenarioManagerComponent::SkipCurrentInteraction()
{
	const FScenarioInteraction* Interaction = FindCurrentInteraction();
	if (!Interaction || GetInteractionState(CurrentInteractionID) != EScenarioInteractionState::Running)
	{
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(WaitTimer);
	const FName NextID = Interaction->NextInteractionID;
	SetInteractionState(CurrentInteractionID, EScenarioInteractionState::Skipped);
	if (!NextID.IsNone())
	{
		return StartInteraction(NextID);
	}
	return CompleteScene();
}

bool UScenarioManagerComponent::CompleteCurrentInteraction()
{
	return CompleteInteraction(CurrentInteractionID);
}

bool UScenarioManagerComponent::GoToScene(const FName SceneID)
{
	return StartScene(SceneID);
}

bool UScenarioManagerComponent::GoToInteraction(const FName InteractionID)
{
	return StartInteraction(InteractionID);
}

FScenarioInteraction UScenarioManagerComponent::GetCurrentInteraction() const
{
	const FScenarioInteraction* Interaction = FindCurrentInteraction();
	return Interaction ? *Interaction : FScenarioInteraction();
}

FScenarioDebugSnapshot UScenarioManagerComponent::GetDebugSnapshot() const
{
	FScenarioDebugSnapshot Snapshot;
	Snapshot.ScenarioID = IsValid(ScenarioDefinition) ? ScenarioDefinition->ScenarioID : NAME_None;
	Snapshot.SceneID = CurrentSceneID;
	Snapshot.InteractionID = CurrentInteractionID;
	Snapshot.ScenarioState = ScenarioState;
	Snapshot.SceneState = SceneState;
	Snapshot.InteractionState = GetInteractionState(CurrentInteractionID);
	if (const FScenarioInteraction* Interaction = FindCurrentInteraction())
	{
		Snapshot.InteractionType = Interaction->InteractionType;
		Snapshot.TargetID = Interaction->TargetID;
	}
	return Snapshot;
}

void UScenarioManagerComponent::PrintDebugState() const
{
	const FScenarioDebugSnapshot Snapshot = GetDebugSnapshot();
	UE_LOG(LogTemp, Display, TEXT("Scenario=%s Scene=%s Interaction=%s Type=%s Target=%s States=%s/%s/%s"),
		*Snapshot.ScenarioID.ToString(), *Snapshot.SceneID.ToString(), *Snapshot.InteractionID.ToString(),
		*UEnum::GetValueAsString(Snapshot.InteractionType), *Snapshot.TargetID.ToString(),
		*UEnum::GetValueAsString(Snapshot.ScenarioState), *UEnum::GetValueAsString(Snapshot.SceneState),
		*UEnum::GetValueAsString(Snapshot.InteractionState));
}

EScenarioInteractionState UScenarioManagerComponent::GetInteractionState(const FName InteractionID) const
{
	if (const EScenarioInteractionState* State = InteractionStates.Find(InteractionID))
	{
		return *State;
	}
	return EScenarioInteractionState::Inactive;
}

void UScenarioManagerComponent::SetScenarioState(const EScenarioState NewState)
{
	if (ScenarioState == NewState)
	{
		return;
	}
	const EScenarioState OldState = ScenarioState;
	ScenarioState = NewState;
	OnScenarioStateChanged.Broadcast(OldState, NewState);
}

void UScenarioManagerComponent::SetSceneState(const EScenarioSceneState NewState)
{
	if (SceneState == NewState)
	{
		return;
	}
	const EScenarioSceneState OldState = SceneState;
	SceneState = NewState;
	OnSceneStateChanged.Broadcast(CurrentSceneID, OldState, NewState);
}

void UScenarioManagerComponent::SetInteractionState(
	const FName InteractionID,
	const EScenarioInteractionState NewState)
{
	InteractionStates.FindOrAdd(InteractionID) = NewState;
	if (IsValid(CurrentScene))
	{
		if (const FScenarioInteraction* Interaction = CurrentScene->FindInteraction(InteractionID))
		{
			OnInteractionStateChanged.Broadcast(*Interaction, NewState);
		}
	}
}

bool UScenarioManagerComponent::AreRequiredInteractionsComplete() const
{
	if (!IsValid(CurrentScene))
	{
		return false;
	}
	for (const FScenarioInteraction& Interaction : CurrentScene->Interactions)
	{
		const EScenarioInteractionState State = GetInteractionState(Interaction.InteractionID);
		if (Interaction.bRequired &&
			State != EScenarioInteractionState::Completed &&
			State != EScenarioInteractionState::Skipped)
		{
			return false;
		}
	}
	return true;
}

const FScenarioInteraction* UScenarioManagerComponent::FindCurrentInteraction() const
{
	return IsValid(CurrentScene) ? CurrentScene->FindInteraction(CurrentInteractionID) : nullptr;
}

void UScenarioManagerComponent::FailScenario(const FString& Reason)
{
	ClearTimers();
	UE_LOG(LogTemp, Error, TEXT("Scenario validation/runtime failure: %s"), *Reason);
	OnValidationFailed.Broadcast(Reason);
	if (SceneState == EScenarioSceneState::Running)
	{
		SetSceneState(EScenarioSceneState::Failed);
	}
	SetScenarioState(EScenarioState::Failed);
}

void UScenarioManagerComponent::ClearTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartDelayTimer);
		World->GetTimerManager().ClearTimer(CompletionDelayTimer);
		World->GetTimerManager().ClearTimer(WaitTimer);
	}
	PendingInteractionID = NAME_None;
	PendingNextInteractionID = NAME_None;
}

void UScenarioManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearTimers();
	Super::EndPlay(EndPlayReason);
}
