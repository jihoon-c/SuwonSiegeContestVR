#include "Shared/Combat/FactionComponent.h"

UFactionComponent::UFactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFactionComponent::IsHostileTo(const UFactionComponent* Other) const
{
	if (!Other || Faction == ECombatFaction::Neutral || Other->Faction == ECombatFaction::Neutral)
	{
		return false;
	}
	return (Faction == ECombatFaction::Enemy) != (Other->Faction == ECombatFaction::Enemy);
}
