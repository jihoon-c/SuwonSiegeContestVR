#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "CombatThreatComponent.generated.h"

class UHealthComponent;

/** Records recent attackers of an Actor so defensive allies can prioritize them. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UCombatThreatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatThreatComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
	void RegisterAttacker(AActor* Attacker);

	UFUNCTION(BlueprintPure, Category = "Combat|Threat")
	TArray<AActor*> GetActiveAttackers();

protected:
	UFUNCTION()
	void HandleOwnerDamaged(UHealthComponent* DamagedHealthComponent, const FCombatDamageSpec& DamageSpec);

	void PruneExpiredAttackers();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Threat", meta = (ClampMin = "0.0"))
	float AttackerMemorySeconds = 8.0f;

	UPROPERTY(Transient)
	TMap<TObjectPtr<AActor>, float> RecentAttackers;
};
