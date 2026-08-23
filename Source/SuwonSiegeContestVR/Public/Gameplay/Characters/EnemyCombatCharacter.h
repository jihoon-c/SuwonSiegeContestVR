#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/CombatCharacter.h"
#include "Gameplay/Pooling/PoolableActorInterface.h"
#include "EnemyCombatCharacter.generated.h"

class UCombatAttackComponent;

UCLASS(Abstract, Blueprintable)
class SUWONSIEGECONTESTVR_API AEnemyCombatCharacter : public ACombatCharacter, public IPoolableActorInterface
{
	GENERATED_BODY()

public:
	AEnemyCombatCharacter();
	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatAttackComponent* GetAttackComponent() const { return AttackComponent; }

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetObjectiveTarget(AActor* NewObjectiveTarget);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|AI")
	void SetRetreatTargetLocation(FVector RetreatLocation);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatAttackComponent> AttackComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gameplay|AI")
	TObjectPtr<AActor> ObjectiveTarget;
};
