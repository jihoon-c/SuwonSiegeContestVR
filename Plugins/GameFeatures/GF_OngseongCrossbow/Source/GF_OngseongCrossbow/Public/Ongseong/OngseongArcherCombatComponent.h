#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OngseongArcherCombatComponent.generated.h"

class AActorPool;
class AEnemyCombatCharacter;
class AGameplayProjectileActor;
class UAnimMontage;
class UAnimSequenceBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOngseongArcherShot, AActor*, Target, AGameplayProjectileActor*, Projectile, bool, bIntendedHit);

/** Feature-only ranged behavior used by pooled Ongseong archers. */
UCLASS(ClassGroup=(Ongseong), meta=(BlueprintSpawnableComponent))
class GF_ONGSEONGCROSSBOW_API UOngseongArcherCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOngseongArcherCombatComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category="Ongseong|Archer")
	/** The archer chooses the closest viable target from the allied cannon and player. */
	void ConfigureCombat(AActor* NewCannonTarget, AActor* NewPlayerTarget, AActorPool* NewProjectilePool);
	UFUNCTION(BlueprintCallable, Category="Ongseong|Archer")
	void ActivateCombat();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Archer")
	void DeactivateCombat();
	UFUNCTION(BlueprintCallable, Category="Ongseong|Archer")
	bool TryFireArrow();
	UFUNCTION(BlueprintPure, Category="Ongseong|Archer")
	AActor* GetCurrentTarget() const;
	/** The allied emplacement whose attack slot this archer holds, or null while escorting. */
	UFUNCTION(BlueprintPure, Category="Ongseong|Archer")
	AActor* GetReservedCannon() const { return PrimaryTarget; }
	UFUNCTION(BlueprintPure, Category="Ongseong|Archer")
	bool IsInFiringPosition() const;
	UFUNCTION(BlueprintPure, Category="Ongseong|Archer")
	float GetEngagementRange() const { return EngagementRange; }

	void ApplyTuning(float InHitChance, float InRange, float InInterval, float InMissRadius, float InDamage, float InProjectileSpeed);

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Archer")
	FOnOngseongArcherShot OnArrowFired;

protected:
	bool IsUsableTarget(const AActor* Target) const;
	FVector BuildAimPoint(AActor* Target, bool bIntendedHit);
	AGameplayProjectileActor* SpawnArrow(const FVector& SpawnLocation, const FVector& Direction);
	void FinishAttackAnimation();
	void PerformScheduledShot();
	void BeginAttackAnimation();
	void FirePendingArrow();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer")
	TSubclassOf<AGameplayProjectileActor> ArrowClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer")
	TObjectPtr<AActorPool> ProjectilePool;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Archer")
	TObjectPtr<AActor> PrimaryTarget;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Ongseong|Archer")
	TObjectPtr<AActor> FallbackTarget;
	/** Sequence played by code through the AnimBP's named slot before the arrow is launched. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Archer|Animation")
	TObjectPtr<UAnimSequenceBase> AttackAnimation;
	/** Add a Slot node with this name to ABP_EnemyArcher's AnimGraph. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Archer|Animation")
	FName AttackAnimationSlot = TEXT("DefaultSlot");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Archer|Animation", meta=(ClampMin="0.0"))
	float AttackBlendInTime = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ongseong|Archer|Animation", meta=(ClampMin="0.0"))
	float AttackBlendOutTime = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HitChance = 0.65f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="100.0"))
	float EngagementRange = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="0.1"))
	float FireInterval = 2.75f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="0.0"))
	float MissRadius = 275.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="0.0"))
	float ArrowDamage = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="1.0"))
	float ArrowSpeed = 6500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="0.0"))
	float SpawnHeight = 140.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ongseong|Archer", meta=(ClampMin="0.0"))
	float FallbackAttackAnimationDuration = 0.8f;

	bool bCombatActive = false;
	bool bAttackInProgress = false;
	bool bPendingIntendedHit = false;
	FRandomStream RandomStream;
	FTimerHandle AttackAnimationTimerHandle;
	FTimerHandle FireTimerHandle;
};
