#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnCombatHealthChanged, UHealthComponent*, HealthComponent, float, PreviousHealth, float, NewHealth, float, AppliedDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatDamaged, UHealthComponent*, HealthComponent, const FCombatDamageSpec&, DamageSpec);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatDeath, UHealthComponent*, HealthComponent, const FCombatDamageSpec&, KillingDamage);

/** Reusable health state. It does not destroy its owner; game-specific death presentation subscribes to OnDeath. */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	bool ApplyDamage(const FCombatDamageSpec& DamageSpec);

	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	bool RestoreHealth(float Amount);

	/** Restores the component to its configured maximum without treating the reset as healing or damage. */
	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	void ResetHealth();

	/** Direct state override for setup/debugging. Gameplay damage should use ApplyDamage. */
	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	void SetCurrentHealth(float NewHealth);

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnCombatHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnCombatDamaged OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnCombatDeath OnDeath;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Health", meta = (ClampMin = "0.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Health")
	bool bCanBeDamaged = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|Health")
	bool bIsDead = false;

	void BroadcastHealthChanged(float PreviousHealth);
};
