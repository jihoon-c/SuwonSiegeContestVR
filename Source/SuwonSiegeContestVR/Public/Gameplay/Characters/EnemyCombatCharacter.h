#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CombatCharacter.h"
#include "Gameplay/Pooling/PoolableActorInterface.h"
#include "EnemyCombatCharacter.generated.h"

class UCombatAttackComponent;
class UAnimSequenceBase;
class UHealthComponent;
class AEnemyCombatCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeathPresentationFinished, AEnemyCombatCharacter*, Enemy);

UCLASS(Abstract, Blueprintable)
class SUWONSIEGECONTESTVR_API AEnemyCombatCharacter : public ACombatCharacter, public IPoolableActorInterface
{
	GENERATED_BODY()

public:
	AEnemyCombatCharacter();
	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatAttackComponent* GetAttackComponent() const { return AttackComponent; }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetObjectiveTarget(AActor* NewObjectiveTarget);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetRetreatTargetLocation(FVector RetreatLocation);

	/** Fired after the death animation has been visible for DeathRemovalDelay seconds. */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Death")
	FOnEnemyDeathPresentationFinished OnDeathPresentationFinished;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatAttackComponent> AttackComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<AActor> ObjectiveTarget;

	/** Assign the final death sequence in BP_EnemySword / BP_EnemyArcher. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Death")
	TObjectPtr<UAnimSequenceBase> DeathAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Death", meta = (ClampMin = "0.0"))
	float DeathRemovalDelay = 5.0f;

	/** AI movement owns locomotion; ignore animation root translation to prevent run-forward/snap-back. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	bool bIgnoreLocomotionRootMotion = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Death")
	bool bDeathPresentationActive = false;

	UFUNCTION()
	void HandleHealthDeath(UHealthComponent* DeadHealthComponent, const FCombatDamageSpec& KillingDamage);
	void FinishDeathPresentation();

	FTimerHandle DeathPresentationTimerHandle;
};
