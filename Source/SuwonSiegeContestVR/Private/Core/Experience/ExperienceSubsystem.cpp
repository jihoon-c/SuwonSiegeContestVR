#include "Core/Experience/ExperienceSubsystem.h"

#include "Core/Experience/ExperienceDefinition.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

void UExperienceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

void UExperienceSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	CurrentExperienceDefinition = nullptr;
	CurrentExperienceID = NAME_None;
	PendingDestinationLevelName = NAME_None;
	bPendingReturnTravel = false;
	State = EExperienceState::Inactive;
	Super::Deinitialize();
}

bool UExperienceSubsystem::StartExperience(
	UExperienceDefinition* Definition,
	const bool bRestartIfAlreadyActive)
{
	FString ValidationError;
	if (!IsValid(Definition) || !Definition->ValidateDefinition(ValidationError))
	{
		ReportFailure(IsValid(Definition) ? ValidationError : TEXT("ExperienceDefinition is not assigned."));
		return false;
	}

	if (!bRestartIfAlreadyActive && CurrentExperienceID == Definition->ExperienceID &&
		(State == EExperienceState::Active || State == EExperienceState::Traveling))
	{
		return false;
	}

	CurrentExperienceDefinition = Definition;
	CurrentExperienceID = Definition->ExperienceID;
	return TravelToLevel(Definition->ExperienceLevel, Definition->TravelOptions, false);
}

bool UExperienceSubsystem::ActivateExperienceForCurrentLevel(UExperienceDefinition* Definition)
{
	FString ValidationError;
	if (!IsValid(Definition) || !Definition->ValidateDefinition(ValidationError))
	{
		ReportFailure(IsValid(Definition) ? ValidationError : TEXT("ExperienceDefinition is not assigned."));
		return false;
	}

	CurrentExperienceDefinition = Definition;
	CurrentExperienceID = Definition->ExperienceID;
	PendingDestinationLevelName = NAME_None;
	bPendingReturnTravel = false;
	SetState(EExperienceState::Active);
	OnExperienceStarted.Broadcast(CurrentExperienceID);
	return true;
}

bool UExperienceSubsystem::CompleteCurrentExperience(const bool bRequestConfiguredReturn)
{
	return CompleteExperience(CurrentExperienceID, bRequestConfiguredReturn);
}

bool UExperienceSubsystem::CompleteExperience(
	const FName ExperienceID,
	const bool bRequestConfiguredReturn)
{
	if (ExperienceID.IsNone() || ExperienceID != CurrentExperienceID ||
		(State != EExperienceState::Active && State != EExperienceState::Completed))
	{
		return false;
	}

	const bool bWasAlreadyCompleted = CompletedExperienceIDs.Contains(ExperienceID);
	if (bWasAlreadyCompleted && State == EExperienceState::Completed)
	{
		return true;
	}

	CompletedExperienceIDs.Add(ExperienceID);
	SetState(EExperienceState::Completed);
	OnExperienceCompleted.Broadcast(ExperienceID);
	if (!bWasAlreadyCompleted)
	{
		OnExperienceProgressChanged.Broadcast(ExperienceID);
	}

	if (bRequestConfiguredReturn && CurrentExperienceDefinition &&
		CurrentExperienceDefinition->bReturnOnCompletion && !CurrentExperienceDefinition->ReturnLevel.IsNull())
	{
		return ReturnToMain();
	}
	return true;
}

bool UExperienceSubsystem::ReturnToMain()
{
	if (!CurrentExperienceDefinition || CurrentExperienceDefinition->ReturnLevel.IsNull())
	{
		ReportFailure(TEXT("The current ExperienceDefinition has no ReturnLevel."));
		return false;
	}

	return TravelToLevel(CurrentExperienceDefinition->ReturnLevel, FString(), true);
}

bool UExperienceSubsystem::IsExperienceCompleted(const FName ExperienceID) const
{
	return !ExperienceID.IsNone() && CompletedExperienceIDs.Contains(ExperienceID);
}

FExperienceProgressSnapshot UExperienceSubsystem::GetProgressSnapshot() const
{
	FExperienceProgressSnapshot Snapshot;
	Snapshot.CurrentExperienceID = CurrentExperienceID;
	Snapshot.State = State;
	Snapshot.CompletedExperienceIDs.Reserve(CompletedExperienceIDs.Num());
	for (const FName CompletedExperienceID : CompletedExperienceIDs)
	{
		Snapshot.CompletedExperienceIDs.Add(CompletedExperienceID);
	}
	Snapshot.CompletedExperienceIDs.Sort(FNameLexicalLess());
	return Snapshot;
}

void UExperienceSubsystem::ResetSessionProgress()
{
	CompletedExperienceIDs.Reset();
	OnProgressReset.Broadcast();
}

void UExperienceSubsystem::SetState(const EExperienceState NewState)
{
	if (State == NewState)
	{
		return;
	}

	const EExperienceState OldState = State;
	State = NewState;
	OnExperienceStateChanged.Broadcast(CurrentExperienceID, OldState, State);
}

bool UExperienceSubsystem::TravelToLevel(
	const TSoftObjectPtr<UWorld>& Level,
	const FString& Options,
	const bool bIsReturnTravel)
{
	if (Level.IsNull())
	{
		ReportFailure(TEXT("The requested travel level is not assigned."));
		return false;
	}

	PendingDestinationLevelName = FName(*Level.ToSoftObjectPath().GetAssetName());
	bPendingReturnTravel = bIsReturnTravel;
	SetState(EExperienceState::Traveling);
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Level, true, Options);
	return true;
}

void UExperienceSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || State != EExperienceState::Traveling || PendingDestinationLevelName.IsNone())
	{
		return;
	}

	const FName LoadedLevelName(*UGameplayStatics::GetCurrentLevelName(LoadedWorld, true));
	if (LoadedLevelName != PendingDestinationLevelName)
	{
		return;
	}

	PendingDestinationLevelName = NAME_None;
	if (bPendingReturnTravel)
	{
		bPendingReturnTravel = false;
		CurrentExperienceDefinition = nullptr;
		CurrentExperienceID = NAME_None;
		SetState(EExperienceState::Inactive);
		return;
	}

	SetState(EExperienceState::Active);
	OnExperienceStarted.Broadcast(CurrentExperienceID);
}

void UExperienceSubsystem::ReportFailure(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Error, TEXT("Experience failure: %s"), *ErrorMessage);
	PendingDestinationLevelName = NAME_None;
	bPendingReturnTravel = false;
	SetState(EExperienceState::Failed);
	OnTravelFailed.Broadcast(ErrorMessage);
}
