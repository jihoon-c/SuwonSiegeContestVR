#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "CombatFactionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatFactionChanged, ECombatFaction, PreviousFaction, ECombatFaction, NewFaction);

/** Stores an Actor's team and provides shared hostility rules. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UCombatFactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatFactionComponent();

	UFUNCTION(BlueprintPure, Category = "Combat|Faction")
	ECombatFaction GetFaction() const { return Faction; }

	UFUNCTION(BlueprintCallable, Category = "Combat|Faction")
	void SetFaction(ECombatFaction NewFaction);

	UFUNCTION(BlueprintPure, Category = "Combat|Faction")
	bool IsHostileTo(const AActor* OtherActor) const;

	UFUNCTION(BlueprintPure, Category = "Combat|Faction")
	static bool AreHostile(ECombatFaction FirstFaction, ECombatFaction SecondFaction);

	UPROPERTY(BlueprintAssignable, Category = "Combat|Faction")
	FOnCombatFactionChanged OnFactionChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Faction")
	ECombatFaction Faction = ECombatFaction::Neutral;
};
