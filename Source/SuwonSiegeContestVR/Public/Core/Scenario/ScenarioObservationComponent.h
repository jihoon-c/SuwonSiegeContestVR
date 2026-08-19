#pragma once

#include "CoreMinimal.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"
#include "ScenarioObservationComponent.generated.h"

class USceneComponent;

/** HMD/player-camera gaze check. Tick is enabled only while an Observe interaction is active. */
UCLASS(ClassGroup = (Scenario), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UScenarioObservationComponent : public UScenarioInteractableComponent
{
	GENERATED_BODY()

public:
	UScenarioObservationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Scenario|Observation")
	void StartObservation();

	UFUNCTION(BlueprintCallable, Category = "Scenario|Observation")
	void StopObservation(bool bReportFailure = false);

	UFUNCTION(BlueprintPure, Category = "Scenario|Observation")
	float GetObservationProgress() const;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Scenario|Observation")
	TObjectPtr<USceneComponent> ObservationTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Observation", meta = (ClampMin = "0.0"))
	float RequiredViewTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Observation", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float RequiredViewAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Observation", meta = (ClampMin = "0.0"))
	float MaxDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Observation")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario|Observation")
	TEnumAsByte<ECollisionChannel> LineOfSightChannel = ECC_Visibility;

private:
	bool IsTargetObserved() const;
	FVector GetTargetLocation() const;

	float AccumulatedViewTime = 0.0f;
	bool bObservationActive = false;
};
