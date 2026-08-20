#include "Gameplay/Characters/CombatCharacter.h"

#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"

ACombatCharacter::ACombatCharacter()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	FactionComponent = CreateDefaultSubobject<UCombatFactionComponent>(TEXT("FactionComponent"));
}

bool ACombatCharacter::ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec)
{
	return HealthComponent && HealthComponent->ApplyDamage(DamageSpec);
}
