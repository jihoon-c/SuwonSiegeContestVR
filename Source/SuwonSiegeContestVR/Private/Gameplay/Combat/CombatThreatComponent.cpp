#include "Gameplay/Combat/CombatThreatComponent.h"

#include "Gameplay/Combat/HealthComponent.h"

UCombatThreatComponent::UCombatThreatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatThreatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UHealthComponent>())
	{
		HealthComponent->OnDamaged.AddDynamic(this, &UCombatThreatComponent::HandleOwnerDamaged);
	}
}

void UCombatThreatComponent::RegisterAttacker(AActor* Attacker)
{
	if (IsValid(Attacker) && Attacker != GetOwner() && GetWorld())
	{
		RecentAttackers.FindOrAdd(Attacker) = GetWorld()->GetTimeSeconds() + AttackerMemorySeconds;
	}
}

TArray<AActor*> UCombatThreatComponent::GetActiveAttackers()
{
	PruneExpiredAttackers();
	TArray<AActor*> Attackers;
	RecentAttackers.GetKeys(Attackers);
	return Attackers;
}

void UCombatThreatComponent::HandleOwnerDamaged(UHealthComponent* DamagedHealthComponent, const FCombatDamageSpec& DamageSpec)
{
	RegisterAttacker(DamageSpec.InstigatorActor ? DamageSpec.InstigatorActor.Get() : DamageSpec.DamageCauser.Get());
}

void UCombatThreatComponent::PruneExpiredAttackers()
{
	if (!GetWorld())
	{
		RecentAttackers.Empty();
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	for (auto Iterator = RecentAttackers.CreateIterator(); Iterator; ++Iterator)
	{
		if (!IsValid(Iterator.Key()) || Iterator.Value() <= CurrentTime)
		{
			Iterator.RemoveCurrent();
		}
	}
}
