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

	/** Drops an impact point down onto the terrain. Returns the impact point when no floor is found. */
	FVector FindGroundedEffectLocation(const FVector& ImpactLocation, const AActor* HitActor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Explosion", meta=(ClampMin="0.0"))
	float ExplosionRadius = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Chongtong|Explosion", meta=(ClampMin="0.0"))
	float AreaDamage = 80.0f;

	/**
	 * Plays the burst on the ground under the impact instead of wherever the shell touched.
	 * A shell that clips a soldier detonates at head height, which reads as the soldier exploding.
	 * Damage is unaffected: it is still resolved at the true impact point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Explosion")
	bool bGroundExplosionEffect = true;

	/** How far below the impact the floor may be before the effect stays at the impact point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Explosion", meta=(ClampMin="0.0"))
	float GroundTraceDistance = 600.0f;

	/** Lifts the burst slightly off the floor so it is not half-buried in the terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Explosion", meta=(ClampMin="0.0"))
	float GroundEffectHeightOffset = 15.0f;
	/** Impact Cascade particle system; replace it in BP_ChongtongProjectile Class Defaults. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Feedback")
	TObjectPtr<class UParticleSystem> ExplosionEffect;
	/**
	 * Per-effect scale exposed for projectile Blueprint tuning.
	 * The stock P_Explosion is authored for a grenade-sized burst, so 1.0 is small for a shell.
	 * Scaling the system also scales its point light radius, so keep this modest and fix the
	 * asset rather than pushing the number up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Chongtong|Feedback")
	FVector ExplosionEffectScale = FVector(2.0f);
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
