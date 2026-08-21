#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperienceTravelTriggerActor.generated.h"

class UBoxComponent;
class UExperienceDefinition;

/** Generic event source that stores a Scenario return checkpoint before Experience travel. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AExperienceTravelTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	AExperienceTravelTriggerActor();
	virtual void Tick(float DeltaSeconds) override;

	/** Can be called by any Blueprint event; overlap is only the ready-to-play example. */
	UFUNCTION(BlueprintCallable, Category = "Experience")
	bool TriggerExperienceTravel(AActor* TriggeringActor);

	/** True when no guard is configured or the active Scenario is at RequiredInteractionID. */
	UFUNCTION(BlueprintPure, Category = "Experience")
	bool IsInteractionRequirementMet() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	TObjectPtr<UExperienceDefinition> DestinationExperience;

	/** Optional guard used when multiple Experience triggers share one Level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience")
	FName RequiredInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Return Checkpoint")
	FName ReturnScenarioID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Return Checkpoint", meta = (DisplayName = "Return Stage ID"))
	FName ReturnSceneID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Experience|Return Checkpoint")
	FName ReturnInteractionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	bool bTriggerOnPawnOverlap = true;

	/** Supports VR Pawns whose HMD/root has no collision primitive. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	bool bTriggerOnPlayerViewLocation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	FVector TriggerExtent = FVector(100.0, 100.0, 120.0);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool bHasTriggered = false;
	bool bPlayerViewWasInsideTrigger = false;
};
