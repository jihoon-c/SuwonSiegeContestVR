#include "Shared/Combat/HealthComponent.h"

#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetHealth();
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->OnTakeAnyDamage.AddUniqueDynamic(this, &ThisClass::HandleOwnerTakeAnyDamage);
	}
}

float UHealthComponent::ApplyHealthDamage(const float DamageAmount)
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

void UHealthComponent::ResetHealth()
{
	CurrentHealth = FMath::Max(1.0f, MaxHealth);
	OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, MaxHealth, 0.0f);
}

void UHealthComponent::HandleOwnerTakeAnyDamage(
	AActor* DamagedActor,
	const float Damage,
	const UDamageType* DamageType,
	AController* InstigatedBy,
	AActor* DamageCauser)
{
	ApplyHealthDamage(Damage);
}
