#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NokroScenarioManager.generated.h"

class ANokroCraneActor;
class ANokroRepairTargetActor;
class UNokroNarrationComponent;
class UVRHUDComponent;

UENUM(BlueprintType)
enum class ENokroScenarioState : uint8
{
	Idle,
	Repairing,
	Completed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNokroRepairProgress, int32, RepairedCount, int32, TotalCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNokroPlacementResult, bool, bSucceeded);

/** Owns the repair targets, validation/reset loop, shared HUD/narration, and Experience completion. */
UCLASS(Blueprintable)
class GF_GEOJUNGGI_API ANokroScenarioManager : public AActor
{
	GENERATED_BODY()

public:
	ANokroScenarioManager();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Nokro|Scenario")
	bool StartScenario();

	UFUNCTION(BlueprintCallable, Category="Nokro|Scenario")
	void ResetScenario();

	UFUNCTION(BlueprintCallable, Category="Nokro|Scenario")
	bool TryPlaceStoneAtTransform(const FTransform& StoneTransform);

	UFUNCTION(BlueprintPure, Category="Nokro|Scenario")
	int32 GetRepairedCount() const;

	UFUNCTION(BlueprintPure, Category="Nokro|Scenario")
	int32 GetTotalTargetCount() const { return RepairTargets.Num(); }

	UFUNCTION(BlueprintPure, Category="Nokro|Scenario")
	ENokroScenarioState GetScenarioState() const { return ScenarioState; }

	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroRepairProgress OnRepairProgress;
	UPROPERTY(BlueprintAssignable, Category="Nokro|Events")
	FOnNokroPlacementResult OnPlacementResult;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Nokro|Setup")
	TObjectPtr<ANokroCraneActor> Crane;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Nokro|Setup")
	TArray<TObjectPtr<ANokroRepairTargetActor>> RepairTargets;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Setup")
	TSubclassOf<ANokroCraneActor> CraneClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Setup")
	TSubclassOf<ANokroRepairTargetActor> RepairTargetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Setup")
	bool bSpawnDefaultLayout = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Setup")
	TArray<FVector> DefaultTargetOffsets;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Scenario")
	bool bAutoStart = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Scenario")
	bool bCompleteExperienceOnFinish = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Nokro|Scenario", meta=(ClampMin="0.0"))
	float ExperienceCompletionDelay = 20.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nokro|Narration")
	TObjectPtr<UNokroNarrationComponent> Narration;

private:
	void DiscoverOrSpawnLayout();
	void BindCrane();
	void UpdateHUD();
	void CompleteScenario();
	void CompleteExperience();

	UFUNCTION()
	void HandlePlacementRequested(FTransform StoneTransform);
	UFUNCTION()
	void HandleHandleGrabbed();
	UFUNCTION()
	void HandleHeightAdjusted();
	UFUNCTION()
	void HandleDirectionAdjusted();

	UPROPERTY(Transient)
	TObjectPtr<UVRHUDComponent> VRHUD;
	UPROPERTY(VisibleInstanceOnly, Category="Nokro|Scenario")
	ENokroScenarioState ScenarioState = ENokroScenarioState::Idle;
	FTimerHandle ExperienceCompletionTimer;
};
