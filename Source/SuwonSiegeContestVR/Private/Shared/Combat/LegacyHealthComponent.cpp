#include "Shared/Combat/LegacyHealthComponent.h"

#include "GameFramework/Actor.h"

ULegacyHealthComponent::ULegacyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULegacyHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetHealth();
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->OnTakeAnyDamage.AddUniqueDynamic(this, &ThisClass::HandleOwnerTakeAnyDamage);
	}
}

float ULegacyHealthComponent::ApplyHealthDamage(const float DamageAmount)
{
	if (DamageAmount <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, FMath::Max(1.0f, MaxHealth));
	const float AppliedDamage = PreviousHealth - CurrentHealth;
	OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, MaxHealth, -AppliedDamage);
	if (CurrentHealth <= 0.0f)
	{
		OnHealthDepleted.Broadcast(GetOwner());
	}
	return AppliedDamage;
}

void ULegacyHealthComponent::ResetHealth()
{
	CurrentHealth = FMath::Max(1.0f, MaxHealth);
	OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, MaxHealth, 0.0f);
}

void ULegacyHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	const float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser)
{
	ApplyHealthDamage(Damage);
}
