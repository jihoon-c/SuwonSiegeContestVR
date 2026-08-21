#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "DamageReceiverInterface.generated.h"

UINTERFACE(BlueprintType)
class SUWONSIEGECONTESTVR_API UDamageReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/** Optional Actor-level damage contract for targets that do not use UHealthComponent. */
class SUWONSIEGECONTESTVR_API IDamageReceiverInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat|Damage")
	bool ReceiveCombatDamage(const FCombatDamageSpec& DamageSpec);
};
