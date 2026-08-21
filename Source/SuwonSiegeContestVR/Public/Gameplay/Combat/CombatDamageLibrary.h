#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "CombatDamageLibrary.generated.h"

class UHealthComponent;

/** Single entry point for faction-aware damage application. */
UCLASS()
class SUWONSIEGECONTESTVR_API UCombatDamageLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
	static bool ApplyCombatDamage(AActor* Target, const FCombatDamageSpec& DamageSpec);

	UFUNCTION(BlueprintPure, Category = "Combat|Damage")
	static bool CanDamageTarget(const AActor* Target, const FCombatDamageSpec& DamageSpec);

	UFUNCTION(BlueprintPure, Category = "Combat|Damage")
	static UHealthComponent* FindHealthComponent(const AActor* Target);
};
