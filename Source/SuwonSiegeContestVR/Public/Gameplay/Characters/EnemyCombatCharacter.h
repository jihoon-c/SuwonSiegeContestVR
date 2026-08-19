#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CombatCharacter.h"
#include "Gameplay/Pooling/PoolableActorInterface.h"
#include "EnemyCombatCharacter.generated.h"

class UEnemyAILODComponent;
class UEnemyBehaviorStateComponent;
class UEnemySimpleMovementComponent;
class UCombatAttackComponent;

UCLASS(Abstract, Blueprintable)
class SUWONSIEGECONTESTVR_API AEnemyCombatCharacter : public ACombatCharacter, public IPoolableActorInterface
{
	GENERATED_BODY()

public:
	AEnemyCombatCharacter();
	virtual void BeginPlay() override;

	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI")
	UEnemyAILODComponent* GetAILODComponent() const { return AILODComponent; }

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI")
	UEnemySimpleMovementComponent* GetSimpleMovementComponent() const { return SimpleMovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Gameplay|AI")
	UEnemyBehaviorStateComponent* GetBehaviorStateComponent() const { return BehaviorStateComponent; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatAttackComponent* GetAttackComponent() const { return AttackComponent; }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetObjectiveTarget(AActor* NewObjectiveTarget);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetRetreatTargetLocation(FVector RetreatLocation);

protected:
	UFUNCTION()
	void HandleObjectiveReached(AActor* ReachedEnemy);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<UEnemyAILODComponent> AILODComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<UEnemySimpleMovementComponent> SimpleMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<UEnemyBehaviorStateComponent> BehaviorStateComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatAttackComponent> AttackComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<AActor> ObjectiveTarget;
};
