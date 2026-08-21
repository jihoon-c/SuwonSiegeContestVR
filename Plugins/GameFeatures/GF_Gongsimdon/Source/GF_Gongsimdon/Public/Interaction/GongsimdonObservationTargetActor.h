#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GongsimdonObservationTargetActor.generated.h"

class UArrowComponent;
class USceneComponent;
class UScenarioObservationComponent;

/** Editor-placeable gaze target used by the Gongsimdon action scenario. */
UCLASS(Blueprintable)
class GF_GONGSIMDON_API AGongsimdonObservationTargetActor : public AActor
{
	GENERATED_BODY()

public:
	AGongsimdonObservationTargetActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Observation")
	void ActivateObservation();

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Observation")
	void DeactivateObservation(bool bReportFailure = false);

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Observation")
	FName GetTargetID() const { return TargetID; }

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Observation")
	UScenarioObservationComponent* GetObservationComponent() const { return Observation; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Observation")
	FName TargetID = TEXT("OBS_TARGET");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Observation", meta = (ClampMin = "0.0"))
	float RequiredViewTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Observation", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float RequiredViewAngle = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Observation", meta = (ClampMin = "0.0"))
	float MaxDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Observation")
	bool bRequireLineOfSight = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Observation")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Observation")
	TObjectPtr<UArrowComponent> EditorMarker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Observation")
	TObjectPtr<UScenarioObservationComponent> Observation;
};
