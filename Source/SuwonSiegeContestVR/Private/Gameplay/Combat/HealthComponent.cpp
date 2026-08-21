#include "Gameplay/Combat/HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	MaxHealth = FMath::Max(0.0f, MaxHealth);
	CurrentHealth = MaxHealth;
	bIsDead = CurrentHealth <= 0.0f;
}

bool UHealthComponent::ApplyDamage(const FCombatDamageSpec& DamageSpec)
{
	if (!bCanBeDamaged || bIsDead || DamageSpec.Amount <= 0.0f)
	{
		return false;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageSpec.Amount);
	const float AppliedDamage = PreviousHealth - CurrentHealth;
	if (AppliedDamage <= 0.0f)
	{
		return false;
	}

	OnDamaged.Broadcast(this, DamageSpec);
	BroadcastHealthChanged(PreviousHealth);
	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast(this, DamageSpec);
	}
	return true;
}

bool UHealthComponent::RestoreHealth(const float Amount)
{
	if (Amount <= 0.0f || bIsDead)
	{
		return false;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
	if (FMath::IsNearlyEqual(CurrentHealth, PreviousHealth))
	{
		return false;
	}

	BroadcastHealthChanged(PreviousHealth);
	return true;
}

void UHealthComponent::ResetHealth()
{
	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Max(0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;
	if (!FMath::IsNearlyEqual(PreviousHealth, CurrentHealth))
	{
		BroadcastHealthChanged(PreviousHealth);
	}
}

void UHealthComponent::SetCurrentHealth(const float NewHealth)
{
	const float PreviousHealth = CurrentHealth;
	const bool bWasDead = bIsDead;
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	bIsDead = CurrentHealth <= 0.0f;
	if (!FMath::IsNearlyEqual(PreviousHealth, CurrentHealth))
	{
		BroadcastHealthChanged(PreviousHealth);
	}
	if (!bWasDead && bIsDead)
	{
		FCombatDamageSpec StateChangeDamage;
		StateChangeDamage.Amount = PreviousHealth - CurrentHealth;
		OnDeath.Broadcast(this, StateChangeDamage);
	}
}

void UHealthComponent::BroadcastHealthChanged(const float PreviousHealth)
{
	OnHealthChanged.Broadcast(this, PreviousHealth, CurrentHealth, CurrentHealth - PreviousHealth);
}
