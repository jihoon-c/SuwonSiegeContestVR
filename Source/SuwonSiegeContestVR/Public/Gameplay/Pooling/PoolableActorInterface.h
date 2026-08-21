#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActorInterface.generated.h"

UINTERFACE(BlueprintType)
class SUWONSIEGECONTESTVR_API UPoolableActorInterface : public UInterface
{
	GENERATED_BODY()
};

/** Lets an Actor reset timers, movement, and visual state whenever a shared pool reuses it. */
class SUWONSIEGECONTESTVR_API IPoolableActorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay|Pooling")
	void OnAcquiredFromPool();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gameplay|Pooling")
	void OnReleasedToPool();
};
