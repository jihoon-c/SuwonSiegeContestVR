#include "Ongseong/OngseongArcherCombatComponent.h"

#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Characters/EnemyCombatCharacter.h"
#include "Gameplay/Combat/CombatAttackComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/OngseongBoltProjectileActor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

UOngseongArcherCombatComponent::UOngseongArcherCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ArrowClass = AOngseongBoltProjectileActor::StaticClass();
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> DefaultAttackAnimation(
		TEXT("/GF_OngseongCrossbow/Asset/Character/Enemy/AS_Shooting.AS_Shooting"));
	AttackAnimation = DefaultAttackAnimation.Object;
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

void UOngseongArcherCombatComponent::ConfigureCombat(AActor* NewCannonTarget, AActor* NewPlayerTarget, AActorPool* NewProjectilePool)
{
	PrimaryTarget = NewCannonTarget;
	FallbackTarget = NewPlayerTarget;
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
	// The Behavior Tree's fire task only runs on the branch that follows a completed move.
	// When that branch never completes the archer stood there without ever firing, so the
	// recurring attempt is owned here. TryFireArrow no-ops while out of range or mid-attack.
	if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		const float Interval = FMath::Max(0.1f, FireInterval);
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
			&UOngseongArcherCombatComponent::PerformScheduledShot, Interval, true, Interval);
	}
}

void UOngseongArcherCombatComponent::DeactivateCombat()
{
	bCombatActive = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackAnimationTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	}
	FinishAttackAnimation();
}

AActor* UOngseongArcherCombatComponent::GetCurrentTarget() const
{
	const AActor* Owner = GetOwner();
	const bool bHasCannon = IsUsableTarget(PrimaryTarget);
	const bool bHasPlayer = IsUsableTarget(FallbackTarget);
	if (!Owner || !bHasCannon) return bHasPlayer ? FallbackTarget.Get() : nullptr;
	if (!bHasPlayer) return PrimaryTarget;
	return FVector::DistSquared(Owner->GetActorLocation(), PrimaryTarget->GetActorLocation())
		<= FVector::DistSquared(Owner->GetActorLocation(), FallbackTarget->GetActorLocation())
		? PrimaryTarget.Get() : FallbackTarget.Get();
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
	if (bAttackInProgress || !IsInFiringPosition() || !Target || !GetOwner()) return false;
	bPendingIntendedHit = RandomStream.FRand() <= HitChance;
	BeginAttackAnimation();
	return true;
}

void UOngseongArcherCombatComponent::BeginAttackAnimation()
{
	AActor* Target = GetCurrentTarget();
	if (!Target || !GetOwner() || !GetWorld()) return;
	bAttackInProgress = true;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (ACombatAIController* Controller = OwnerPawn ? Cast<ACombatAIController>(OwnerPawn->GetController()) : nullptr)
	{
		Controller->StopCombatMovement();
		Controller->SetCombatTarget(Target);
		Controller->SetFocus(Target);
	}

	float AttackDuration = FallbackAttackAnimationDuration;
	if (USkeletalMeshComponent* Mesh = OwnerPawn ? OwnerPawn->FindComponentByClass<USkeletalMeshComponent>() : nullptr)
	{
		if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance(); AnimInstance && AttackAnimation)
		{
			if (UAnimMontage* Montage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
				AttackAnimation, AttackAnimationSlot, AttackBlendInTime, AttackBlendOutTime))
			{
				AttackDuration = Montage->GetPlayLength();
			}
		}
	}
	GetWorld()->GetTimerManager().SetTimer(AttackAnimationTimerHandle, this,
		&UOngseongArcherCombatComponent::FirePendingArrow, FMath::Max(0.01f, AttackDuration), false);
	// The Behavior Tree gets us into position and kicks off the first shot. Own the recurring
	// fire loop here so a BT branch that completes after one task cannot silence the archer.
	if (!GetWorld()->GetTimerManager().IsTimerActive(FireTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
			&UOngseongArcherCombatComponent::PerformScheduledShot, FireInterval, true, FireInterval);
	}
}

void UOngseongArcherCombatComponent::FirePendingArrow()
{
	AActor* Target = GetCurrentTarget();
	if (bCombatActive && Target && GetOwner())
	{
		const FVector SpawnLocation = GetOwner()->GetActorLocation() + FVector::UpVector * SpawnHeight + GetOwner()->GetActorForwardVector() * 40.0f;
		const FVector Direction = (BuildAimPoint(Target, bPendingIntendedHit) - SpawnLocation).GetSafeNormal();
		if (AGameplayProjectileActor* Projectile = SpawnArrow(SpawnLocation, Direction))
		{
			OnArrowFired.Broadcast(Target, Projectile, bPendingIntendedHit);
		}
	}
	FinishAttackAnimation();
}


void UOngseongArcherCombatComponent::PerformScheduledShot()
{
	TryFireArrow();
}

void UOngseongArcherCombatComponent::FinishAttackAnimation()
{
	bAttackInProgress = false;
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (ACombatAIController* Controller = Cast<ACombatAIController>(OwnerPawn->GetController()))
		{
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
