#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Pooling/PoolableActorInterface.h"
#include "GameplayProjectileActor.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UPrimitiveComponent;
class AGameplayProjectileActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameplayProjectileImpact, AGameplayProjectileActor*, Projectile, AActor*, HitActor, FHitResult, Hit);

/** Collision-based projectile base with faction-aware damage. Feature projectiles provide their visual and launch policy. */
UCLASS(Abstract, Blueprintable)
class SUWONSIEGECONTESTVR_API AGameplayProjectileActor : public AActor, public IPoolableActorInterface
{
	GENERATED_BODY()

public:
	AGameplayProjectileActor();

	virtual void BeginPlay() override;
	virtual void LifeSpanExpired() override;
	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Combat|Projectile")
	void LaunchProjectile(FVector Direction, float Speed, const FCombatDamageSpec& InDamageSpec);

	UFUNCTION(BlueprintCallable, Category = "Combat|Projectile")
	void SetDamageSpec(const FCombatDamageSpec& InDamageSpec);

	UFUNCTION(BlueprintPure, Category = "Combat|Projectile")
	FCombatDamageSpec GetDamageSpec() const { return DamageSpec; }

	UPROPERTY(BlueprintAssignable, Category = "Combat|Projectile")
	FOnGameplayProjectileImpact OnProjectileImpact;

protected:
	UFUNCTION()
	void HandleProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Projectile")
	FCombatDamageSpec DamageSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Projectile", meta = (ClampMin = "0.0"))
	float LifeSpanSeconds = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Projectile")
	bool bDestroyOnImpact = true;
};
