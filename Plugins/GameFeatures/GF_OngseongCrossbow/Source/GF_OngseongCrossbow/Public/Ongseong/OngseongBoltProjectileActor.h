#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "OngseongBoltProjectileActor.generated.h"

class UStaticMeshComponent;

/** Lightweight physical arrow fired by the Ongseong enemy archers. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongBoltProjectileActor : public AGameplayProjectileActor
{
	GENERATED_BODY()

public:
	AOngseongBoltProjectileActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BoltMesh;
};
