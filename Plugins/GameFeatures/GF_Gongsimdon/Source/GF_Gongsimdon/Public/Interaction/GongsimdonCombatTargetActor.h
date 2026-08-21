#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GongsimdonCombatTargetActor.generated.h"

class UBoxComponent;
class UScenarioInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGongsimdonCombatHit, int32, CurrentHits, int32, RequiredHits);

/** Invisible/replaceable combat volume that completes after the configured number of valid hits. */
UCLASS(Blueprintable)
class GF_GONGSIMDON_API AGongsimdonCombatTargetActor : public AActor
{
	GENERATED_BODY()

public:
	AGongsimdonCombatTargetActor();
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Combat")
	bool RegisterHit(AActor* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Gongsimdon|Combat")
	void SetCombatArmed(bool bArmed);

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Combat")
	FName GetTargetID() const { return TargetID; }

	UFUNCTION(BlueprintPure, Category = "Gongsimdon|Combat")
	int32 GetCurrentHits() const { return CurrentHits; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Combat")
	FName TargetID = TEXT("COMBAT_RETREATING");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Combat", meta = (ClampMin = "1"))
	int32 RequiredHits = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gongsimdon|Combat")
	FVector TargetExtent = FVector(150.0, 150.0, 120.0);

	UPROPERTY(BlueprintAssignable, Category = "Gongsimdon|Combat")
	FOnGongsimdonCombatHit OnCombatHit;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Combat")
	TObjectPtr<UBoxComponent> TargetVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gongsimdon|Combat")
	TObjectPtr<UScenarioInteractableComponent> ScenarioInteraction;

private:
	int32 CurrentHits = 0;
	bool bCombatArmed = false;
};
