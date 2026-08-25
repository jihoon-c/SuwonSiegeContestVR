#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScenarioManagerActor.generated.h"

class USceneComponent;
class UDataTable;
class UExperienceDefinition;
class UScenarioDefinition;
class UScenarioExperienceBridgeComponent;
class UScenarioInteractionGuideComponent;
class UScenarioManagerComponent;
class UScenarioNarrationBridgeComponent;

/** Ready-to-place Core manager. Feature Blueprints may subclass this actor for presentation bindings. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AScenarioManagerActor : public AActor
{
	GENERATED_BODY()

public:
	AScenarioManagerActor();

	/** Starts the Scenario Definition assigned on this actor. */
	UFUNCTION(BlueprintCallable, Category = "Scenario")
	bool StartConfiguredScenario();

	UFUNCTION(BlueprintCallable, Category = "Scenario")
	void SetScenarioDefinition(UScenarioDefinition* NewScenarioDefinition);

	/** Re-resolves Scenario, Narration, and policies from the assigned Experience. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Scenario|Configuration")
	void RefreshResolvedConfiguration();

	UFUNCTION(BlueprintPure, Category = "Scenario")
	UScenarioManagerComponent* GetScenarioManager() const { return ScenarioManager; }

	UFUNCTION(BlueprintPure, Category = "Scenario|Narration")
	UScenarioNarrationBridgeComponent* GetNarrationBridge() const { return NarrationBridge; }

	UFUNCTION(BlueprintPure, Category = "Scenario|Experience")
	UScenarioExperienceBridgeComponent* GetExperienceBridge() const { return ExperienceBridge; }

	UFUNCTION(BlueprintPure, Category = "Scenario|Guide")
	UScenarioInteractionGuideComponent* GetInteractionGuide() const { return InteractionGuide; }

	/** Development-only shortcut used by the Spacebar debug binding. */
	UFUNCTION(BlueprintCallable, Category = "Scenario|Debug")
	bool DebugAdvanceCurrentInteraction();

	/** Resolved automatically from ExperienceDefinition. Do not configure twice. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resolved Configuration")
	TObjectPtr<UScenarioDefinition> ScenarioDefinition;

	/** Resolved from LevelNarrationTable when assigned, otherwise from ScenarioDefinition. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resolved Configuration")
	TObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resolved Configuration")
	bool bAutoStartScenario = false;

	/** The only required Level authoring assignment for an Experience. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Configuration")
	TObjectPtr<UExperienceDefinition> ExperienceDefinition;

	/** Optional per-Level override. Leave empty to use ScenarioDefinition.NarrationTable. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Narration", meta = (DisplayName = "Level Narration Table"))
	TObjectPtr<UDataTable> LevelNarrationTable;

	/** Only for a level-local Scenario that deliberately has no Experience. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Advanced", meta = (AdvancedDisplay))
	TObjectPtr<UScenarioDefinition> StandaloneScenarioDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Configuration")
	bool bActivateExperienceWhenOpenedDirectly = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resolved Configuration")
	bool bCompleteExperienceOnScenarioFinished = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Configuration")
	bool bRestoreScenarioCheckpoint = true;

	/** In non-Shipping builds, Space completes the running interaction through its success path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Debug", meta = (AdvancedDisplay))
	bool bEnableSpacebarDebugAdvance = true;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	/** Feature-level managers may temporarily defer the configured scenario (for example, until a level intro finishes). */
	virtual bool ShouldAutoStartScenario() const { return true; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioNarrationBridgeComponent> NarrationBridge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioExperienceBridgeComponent> ExperienceBridge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioInteractionGuideComponent> InteractionGuide;

private:
	void ApplyConfiguration();
	void SetupDebugInput();
	void HandleSpacebarDebugAdvance();
	bool bDebugInputBound = false;
};
