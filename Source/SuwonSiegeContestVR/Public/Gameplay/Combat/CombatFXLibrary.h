#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatFXLibrary.generated.h"

class UNiagaraComponent;
class UParticleSystem;
class UParticleSystemComponent;
class UNiagaraSystem;
class USoundBase;
class USoundConcurrency;
class USoundAttenuation;

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
	/**
	 * One-shot effect only. Looping systems never return to the pool and must not use this.
	 * bPreCullCheck lets the system's own cull settings drop the spawn entirely; pass false for
	 * gameplay-critical feedback such as an impact the player is being asked to watch for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Rotation,Scale,bPreCullCheck"))
	static UNiagaraComponent* SpawnPooledSystemAtLocation(
		const UObject* WorldContextObject,
		UNiagaraSystem* SystemTemplate,
		FVector Location,
		FRotator Rotation = FRotator::ZeroRotator,
		FVector Scale = FVector(1.0f),
		bool bPreCullCheck = true);

	/**
	 * Cascade equivalent of SpawnPooledSystemAtLocation for the StarterContent explosion/spark
	 * systems used by the cannon feedback. One-shot only; the component returns to the PSC pool.
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Rotation,Scale"))
	static UParticleSystemComponent* SpawnPooledEmitterAtLocation(
		const UObject* WorldContextObject,
		UParticleSystem* EmitterTemplate,
		FVector Location,
		FRotator Rotation = FRotator::ZeroRotator,
		FVector Scale = FVector(1.0f));

	/** Fire-and-forget sound with an optional concurrency asset that caps simultaneous combat noise. */
	UFUNCTION(BlueprintCallable, Category = "Combat|FX", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "VolumeMultiplier,PitchMultiplier,Concurrency,Attenuation"))
	static void PlayPooledSoundAtLocation(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		FVector Location,
		float VolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f,
		USoundConcurrency* Concurrency = nullptr,
		USoundAttenuation* Attenuation = nullptr);
};
