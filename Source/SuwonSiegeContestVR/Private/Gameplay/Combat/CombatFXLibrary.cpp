#include "Gameplay/Combat/CombatFXLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"

UNiagaraComponent* UCombatFXLibrary::SpawnPooledSystemAtLocation(
	const UObject* WorldContextObject,
	UNiagaraSystem* SystemTemplate,
	const FVector Location,
	const FRotator Rotation,
	const FVector Scale,
	const bool bPreCullCheck)
{
	if (!WorldContextObject || !SystemTemplate)
	{
		return nullptr;
	}

	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		WorldContextObject,
		SystemTemplate,
		Location,
		Rotation,
		Scale,
		/*bAutoDestroy=*/true,
		/*bAutoActivate=*/true,
		ENCPoolMethod::AutoRelease,
		bPreCullCheck);
}

UParticleSystemComponent* UCombatFXLibrary::SpawnPooledEmitterAtLocation(
	const UObject* WorldContextObject,
	UParticleSystem* EmitterTemplate,
	const FVector Location,
	const FRotator Rotation,
	const FVector Scale)
{
	if (!WorldContextObject || !EmitterTemplate)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnEmitterAtLocation(
		WorldContextObject,
		EmitterTemplate,
		Location,
		Rotation,
		Scale,
		/*bAutoDestroy=*/true,
		EPSCPoolMethod::AutoRelease);
}

void UCombatFXLibrary::PlayPooledSoundAtLocation(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	const FVector Location,
	const float VolumeMultiplier,
	const float PitchMultiplier,
	USoundConcurrency* Concurrency,
	USoundAttenuation* Attenuation)
{
	if (!WorldContextObject || !Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		WorldContextObject,
		Sound,
		Location,
		FRotator::ZeroRotator,
		VolumeMultiplier,
		PitchMultiplier,
		/*StartTime=*/0.0f,
		/*AttenuationSettings=*/Attenuation,
		Concurrency);
}
