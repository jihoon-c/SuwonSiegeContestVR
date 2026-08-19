#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChongtongCannonActor.generated.h"

class AGameplayProjectileActor;
class UCombatFactionComponent;
class UHealthComponent;
class UCombatTargetingComponent;
class UCombatThreatComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChongtongFired, AActor*, Target, AGameplayProjectileActor*, Projectile);

/** Defensive fixed cannon. Selects hostile targets by attacker, gate proximity, then random fallback. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AChongtongCannonActor : public AActor
{
	GENERATED_BODY()

public:
	AChongtongCannonActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong")
	void SetGateTarget(AActor* NewGateTarget);

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong")
	bool TryFire();

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong")
	AActor* SelectTarget() const;

	UPROPERTY(BlueprintAssignable, Category = "Ongseong|Chongtong")
	FOnChongtongFired OnFired;

protected:
	void FireScheduledShot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Muzzle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatFactionComponent> FactionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatThreatComponent> ThreatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatTargetingComponent> TargetingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TObjectPtr<AActor> GateTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TSubclassOf<AGameplayProjectileActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.1"))
	float FireInterval = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float FireRange = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileDamage = 40.0f;

	FTimerHandle FireTimerHandle;
};
