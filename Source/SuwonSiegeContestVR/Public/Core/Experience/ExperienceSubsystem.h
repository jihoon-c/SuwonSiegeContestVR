#pragma once

#include "CoreMinimal.h"
#include "Core/Experience/ExperienceTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExperienceSubsystem.generated.h"

class UExperienceDefinition;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnExperienceStateChanged, FName, ExperienceID, EExperienceState, OldState, EExperienceState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperienceEvent, FName, ExperienceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperienceTravelFailed, FString, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExperienceProgressReset);

/**
 * Owns cross-level experience travel and in-memory progress for the current game session.
 * Level-local Scenario execution remains the responsibility of UScenarioManagerComponent.
 */
UCLASS()
class SUWONSIEGECONTESTVR_API UExperienceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Requests OpenLevel using the supplied definition. */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	bool StartExperience(UExperienceDefinition* Definition, bool bRestartIfAlreadyActive = false);

	/** Marks an already loaded experience active, including direct PIE entry into its level. */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	bool ActivateExperienceForCurrentLevel(UExperienceDefinition* Definition);

	UFUNCTION(BlueprintCallable, Category = "Experience")
	bool CompleteCurrentExperience(bool bRequestConfiguredReturn = true);

	UFUNCTION(BlueprintCallable, Category = "Experience")
	bool CompleteExperience(FName ExperienceID, bool bRequestConfiguredReturn = true);

	UFUNCTION(BlueprintCallable, Category = "Experience")
	bool ReturnToMain();

	UFUNCTION(BlueprintPure, Category = "Experience")
	bool IsExperienceCompleted(FName ExperienceID) const;

	UFUNCTION(BlueprintPure, Category = "Experience")
	FName GetCurrentExperienceID() const { return CurrentExperienceID; }

	UFUNCTION(BlueprintPure, Category = "Experience")
	EExperienceState GetExperienceState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Experience")
	UExperienceDefinition* GetCurrentExperienceDefinition() const { return CurrentExperienceDefinition; }

	UFUNCTION(BlueprintPure, Category = "Experience")
	FExperienceProgressSnapshot GetProgressSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Experience")
	void ResetSessionProgress();

	UPROPERTY(BlueprintAssignable, Category = "Experience|Events")
	FOnExperienceStateChanged OnExperienceStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Experience|Events")
	FOnExperienceEvent OnExperienceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Experience|Events")
	FOnExperienceEvent OnExperienceCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Experience|Events")
	FOnExperienceEvent OnExperienceProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Experience|Events")
	FOnExperienceTravelFailed OnTravelFailed;

	UPROPERTY(BlueprintAssignable, Category = "Experience|Events")
	FOnExperienceProgressReset OnProgressReset;

private:
	void SetState(EExperienceState NewState);
	bool TravelToLevel(const TSoftObjectPtr<UWorld>& Level, const FString& Options, bool bIsReturnTravel);
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void ReportFailure(const FString& ErrorMessage);

	UPROPERTY(Transient)
	TObjectPtr<UExperienceDefinition> CurrentExperienceDefinition;

	UPROPERTY(Transient)
	FName CurrentExperienceID;

	UPROPERTY(Transient)
	TSet<FName> CompletedExperienceIDs;

	EExperienceState State = EExperienceState::Inactive;
	FName PendingDestinationLevelName;
	bool bPendingReturnTravel = false;
};
