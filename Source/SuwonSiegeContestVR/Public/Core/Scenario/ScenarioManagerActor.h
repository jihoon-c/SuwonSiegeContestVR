#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScenarioManagerActor.generated.h"

class USceneComponent;
class UDataTable;
class UScenarioDefinition;
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

	/** Scenario-level asset. A ScenarioSceneData asset cannot be assigned here directly. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Configuration")
	TObjectPtr<UScenarioDefinition> ScenarioDefinition;

	/** Optional bridge table. Row names must match Narration interaction NarrationID values. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Configuration")
	TObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scenario|Configuration")
	bool bAutoStartScenario = false;

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

private:
	void ApplyConfiguration();
};
