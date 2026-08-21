#include "Interaction/GongsimdonCombatTargetActor.h"

#include "Components/BoxComponent.h"
#include "Core/Scenario/ScenarioInteractableComponent.h"

AGongsimdonCombatTargetActor::AGongsimdonCombatTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TargetVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TargetVolume"));
	SetRootComponent(TargetVolume);
	TargetVolume->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TargetVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TargetVolume->SetCollisionResponseToAllChannels(ECR_Block);
	TargetVolume->SetHiddenInGame(true);

	ScenarioInteraction = CreateDefaultSubobject<UScenarioInteractableComponent>(TEXT("ScenarioInteraction"));
	ScenarioInteraction->SupportedInteractionTypes = {EScenarioInteractionType::Combat};
}

void AGongsimdonCombatTargetActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (TargetVolume)
	{
		TargetVolume->SetBoxExtent(TargetExtent);
	}
	if (ScenarioInteraction)
	{
		ScenarioInteraction->TargetID = TargetID;
		ScenarioInteraction->SetInteractionEnabled(bCombatArmed);
	}
}

float AGongsimdonCombatTargetActor::TakeDamage(
	const float DamageAmount,
	const FDamageEvent& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (DamageAmount > 0.0f)
	{
		RegisterHit(DamageCauser);
	}
	return AppliedDamage;
}

bool AGongsimdonCombatTargetActor::RegisterHit(AActor* DamageCauser)
{
	if (!bCombatArmed || !ScenarioInteraction)
	{
		return false;
	}

	CurrentHits = FMath::Min(CurrentHits + 1, FMath::Max(1, RequiredHits));
	OnCombatHit.Broadcast(CurrentHits, RequiredHits);
	ScenarioInteraction->ReportInteractionProgress(
		EScenarioInteractionType::Combat,
		static_cast<float>(CurrentHits) / static_cast<float>(FMath::Max(1, RequiredHits)));
	if (CurrentHits < RequiredHits)
	{
		return true;
	}

	const bool bReported = ScenarioInteraction->ReportInteractionCompleted(EScenarioInteractionType::Combat);
	if (bReported)
	{
		SetCombatArmed(false);
	}
	return bReported;
}

void AGongsimdonCombatTargetActor::SetCombatArmed(const bool bArmed)
{
	bCombatArmed = bArmed;
	CurrentHits = 0;
	if (ScenarioInteraction)
	{
		ScenarioInteraction->SetInteractionEnabled(bArmed);
	}
}
