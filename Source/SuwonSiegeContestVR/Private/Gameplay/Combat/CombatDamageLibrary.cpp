#include "Gameplay/Combat/CombatDamageLibrary.h"

#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "Gameplay/Combat/HealthComponent.h"

bool UCombatDamageLibrary::ApplyCombatDamage(AActor* Target, const FCombatDamageSpec& DamageSpec)
{
	if (!CanDamageTarget(Target, DamageSpec))
	{
		return false;
	}

	if (Target->GetClass()->ImplementsInterface(UDamageReceiverInterface::StaticClass()))
	{
		return IDamageReceiverInterface::Execute_ReceiveCombatDamage(Target, DamageSpec);
	}

	if (UHealthComponent* HealthComponent = FindHealthComponent(Target))
	{
		return HealthComponent->ApplyDamage(DamageSpec);
	}

	return false;
}

bool UCombatDamageLibrary::CanDamageTarget(const AActor* Target, const FCombatDamageSpec& DamageSpec)
{
	if (!IsValid(Target) || DamageSpec.Amount <= 0.0f || Target == DamageSpec.InstigatorActor)
	{
		return false;
	}

	if (DamageSpec.bIgnoreFaction)
	{
		return true;
	}

	const AActor* Source = DamageSpec.InstigatorActor ? DamageSpec.InstigatorActor.Get() : DamageSpec.DamageCauser.Get();
	if (!IsValid(Source))
	{
		return true;
	}

	const UCombatFactionComponent* SourceFaction = Source->FindComponentByClass<UCombatFactionComponent>();
	const UCombatFactionComponent* TargetFaction = Target->FindComponentByClass<UCombatFactionComponent>();
	return !SourceFaction || !TargetFaction || UCombatFactionComponent::AreHostile(SourceFaction->GetFaction(), TargetFaction->GetFaction());
}

UHealthComponent* UCombatDamageLibrary::FindHealthComponent(const AActor* Target)
{
	return IsValid(Target) ? Target->FindComponentByClass<UHealthComponent>() : nullptr;
}
