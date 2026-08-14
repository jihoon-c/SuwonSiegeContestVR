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

	UFUNCTION(BlueprintPure, Category = "Scenario")
	UScenarioManagerComponent* GetScenarioManager() const { return ScenarioManager; }

	UFUNCTION(BlueprintPure, Category = "Scenario|Narration")
	UScenarioNarrationBridgeComponent* GetNarrationBridge() const { return NarrationBridge; }

	UFUNCTION(BlueprintPure, Category = "Scenario|Experience")
	UScenarioExperienceBridgeComponent* GetExperienceBridge() const { return ExperienceBridge; }

	/** Scenario-level asset. A ScenarioSceneData asset cannot be assigned here directly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Configuration")
	TObjectPtr<UScenarioDefinition> ScenarioDefinition;

	/** Optional bridge table. Row names must match Narration interaction NarrationID values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Configuration")
	TObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Configuration")
	bool bAutoStartScenario = false;

	/** Optional cross-level definition. Leave empty for scenarios that are not standalone experiences. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Configuration")
	TObjectPtr<UExperienceDefinition> ExperienceDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Configuration")
	bool bActivateExperienceWhenOpenedDirectly = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Configuration")
	bool bCompleteExperienceOnScenarioFinished = true;

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
