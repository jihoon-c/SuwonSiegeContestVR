#include "Gameplay/Characters/AllyCombatCharacter.h"

#include "Gameplay/Combat/CombatFactionComponent.h"

AAllyCombatCharacter::AAllyCombatCharacter()
{
	FactionComponent->SetFaction(ECombatFaction::Ally);
}
