#pragma once

#include "CoreMinimal.h"
#include "Core/Scenario/ScenarioTypes.h"
#include "GameFramework/Actor.h"
#include "GongsimdonScenarioDirectorActor.generated.h"

class USceneComponent;
class UScenarioManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGongsimdonActionRequested, FName, InteractionID, FName, TargetID);

/** Routes data-authored Gongsimdon actions to level actors without Level Blueprint logic. */
UCLASS(Blueprintable)
class GF_GONGSIMDON_API AGongsimdonScenarioDirectorActor : public AActor
{
	GENERATED_BODY()

public:
	AGongsimdonScenarioDirectorActor();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Scenario")
	bool InitializeDirector();

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Scenario")
	UScenarioManagerComponent* GetScenarioManager() const { return ScenarioManager; }

	UPROPERTY(BlueprintAssignable, Category = "Gongsimdon|Scenario")
	FOnGongsimdonActionRequested OnCueRequested;

	UPROPERTY(BlueprintAssignable, Category = "Gongsimdon|Scenario")
	FOnGongsimdonActionRequested OnActionRequested;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleInteractionRequested(FScenarioInteraction Interaction);

	UFUNCTION()
	void HandleInteractionStateChanged(
		FScenarioInteraction Interaction,
		EScenarioInteractionState State);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Scenario")
	TObjectPtr<USceneComponent> SceneRoot;

private:
	void DeactivateAllTargets();
	bool ActivateObservationTarget(FName TargetID);
	bool ArmReportTarget(FName TargetID);
	bool ArmCombatTarget(FName TargetID);

	UPROPERTY(Transient)
	TObjectPtr<UScenarioManagerComponent> ScenarioManager;
};
