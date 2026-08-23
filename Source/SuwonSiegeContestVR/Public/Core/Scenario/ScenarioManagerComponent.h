#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "ScenarioManagerComponent.generated.h"

class UScenarioDefinition;
class UScenarioSceneData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioStateChanged, EScenarioState, OldState, EScenarioState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScenarioSceneChanged, FName, SceneID, EScenarioSceneState, OldState, EScenarioSceneState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioInteractionChanged, FScenarioInteraction, Interaction, EScenarioInteractionState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioInteractionRequested, FScenarioInteraction, Interaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioNarrationRequested, FName, NarrationID, FName, InteractionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScenarioObjectiveRequested, FText, ObjectiveText, FName, InteractionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScenarioValidationFailed, FString, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScenarioFinished);

/**
 * Runs one level-local educational scenario. Level travel and cross-level progress belong to ExperienceSubsystem.
 * Presentation systems consume request delegates and report results back by logical ID.
 */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScenarioManagerComponent();

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool StartScenario(UScenarioDefinition* Scenario = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	void EndScenario();

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool StartScene(FName SceneID);

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool CompleteScene();

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool MoveToNextScene();

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool StartInteraction(FName InteractionID);

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool CompleteInteraction(FName InteractionID);

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool FailInteraction(FName InteractionID);

	/** Actor/system report. The current interaction must match Type and any non-None TargetID. */
	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool ReportInteractionResult(FName TargetID, EScenarioInteractionType InteractionType, bool bSuccess = true);

	/** Non-mutating check used before gameplay state changes. */
	UFUNCTION(BlueprintPure, Category = "Scenario")
	bool CanReportInteractionResult(FName TargetID, EScenarioInteractionType InteractionType) const;

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool RestartScenario();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool RestartScene();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool RestartInteraction();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool SkipCurrentInteraction();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool CompleteCurrentInteraction();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool GoToScene(FName SceneID);

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool GoToInteraction(FName InteractionID);

	/** Restores a linear authored Scene at an interaction and marks earlier entries complete. */
	UFUNCTION(BlueprintCallable, Category = "Scenario|Progress")
	bool RestoreProgressAtInteraction(FName SceneID, FName InteractionID);

	UFUNCTION(BlueprintPure, Category = "Scenario")
	FScenarioInteraction GetCurrentInteraction() const;

	UFUNCTION(BlueprintPure, Category = "Scenario")
	FScenarioDebugSnapshot GetDebugSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	void PrintDebugState() const;

	UFUNCTION(BlueprintPure, Category = "Scenario")
	EScenarioInteractionState GetInteractionState(FName InteractionID) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioDefinition> ScenarioDefinition;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioStateChanged OnScenarioStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioSceneChanged OnSceneStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioInteractionChanged OnInteractionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioInteractionRequested OnInteractionRequested;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioNarrationRequested OnNarrationRequested;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioObjectiveRequested OnObjectiveRequested;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioValidationFailed OnValidationFailed;

	UPROPERTY(BlueprintAssignable, Category = "Scenario|Events")
	FOnScenarioFinished OnScenarioFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetScenarioState(EScenarioState NewState);
	void SetSceneState(EScenarioSceneState NewState);
	void SetInteractionState(FName InteractionID, EScenarioInteractionState NewState);
	void BeginPendingInteraction();
	void AdvanceAfterInteraction();
	void HandleWaitCompleted();
	void FailScenario(const FString& Reason);
	bool AreRequiredInteractionsComplete() const;
	void ClearTimers();
	const FScenarioInteraction* FindCurrentInteraction() const;

	UPROPERTY(Transient)
	TObjectPtr<UScenarioSceneData> CurrentScene;

	FName CurrentSceneID;
	FName CurrentInteractionID;
	FName PendingInteractionID;
	FName PendingNextInteractionID;
	TMap<FName, EScenarioInteractionState> InteractionStates;
	EScenarioState ScenarioState = EScenarioState::Inactive;
	EScenarioSceneState SceneState = EScenarioSceneState::Inactive;
	FTimerHandle StartDelayTimer;
	FTimerHandle CompletionDelayTimer;
	FTimerHandle WaitTimer;
};
