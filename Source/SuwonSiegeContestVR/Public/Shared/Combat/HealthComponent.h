#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnHealthChanged, AActor*, OwnerActor, float, CurrentHealth, float, MaxHealth, float, HealthDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthDepleted, AActor*, OwnerActor);

/** Lightweight shared health that consumes standard Unreal damage events. */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	float ApplyHealthDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	void ResetHealth();

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnHealthDepleted OnHealthDepleted;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOwnerTakeAnyDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser);

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Combat|Health")
	float CurrentHealth = 100.0f;
};
