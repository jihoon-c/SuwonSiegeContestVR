#include "Shared/Combat/FactionComponent.h"

UFactionComponent::UFactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFactionComponent::IsHostileTo(const UFactionComponent* Other) const
{
	if (!Other || Faction == ELegacyCombatFaction::Neutral || Other->Faction == ELegacyCombatFaction::Neutral)
	{
		return false;
	}
	return (Faction == ELegacyCombatFaction::Enemy) != (Other->Faction == ELegacyCombatFaction::Enemy);
}
