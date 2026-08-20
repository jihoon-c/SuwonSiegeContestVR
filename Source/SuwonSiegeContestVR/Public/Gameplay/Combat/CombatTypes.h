#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

class AActor;

/** Shared allegiance groups. Add new entries only at the end to preserve serialized values. */
UENUM(BlueprintType)
enum class ECombatFaction : uint8
{
	Neutral,
	Player,
	Ally,
	Enemy
};

/** Context passed from an attack source to a damage receiver. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FCombatDamageSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage", meta = (ClampMin = "0.0"))
	float Amount = 0.0f;

	/** Actor that owns the attack, used for faction checks and credit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	/** Concrete projectile, weapon, trap, or other object that produced this hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	TObjectPtr<AActor> DamageCauser = nullptr;

	/** Use only for intentional environmental or scripted damage that bypasses faction protection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Damage")
	bool bIgnoreFaction = false;
};
