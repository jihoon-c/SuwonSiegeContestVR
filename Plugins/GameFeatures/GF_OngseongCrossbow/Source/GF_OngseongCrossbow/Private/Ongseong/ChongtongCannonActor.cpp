#include "Ongseong/ChongtongCannonActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Characters/AllyCombatCharacter.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Combat/CombatTargetingComponent.h"
#include "Gameplay/Combat/CombatThreatComponent.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/ChongtongProjectileActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AChongtongCannonActor::AChongtongCannonActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	HwachaBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HwachaBaseMesh"));
	HwachaBaseMesh->SetupAttachment(Root);
	ChongtongMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChongtongMesh"));
	ChongtongMesh->SetupAttachment(HwachaBaseMesh);
	ChongtongMesh->SetRelativeLocation(FVector(610.0f, 150.0f, 100.0f));
	ChongtongMesh->SetRelativeScale3D(FVector(100.0f));
	OperatorSeat = CreateDefaultSubobject<USceneComponent>(TEXT("OperatorSeat"));
	OperatorSeat->SetupAttachment(HwachaBaseMesh);
	Muzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle"));
	Muzzle->SetupAttachment(ChongtongMesh);
	Muzzle->SetRelativeLocation(FVector(0.0f, 1345.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> HwachaBaseAsset(TEXT("/GF_OngseongCrossbow/Art/Namhansanseong/Chongtong/Hwacha/SM_Chongtong_Hwacha.SM_Chongtong_Hwacha"));
	if (HwachaBaseAsset.Succeeded())
	{
		HwachaBaseMesh->SetStaticMesh(HwachaBaseAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ChongtongAsset(TEXT("/GF_OngseongCrossbow/Art/Namhansanseong/Chongtong/FourChongtong/SM_Four_Chongtong_Gun.SM_Four_Chongtong_Gun"));
	if (ChongtongAsset.Succeeded())
	{
		ChongtongMesh->SetStaticMesh(ChongtongAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<AAllyCombatCharacter> OperatorBlueprintClass(TEXT("/GF_OngseongCrossbow/Blueprints/BP_ChongtongOperator"));
	if (OperatorBlueprintClass.Succeeded())
	{
		OperatorClass = OperatorBlueprintClass.Class;
	}
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
	HealthComponent->OnDeath.AddUniqueDynamic(this, &AChongtongCannonActor::HandleDeath);
	if (bSpawnOperatorOnBeginPlay)
	{
		SpawnMountedOperator();
	}
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AChongtongCannonActor::FireScheduledShot, FireInterval, true);
}

void AChongtongCannonActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveMountedOperator();
	Super::EndPlay(EndPlayReason);
}

bool AChongtongCannonActor::ReceiveCombatDamage_Implementation(const FCombatDamageSpec& DamageSpec)
{
	return HealthComponent && HealthComponent->ApplyDamage(DamageSpec);
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

	AGameplayProjectileActor* Projectile = nullptr;
	if (ProjectilePool)
	{
		AActor* AcquiredActor = ProjectilePool->AcquireActor(FTransform(Direction.Rotation(), MuzzleLocation));
		Projectile = Cast<AGameplayProjectileActor>(AcquiredActor);
		if (!Projectile && AcquiredActor)
		{
			ProjectilePool->ReleaseActor(AcquiredActor);
		}
	}
	else
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = nullptr;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Projectile = GetWorld()->SpawnActor<AGameplayProjectileActor>(ProjectileClass, MuzzleLocation, Direction.Rotation(), SpawnParameters);
	}
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

bool AChongtongCannonActor::SpawnMountedOperator()
{
	if (IsValid(MountedOperator))
	{
		return true;
	}

	if (!OperatorClass || !GetWorld())
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	MountedOperator = GetWorld()->SpawnActor<AAllyCombatCharacter>(OperatorClass, OperatorSeat->GetComponentTransform(), SpawnParameters);
	if (!MountedOperator)
	{
		return false;
	}

	MountedOperator->AttachToComponent(OperatorSeat, FAttachmentTransformRules::KeepRelativeTransform);
	MountedOperator->SetActorRelativeTransform(OperatorRelativeTransform);
	MountedOperator->SetActorEnableCollision(false);
	if (UCharacterMovementComponent* MovementComponent = MountedOperator->GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
	}
	return true;
}

void AChongtongCannonActor::RemoveMountedOperator()
{
	if (IsValid(MountedOperator))
	{
		MountedOperator->Destroy();
	}
	MountedOperator = nullptr;
}

void AChongtongCannonActor::HandleDeath(UHealthComponent* DeadHealthComponent, const FCombatDamageSpec& KillingDamage)
{
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
	RemoveMountedOperator();
	SetActorEnableCollision(false);
}

void AChongtongCannonActor::FireScheduledShot()
{
	TryFire();
}
