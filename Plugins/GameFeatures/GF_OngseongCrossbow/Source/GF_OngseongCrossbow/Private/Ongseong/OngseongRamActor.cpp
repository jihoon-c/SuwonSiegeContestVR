#include "Ongseong/OngseongRamActor.h"

#include "Components/StaticMeshComponent.h"
#include "Gameplay/Combat/CombatDamageLibrary.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/DamageReceiverInterface.h"
#include "Gameplay/Combat/HealthComponent.h"

AOngseongRamActor::AOngseongRamActor()
{
	PrimaryActorTick.bCanEverTick = true;
	RamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RamMesh"));
	SetRootComponent(RamMesh);
	RamMesh->SetMobility(EComponentMobility::Movable);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	FactionComponent = CreateDefaultSubobject<UCombatFactionComponent>(TEXT("FactionComponent"));
	FactionComponent->SetFaction(ECombatFaction::Enemy);
}

void AOngseongRamActor::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnDeath.AddUniqueDynamic(this, &AOngseongRamActor::HandleDeath);
	if (GateTarget) ActivateRam(GateTarget);
}

void AOngseongRamActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (RamState == EOngseongRamState::Inactive || !IsValid(GateTarget)) return;

	switch (RamState)
	{
	case EOngseongRamState::Advancing:
		if (MoveTowards(StagingLocation, MoveSpeed, DeltaSeconds)) BeginCharge();
		break;
	case EOngseongRamState::Charging:
	{
		FVector ToGate = GateTarget->GetActorLocation() - StagingLocation;
		ToGate.Z = 0.0f;
		FVector ImpactLocation = GateTarget->GetActorLocation() - ToGate.GetSafeNormal() * ImpactDistance;
		ImpactLocation.Z = GetActorLocation().Z;
		if (MoveTowards(ImpactLocation, ChargeSpeed, DeltaSeconds)) ImpactGate();
		break;
	}
	case EOngseongRamState::Returning:
		if (MoveTowards(StagingLocation, ReturnSpeed, DeltaSeconds)) BeginCharge();
		break;
	default:
		break;
	}
}

bool AOngseongRamActor::ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec)
{
	return HealthComponent && HealthComponent->ApplyDamage(DamageSpec);
}

void AOngseongRamActor::ActivateRam(AActor* NewGateTarget)
{
	GateTarget = NewGateTarget;
	if (!IsValid(GateTarget) || !HealthComponent || HealthComponent->IsDead())
	{
		StopRam();
		return;
	}
	FVector ApproachDirection = GateTarget->GetActorLocation() - GetActorLocation();
	ApproachDirection.Z = 0.0f;
	StagingLocation = GateTarget->GetActorLocation() - ApproachDirection.GetSafeNormal() * StagingDistance;
	StagingLocation.Z = GetActorLocation().Z;
	RamState = EOngseongRamState::Advancing;
}

void AOngseongRamActor::StopRam()
{
	RamState = EOngseongRamState::Inactive;
}

bool AOngseongRamActor::MoveTowards(const FVector& TargetLocation, const float Speed, const float DeltaSeconds)
{
	FVector Delta = TargetLocation - GetActorLocation();
	Delta.Z = 0.0f;
	const float Distance = Delta.Size();
	if (Distance <= ArrivalTolerance)
	{
		SetActorLocation(TargetLocation, true);
		return true;
	}
	const FVector Direction = Delta / Distance;
	SetActorRotation(Direction.Rotation());
	const float Step = FMath::Min(Distance, FMath::Max(0.0f, Speed) * DeltaSeconds);
	SetActorLocation(GetActorLocation() + Direction * Step, true);
	return Distance - Step <= ArrivalTolerance;
}

void AOngseongRamActor::BeginCharge()
{
	if (RamState == EOngseongRamState::Inactive || !IsValid(GateTarget))
	{
		StopRam();
		return;
	}
	RamState = EOngseongRamState::Charging;
}

void AOngseongRamActor::ImpactGate()
{
	if (RamState != EOngseongRamState::Charging || !IsValid(GateTarget))
	{
		StopRam();
		return;
	}
	FCombatDamageSpec Damage;
	Damage.Amount = AttackDamage;
	Damage.InstigatorActor = this;
	Damage.DamageCauser = this;
	// The ram can only strike its explicitly assigned gate target, so faction
	// filtering must not suppress this scripted objective interaction.
	Damage.bIgnoreFaction = true;
	if (IDamageReceiverInterface* DamageReceiver = Cast<IDamageReceiverInterface>(GateTarget))
	{
		DamageReceiver->ReceiveCombatDamage_Implementation(Damage);
	}
	else
	{
		UCombatDamageLibrary::ApplyCombatDamage(GateTarget, Damage);
	}
	if (RamState == EOngseongRamState::Charging)
	{
		RamState = EOngseongRamState::Returning;
	}
}

void AOngseongRamActor::HandleDeath(UHealthComponent* DeadHealth, const FCombatDamageSpec& KillingDamage)
{
	StopRam();
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}
