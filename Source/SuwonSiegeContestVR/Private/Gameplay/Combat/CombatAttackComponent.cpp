#include "Gameplay/Combat/CombatAttackComponent.h"

#include "Gameplay/Combat/CombatDamageLibrary.h"
#include "TimerManager.h"

UCombatAttackComponent::UCombatAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatAttackComponent::BeginPlay()
{
	Super::BeginPlay();
	SetAttackEnabled(bAttackEnabled);
}

void UCombatAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetAttackEnabled(false);
	AttackTarget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UCombatAttackComponent::SetAttackTarget(AActor* NewTarget)
{
	AttackTarget = NewTarget;
}

void UCombatAttackComponent::SetAttackEnabled(const bool bEnabled)
{
	bAttackEnabled = bEnabled;
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
	if (bAttackEnabled)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &UCombatAttackComponent::PerformScheduledAttack, AttackInterval, true);
	}
}

bool UCombatAttackComponent::TryPerformAttack()
{
	AActor* Owner = GetOwner();
	if (!bAttackEnabled || !IsValid(Owner) || !IsValid(AttackTarget) || FVector::DistSquared(Owner->GetActorLocation(), AttackTarget->GetActorLocation()) > FMath::Square(AttackRange))
	{
		return false;
	}

	FCombatDamageSpec DamageSpec;
	DamageSpec.Amount = DamageAmount;
	DamageSpec.InstigatorActor = Owner;
	DamageSpec.DamageCauser = Owner;
	if (!UCombatDamageLibrary::ApplyCombatDamage(AttackTarget, DamageSpec))
	{
		return false;
	}

	OnAttackPerformed.Broadcast(Owner, AttackTarget);
	return true;
}

void UCombatAttackComponent::PerformScheduledAttack()
{
	TryPerformAttack();
}
