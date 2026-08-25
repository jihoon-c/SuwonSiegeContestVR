#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatFXLibrary.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class USoundConcurrency;

/**
 * Shared spawn helpers for one-shot combat feedback.
 * Effects go through the engine Niagara component pool so repeated impacts do not allocate
 * a new component per hit on standalone VR hardware.
 */
UCLASS()
class SUWONSIEGECONTESTVR_API UCombatFXLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** One-shot effect only. Looping systems never return to the pool and must not use this. */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Rotation,Scale"))
	static UNiagaraComponent* SpawnPooledSystemAtLocation(
		const UObject* WorldContextObject,
		UNiagaraSystem* SystemTemplate,
		FVector Location,
		FRotator Rotation = FRotator::ZeroRotator,
		FVector Scale = FVector(1.0f));

	/** Fire-and-forget sound with an optional concurrency asset that caps simultaneous combat noise. */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "VolumeMultiplier,PitchMultiplier,Concurrency"))
	static void PlayPooledSoundAtLocation(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		FVector Location,
		float VolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f,
		USoundConcurrency* Concurrency = nullptr);
};
