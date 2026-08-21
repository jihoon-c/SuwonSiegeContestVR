#pragma once

#include "CoreMinimal.h"
#include "EnemyAITypes.generated.h"

/** Runtime fidelity level selected by UEnemyAILODComponent. */
UENUM(BlueprintType)
enum class EEnemyAILODLevel : uint8
{
	Far,
	Near
};
