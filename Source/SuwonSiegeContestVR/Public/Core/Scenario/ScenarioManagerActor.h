#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScenarioManagerActor.generated.h"

class USceneComponent;
class UDataTable;
class UExperienceDefinition;
class UScenarioDefinition;
class UScenarioExperienceBridgeComponent;
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

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioNarrationBridgeComponent> NarrationBridge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scenario")
	TObjectPtr<UScenarioExperienceBridgeComponent> ExperienceBridge;

private:
	void ApplyConfiguration();
};
