#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CombatCharacter.h"
#include "AllyCombatCharacter.generated.h"

UCLASS(Abstract, Blueprintable)
class SUWONSIEGECONTESTVR_API AAllyCombatCharacter : public ACombatCharacter
{
	GENERATED_BODY()

public:
	AAllyCombatCharacter();
};
