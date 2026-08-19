#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "CombatAttackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatAttackPerformed, AActor*, Attacker, AActor*, Target);

/** Low-frequency direct attack for enemies damaging a gate, cannon, or other strategic Actor. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UCombatAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatAttackComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	void SetAttackTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	void SetAttackEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
	bool TryPerformAttack();

	UPROPERTY(BlueprintAssignable, Category = "Combat|Attack")
	FOnCombatAttackPerformed OnAttackPerformed;

protected:
	void PerformScheduledAttack();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Attack", meta = (ClampMin = "0.0"))
	float DamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Attack", meta = (ClampMin = "0.0"))
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Attack", meta = (ClampMin = "0.1"))
	float AttackInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Attack")
	TObjectPtr<AActor> AttackTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Attack")
	bool bAttackEnabled = false;

	FTimerHandle AttackTimerHandle;
};
