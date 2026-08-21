#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactionComponent.generated.h"

UENUM(BlueprintType)
enum class ECombatFaction : uint8
{
	Neutral,
	Player,
	Ally,
	Enemy
};

/** Minimal reusable faction identity shared by all experiences. */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UFactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactionComponent();

	UFUNCTION(BlueprintPure, Category = "Combat|Faction")
	bool IsHostileTo(const UFactionComponent* Other) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Faction")
	ECombatFaction Faction = ECombatFaction::Neutral;
};
