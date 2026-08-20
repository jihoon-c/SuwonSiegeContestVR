#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "ChongtongCannonActor.generated.h"

class AGameplayProjectileActor;
class AActorPool;
class AAllyCombatCharacter;
class UCombatFactionComponent;
class UHealthComponent;
class UCombatTargetingComponent;
class UCombatThreatComponent;
class USceneComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChongtongFired, AActor*, Target, AGameplayProjectileActor*, Projectile);

/** Defensive fixed cannon. Selects hostile targets by attacker, gate proximity, then random fallback. */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AChongtongCannonActor : public AActor, public IDamageReceiverInterface
{
	GENERATED_BODY()

public:
	AChongtongCannonActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual bool ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec) override;

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong")
	void SetGateTarget(AActor* NewGateTarget);

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong")
	bool TryFire();

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong")
	AActor* SelectTarget() const;

	/** Spawns the configured allied operator and locks it to the cannon's seat. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Operator")
	bool SpawnMountedOperator();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Chongtong|Operator")
	void RemoveMountedOperator();

	UFUNCTION(BlueprintPure, Category = "Ongseong|Chongtong|Operator")
	AAllyCombatCharacter* GetMountedOperator() const { return MountedOperator; }

	UPROPERTY(BlueprintAssignable, Category = "Ongseong|Chongtong")
	FOnChongtongFired OnFired;

protected:
	void FireScheduledShot();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Hwacha carriage/support mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HwachaBaseMesh;

	/** Chongtong barrel mesh mounted on the carriage. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ChongtongMesh;

	/** Attachment point for the friendly AI gunner. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> OperatorSeat;

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

	/** Blueprint class for the allied gunner. Leave empty only when a level supplies one manually. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	TSubclassOf<AAllyCombatCharacter> OperatorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	bool bSpawnOperatorOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	FTransform OperatorRelativeTransform;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Chongtong|Operator")
	TObjectPtr<AAllyCombatCharacter> MountedOperator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TSubclassOf<AGameplayProjectileActor> ProjectileClass;

	/** Optional projectile pool. When unset, the cannon falls back to SpawnActor/Destroy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong")
	TObjectPtr<AActorPool> ProjectilePool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.1"))
	float FireInterval = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float FireRange = 12000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Chongtong", meta = (ClampMin = "0.0"))
	float ProjectileDamage = 40.0f;

	FTimerHandle FireTimerHandle;

	UFUNCTION()
	void HandleDeath(UHealthComponent* DeadHealthComponent, const FCombatDamageSpec& KillingDamage);
};
