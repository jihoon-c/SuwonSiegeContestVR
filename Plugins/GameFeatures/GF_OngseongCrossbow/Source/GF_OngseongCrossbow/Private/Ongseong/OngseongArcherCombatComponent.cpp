#include "Ongseong/OngseongArcherCombatComponent.h"

#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Characters/EnemyCombatCharacter.h"
#include "Gameplay/Combat/CombatAttackComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/OngseongBoltProjectileActor.h"
#include "TimerManager.h"

UOngseongArcherCombatComponent::UOngseongArcherCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ArrowClass = AOngseongBoltProjectileActor::StaticClass();
}

void UOngseongArcherCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	RandomStream.Initialize(GetOwner() ? GetOwner()->GetUniqueID() : 1);
}

void UOngseongArcherCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateCombat();
	Super::EndPlay(EndPlayReason);
}

void UOngseongArcherCombatComponent::ConfigureCombat(AActor* NewPrimaryTarget, AActor* NewFallbackTarget, AActorPool* NewProjectilePool)
{
	PrimaryTarget = NewPrimaryTarget;
	FallbackTarget = NewFallbackTarget;
	ProjectilePool = NewProjectilePool;
}

void UOngseongArcherCombatComponent::ApplyTuning(const float InHitChance, const float InRange, const float InInterval,
	const float InMissRadius, const float InDamage, const float InProjectileSpeed)
{
	HitChance = FMath::Clamp(InHitChance, 0.0f, 1.0f);
	EngagementRange = FMath::Max(100.0f, InRange);
	FireInterval = FMath::Max(0.1f, InInterval);
	MissRadius = FMath::Max(0.0f, InMissRadius);
	ArrowDamage = FMath::Max(0.0f, InDamage);
	ArrowSpeed = FMath::Max(1.0f, InProjectileSpeed);
}

void UOngseongArcherCombatComponent::ActivateCombat()
{
	bCombatActive = true;
}

void UOngseongArcherCombatComponent::DeactivateCombat()
{
	bCombatActive = false;
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimerHandle);
	FinishAttackAnimation();
}

AActor* UOngseongArcherCombatComponent::GetCurrentTarget() const
{
	if (IsUsableTarget(PrimaryTarget)) return PrimaryTarget;
	return IsUsableTarget(FallbackTarget) ? FallbackTarget.Get() : nullptr;
}

bool UOngseongArcherCombatComponent::IsUsableTarget(const AActor* Target) const
{
	if (!IsValid(Target)) return false;
	if (const UHealthComponent* Health = Target->FindComponentByClass<UHealthComponent>()) return !Health->IsDead();
	return true;
}

bool UOngseongArcherCombatComponent::IsInFiringPosition() const
{
	const AActor* Owner = GetOwner();
	const AActor* Target = GetCurrentTarget();
	return bCombatActive && Owner && Target && FVector::DistSquared2D(Owner->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(EngagementRange);
}

bool UOngseongArcherCombatComponent::TryFireArrow()
{
	AActor* Target = GetCurrentTarget();
	if (!IsInFiringPosition() || !Target || !GetOwner()) return false;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (ACombatAIController* Controller = OwnerPawn ? Cast<ACombatAIController>(OwnerPawn->GetController()) : nullptr)
	{
		Controller->StopCombatMovement();
		Controller->SetCombatTarget(Target);
		Controller->SetFocus(Target);
		Controller->SetAttacking(true);
	}
	const bool bIntendedHit = RandomStream.FRand() <= HitChance;
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + FVector::UpVector * SpawnHeight + GetOwner()->GetActorForwardVector() * 40.0f;
	const FVector Direction = (BuildAimPoint(Target, bIntendedHit) - SpawnLocation).GetSafeNormal();
	AGameplayProjectileActor* Projectile = SpawnArrow(SpawnLocation, Direction);
	if (!Projectile)
	{
		FinishAttackAnimation();
		return false;
	}
	OnArrowFired.Broadcast(Target, Projectile, bIntendedHit);
	if (GetWorld()) GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimerHandle, this, &UOngseongArcherCombatComponent::FinishAttackAnimation, AttackAnimationDuration, false);
	return true;
}

void UOngseongArcherCombatComponent::FinishAttackAnimation()
{
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (ACombatAIController* Controller = Cast<ACombatAIController>(OwnerPawn->GetController()))
		{
			Controller->SetAttacking(false);
			Controller->ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}

FVector UOngseongArcherCombatComponent::BuildAimPoint(AActor* Target, const bool bIntendedHit)
{
	FVector Origin;
	FVector Extent;
	Target->GetActorBounds(true, Origin, Extent);
	if (bIntendedHit) return Origin;
	FVector Offset = RandomStream.VRand();
	Offset.Z = FMath::Clamp(Offset.Z, -0.35f, 0.65f);
	Offset = Offset.GetSafeNormal() * FMath::Max(MissRadius, Extent.Size() + 25.0f);
	return Origin + Offset;
}

AGameplayProjectileActor* UOngseongArcherCombatComponent::SpawnArrow(const FVector& SpawnLocation, const FVector& Direction)
{
	if (!ArrowClass || !GetWorld() || Direction.IsNearlyZero()) return nullptr;
	AGameplayProjectileActor* Projectile = nullptr;
	if (ProjectilePool)
	{
		AActor* Acquired = ProjectilePool->AcquireActor(FTransform(Direction.Rotation(), SpawnLocation));
		Projectile = Cast<AGameplayProjectileActor>(Acquired);
		if (!Projectile && Acquired) ProjectilePool->ReleaseActor(Acquired);
	}
	else
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Projectile = GetWorld()->SpawnActor<AGameplayProjectileActor>(ArrowClass, SpawnLocation, Direction.Rotation(), Params);
	}
	if (!Projectile) return nullptr;
	FCombatDamageSpec Spec;
	Spec.Amount = ArrowDamage;
	Spec.InstigatorActor = GetOwner();
	Spec.DamageCauser = Projectile;
	Projectile->LaunchProjectile(Direction, ArrowSpeed, Spec);
	return Projectile;
}
