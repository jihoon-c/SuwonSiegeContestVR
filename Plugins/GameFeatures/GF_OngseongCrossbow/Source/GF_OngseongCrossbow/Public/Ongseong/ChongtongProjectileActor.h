#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "ChongtongProjectileActor.generated.h"

/** Feature projectile used by a Chongtong. A Blueprint child supplies mesh, VFX, and sound. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AChongtongProjectileActor : public AGameplayProjectileActor
{
	GENERATED_BODY()

public:
	AChongtongProjectileActor();
};
