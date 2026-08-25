#include "Gameplay/Characters/AllyCombatCharacter.h"

#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Combat/CombatFactionComponent.h"

AAllyCombatCharacter::AAllyCombatCharacter()
{
	FactionComponent->SetFaction(ECombatFaction::Ally);
	AIControllerClass = ACombatAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
