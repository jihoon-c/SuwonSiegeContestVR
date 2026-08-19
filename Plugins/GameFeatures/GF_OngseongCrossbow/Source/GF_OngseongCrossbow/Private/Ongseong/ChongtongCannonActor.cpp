#include "Ongseong/ChongtongCannonActor.h"

#include "Components/SceneComponent.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Combat/CombatTargetingComponent.h"
#include "Gameplay/Combat/CombatThreatComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Ongseong/ChongtongProjectileActor.h"
#include "TimerManager.h"

AChongtongCannonActor::AChongtongCannonActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(Root);
	FactionComponent = CreateDefaultSubobject<UCombatFactionComponent>(TEXT("FactionComponent"));
	FactionComponent->SetFaction(ECombatFaction::Ally);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	ThreatComponent = CreateDefaultSubobject<UCombatThreatComponent>(TEXT("ThreatComponent"));
	TargetingComponent = CreateDefaultSubobject<UCombatTargetingComponent>(TEXT("TargetingComponent"));
	ProjectileClass = AChongtongProjectileActor::StaticClass();
}

void AChongtongCannonActor::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AChongtongCannonActor::FireScheduledShot, FireInterval, true);
}

void AChongtongCannonActor::SetGateTarget(AActor* NewGateTarget)
{
	GateTarget = NewGateTarget;
}

bool AChongtongCannonActor::TryFire()
{
	AActor* Target = SelectTarget();
	if (!IsValid(Target) || !ProjectileClass)
	{
		return false;
	}

	const FVector MuzzleLocation = Muzzle->GetComponentLocation();
	const FVector Direction = (Target->GetActorLocation() - MuzzleLocation).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = nullptr;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGameplayProjectileActor* Projectile = GetWorld()->SpawnActor<AGameplayProjectileActor>(ProjectileClass, MuzzleLocation, Direction.Rotation(), SpawnParameters);
	if (!Projectile)
	{
		return false;
	}

	FCombatDamageSpec DamageSpec;
	DamageSpec.Amount = ProjectileDamage;
	DamageSpec.InstigatorActor = this;
	DamageSpec.DamageCauser = Projectile;
	Projectile->LaunchProjectile(Direction, ProjectileSpeed, DamageSpec);
	OnFired.Broadcast(Target, Projectile);
	return true;
}

AActor* AChongtongCannonActor::SelectTarget() const
{
	const TArray<AActor*> HostileTargets = TargetingComponent->FindHostileTargets(FireRange);
	if (HostileTargets.IsEmpty())
	{
		return nullptr;
	}

	const TArray<AActor*> Attackers = ThreatComponent->GetActiveAttackers();
	for (AActor* Attacker : Attackers)
	{
		if (HostileTargets.Contains(Attacker))
		{
			return Attacker;
		}
	}

	if (AActor* GatePriorityTarget = TargetingComponent->SelectClosestTo(HostileTargets, GateTarget))
	{
		return GatePriorityTarget;
	}

	return TargetingComponent->SelectRandom(HostileTargets);
}

void AChongtongCannonActor::FireScheduledShot()
{
	TryFire();
}
