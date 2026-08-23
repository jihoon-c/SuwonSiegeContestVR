#include "Gameplay/Combat/CombatFXLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

UNiagaraComponent* UCombatFXLibrary::SpawnPooledSystemAtLocation(
	const UObject* WorldContextObject,
	UNiagaraSystem* SystemTemplate,
	const FVector Location,
	const FRotator Rotation,
	const FVector Scale)
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
		/*bPreCullCheck=*/true);
}

void UCombatFXLibrary::PlayPooledSoundAtLocation(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	const FVector Location,
	const float VolumeMultiplier,
	const float PitchMultiplier,
	USoundConcurrency* Concurrency)
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
		/*AttenuationSettings=*/nullptr,
		Concurrency);
}
