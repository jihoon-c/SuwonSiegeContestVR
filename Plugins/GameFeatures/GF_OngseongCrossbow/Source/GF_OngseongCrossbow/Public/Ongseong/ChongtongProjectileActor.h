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
	virtual void BeginPlay() override;

protected:
	UFUNCTION()
	void HandleExplosion(AGameplayProjectileActor* Projectile, AActor* HitActor, FHitResult Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Explosion", meta=(ClampMin="0.0"))
	float ExplosionRadius = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Explosion", meta=(ClampMin="0.0"))
	float AreaDamage = 80.0f;
	/** Impact Niagara system; replace it in BP_ChongtongProjectile Class Defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<class UNiagaraSystem> ExplosionEffect;
	/** Per-effect scale exposed for projectile Blueprint tuning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Feedback")
	FVector ExplosionEffectScale = FVector(1.0f);
	/** Impact sound/cue; replace it in BP_ChongtongProjectile Class Defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<class USoundBase> ExplosionSound;
	/** Default 3D attenuation for the explosion cue. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<class USoundAttenuation> ExplosionSoundAttenuation;

	/** Caps how many explosions can be audible at once on standalone hardware. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chongtong|Projectile")
	TObjectPtr<class USoundConcurrency> ExplosionSoundConcurrency;
};
