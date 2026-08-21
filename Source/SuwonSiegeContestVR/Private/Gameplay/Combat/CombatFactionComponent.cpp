#include "Gameplay/Combat/CombatFactionComponent.h"

#include "GameFramework/Actor.h"

UCombatFactionComponent::UCombatFactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatFactionComponent::SetFaction(const ECombatFaction NewFaction)
{
	if (Faction == NewFaction)
	{
		return;
	}

	const ECombatFaction PreviousFaction = Faction;
	Faction = NewFaction;
	OnFactionChanged.Broadcast(PreviousFaction, Faction);
}

bool UCombatFactionComponent::IsHostileTo(const AActor* OtherActor) const
{
	if (!IsValid(OtherActor))
	{
		return false;
	}

	const UCombatFactionComponent* OtherFaction = OtherActor->FindComponentByClass<UCombatFactionComponent>();
	return OtherFaction && AreHostile(Faction, OtherFaction->Faction);
}

bool UCombatFactionComponent::AreHostile(const ECombatFaction FirstFaction, const ECombatFaction SecondFaction)
{
	if (FirstFaction == ECombatFaction::Neutral || SecondFaction == ECombatFaction::Neutral || FirstFaction == SecondFaction)
	{
		return false;
	}

	// Player and Ally cooperate; every non-neutral faction is hostile to Enemy.
	return FirstFaction == ECombatFaction::Enemy || SecondFaction == ECombatFaction::Enemy;
}
