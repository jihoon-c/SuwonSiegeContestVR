#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GongsimdonReportActor.generated.h"

class USceneComponent;
class UScenarioInteractableComponent;

UENUM(BlueprintType)
enum class EGongsimdonReportDirection : uint8
{
	North,
	East,
	South,
	West
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGongsimdonReportEvaluated, bool, bAccepted);

/** Validates the player's enemy direction/count report and completes a Custom interaction. */
UCLASS(Blueprintable)
class GF_GONGSIMDON_API AGongsimdonReportActor : public AActor
{
	GENERATED_BODY()

public:
	AGongsimdonReportActor();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Report")
	bool SubmitReport(EGongsimdonReportDirection Direction, int32 EnemyCount);

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Report")
	void SetReportArmed(bool bArmed);

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Report")
	FName GetTargetID() const { return TargetID; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Report")
	FName TargetID = TEXT("REPORT_ENEMY");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Report")
	EGongsimdonReportDirection ExpectedDirection = EGongsimdonReportDirection::East;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Report", meta = (ClampMin = "0"))
	int32 MinimumEnemyCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Report", meta = (ClampMin = "0"))
	int32 MaximumEnemyCount = 8;

	UPROPERTY(BlueprintAssignable, Category = "Gongsimdon|Report")
	FOnGongsimdonReportEvaluated OnReportEvaluated;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Report")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Report")
	TObjectPtr<UScenarioInteractableComponent> ScenarioInteraction;

private:
	bool bReportArmed = false;
};
