#include "Ongseong/OngseongArcherCombatComponent.h"

#include "Gameplay/AI/EnemyBehaviorStateComponent.h"
#include "Gameplay/AI/EnemySimpleMovementComponent.h"
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
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f;
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
	bInFiringPosition = false;
	SetComponentTickEnabled(true);
	if (GetWorld()) GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UOngseongArcherCombatComponent::FireScheduledArrow, FireInterval, true, FireInterval);
}

void UOngseongArcherCombatComponent::DeactivateCombat()
{
	bCombatActive = false;
	bInFiringPosition = false;
	SetComponentTickEnabled(false);
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
}

void UOngseongArcherCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bCombatActive) UpdateFiringPosition();
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

void UOngseongArcherCombatComponent::UpdateFiringPosition()
{
	AEnemyCombatCharacter* Archer = Cast<AEnemyCombatCharacter>(GetOwner());
	AActor* Target = GetCurrentTarget();
	if (!Archer || !Target) { DeactivateCombat(); return; }
	const float DistanceSquared = FVector::DistSquared2D(Archer->GetActorLocation(), Target->GetActorLocation());
	if (DistanceSquared <= FMath::Square(EngagementRange))
	{
		bInFiringPosition = true;
		Archer->GetSimpleMovementComponent()->ClearMoveTarget();
		Archer->GetAttackComponent()->SetAttackEnabled(false);
		Archer->GetBehaviorStateComponent()->SetBehaviorState(TEXT("ArcherFiring"));
		const FVector FlatDirection = (Target->GetActorLocation() - Archer->GetActorLocation()).GetSafeNormal2D();
		if (!FlatDirection.IsNearlyZero()) Archer->SetActorRotation(FlatDirection.Rotation());
	}
	else
	{
		bInFiringPosition = false;
		Archer->GetAttackComponent()->SetAttackEnabled(false);
		const FVector DirectionFromTarget = (Archer->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
		Archer->GetSimpleMovementComponent()->SetMoveTargetLocation(Target->GetActorLocation() + DirectionFromTarget * EngagementRange * 0.85f);
		Archer->GetBehaviorStateComponent()->SetBehaviorState(TEXT("ArcherAdvance"));
	}
}

bool UOngseongArcherCombatComponent::TryFireArrow()
{
	AActor* Target = GetCurrentTarget();
	if (!bCombatActive || !bInFiringPosition || !Target || !GetOwner()) return false;
	const bool bIntendedHit = RandomStream.FRand() <= HitChance;
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + FVector::UpVector * SpawnHeight + GetOwner()->GetActorForwardVector() * 40.0f;
	const FVector Direction = (BuildAimPoint(Target, bIntendedHit) - SpawnLocation).GetSafeNormal();
	AGameplayProjectileActor* Projectile = SpawnArrow(SpawnLocation, Direction);
	if (!Projectile) return false;
	OnArrowFired.Broadcast(Target, Projectile, bIntendedHit);
	return true;
}

void UOngseongArcherCombatComponent::FireScheduledArrow()
{
	TryFireArrow();
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
