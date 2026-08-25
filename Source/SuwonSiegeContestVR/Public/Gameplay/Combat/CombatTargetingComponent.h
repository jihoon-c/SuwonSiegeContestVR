#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatTargetingComponent.generated.h"

/** Finds living hostile Actors without coupling an attacker to concrete enemy classes. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UCombatTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatTargetingComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat|Targeting")
	TArray<AActor*> FindHostileTargets(float SearchRadius) const;

	UFUNCTION(BlueprintCallable, Category = "Combat|Targeting")
	TArray<AActor*> FindVisibleHostileTargets(float SearchRadius) const;

	UFUNCTION(BlueprintPure, Category = "Combat|Targeting")
	AActor* SelectClosestTo(const TArray<AActor*>& Candidates, const AActor* ReferenceActor) const;

	UFUNCTION(BlueprintPure, Category = "Combat|Targeting")
	AActor* SelectRandom(const TArray<AActor*>& Candidates) const;

	UFUNCTION(BlueprintPure, Category = "Combat|Targeting")
	bool IsValidHostileTarget(const AActor* Candidate) const;

	UFUNCTION(BlueprintPure, Category = "Combat|Targeting")
	bool HasLineOfSightTo(const AActor* Candidate) const;
};
