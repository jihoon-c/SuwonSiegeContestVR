#include "Ongseong/OngseongEnemyWaveManager.h"

#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Characters/EnemyCombatCharacter.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/OngseongArcherCombatComponent.h"
#include "Ongseong/ChongtongCannonActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AOngseongEnemyWaveManager::AOngseongEnemyWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOngseongEnemyWaveManager::BeginPlay()
{
	Super::BeginPlay();
	if (!IsValid(ArcherEnemyPool))
	{
		TArray<AActor*> TaggedPools;
		UGameplayStatics::GetAllActorsWithTag(this, TEXT("Ongseong.ArcherPool"), TaggedPools);
		if (!TaggedPools.IsEmpty()) ArcherEnemyPool = Cast<AActorPool>(TaggedPools[0]);
	}
	if (!IsValid(ArcherProjectilePool))
	{
		TArray<AActor*> TaggedPools;
		UGameplayStatics::GetAllActorsWithTag(this, TEXT("Ongseong.ArrowPool"), TaggedPools);
		if (!TaggedPools.IsEmpty()) ArcherProjectilePool = Cast<AActorPool>(TaggedPools[0]);
	}
	if (!IsValid(ArcherPrimaryTarget))
	{
		TArray<AActor*> TaggedTargets;
		UGameplayStatics::GetAllActorsWithTag(this, TEXT("Ongseong.ArcherTarget"), TaggedTargets);
		if (!TaggedTargets.IsEmpty()) ArcherPrimaryTarget = TaggedTargets[0];
		else
		{
			for (TActorIterator<AChongtongCannonActor> It(GetWorld()); It; ++It) { ArcherPrimaryTarget = *It; break; }
		}
	}
	if (bAutoStart)
	{
		StartSpawning();
	}
}

void AOngseongEnemyWaveManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void AOngseongEnemyWaveManager::StartSpawning()
{
	TotalEnemiesToSpawn = FMath::Max(0, SwordsmenToSpawn) + FMath::Max(0, ArchersToSpawn);
	if (!IsSpawnConfigured() || !GetWorld())
	{
		return;
	}

	if (!bWaveStarted)
	{
		bWaveStarted = true;
		OnWaveStarted.Broadcast(TotalEnemiesToSpawn);
	}

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AOngseongEnemyWaveManager::SpawnScheduledEnemy,
		FMath::Max(0.1f, SpawnInterval),
		true,
		FMath::Max(0.0f, InitialDelay));
}

void AOngseongEnemyWaveManager::StopSpawning()
{
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void AOngseongEnemyWaveManager::RetreatAllEnemies(const FVector RetreatLocation)
{
	StopSpawning();
	bRetreating = true;
	ActiveEnemies.RemoveAll([](const AEnemyCombatCharacter* Enemy) { return !IsValid(Enemy); });
	if (ActiveEnemies.IsEmpty())
	{
		bRetreating = false;
		OnAllEnemiesRetreated.Broadcast();
		return;
	}

	for (AEnemyCombatCharacter* Enemy : ActiveEnemies)
	{
		if (UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>()) ArcherCombat->DeactivateCombat();
		if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
		{
			Controller->OnMoveTargetReached.AddUniqueDynamic(this, &AOngseongEnemyWaveManager::HandleRetreatTargetReached);
		}
		Enemy->SetRetreatTargetLocation(RetreatLocation);
	}
}

void AOngseongEnemyWaveManager::ReleaseAllEnemies()
{
	StopSpawning();
	const TArray<TObjectPtr<AEnemyCombatCharacter>> EnemiesToRelease = ActiveEnemies;
	ActiveEnemies.Reset();
	bRetreating = false;
	for (AEnemyCombatCharacter* Enemy : EnemiesToRelease)
	{
		if (!IsValid(Enemy)) continue;
		if (UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>()) ArcherCombat->DeactivateCombat();
		if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
		{
			Controller->OnMoveTargetReached.RemoveDynamic(this, &AOngseongEnemyWaveManager::HandleRetreatTargetReached);
		}
		if (TObjectPtr<AActorPool>* Pool = EnemyPoolsByActor.Find(Enemy); Pool && IsValid(*Pool))
		{
			(*Pool)->ReleaseActor(Enemy);
		}
	}
	EnemyPoolsByActor.Reset();
}

void AOngseongEnemyWaveManager::ResetWave()
{
	ReleaseAllEnemies();
	SpawnSequence = 0;
	SpawnedEnemyCount = 0;
	SpawnedSwordsmanCount = 0;
	SpawnedArcherCount = 0;
	DefeatedEnemyCount = 0;
	bWaveStarted = false;
}

bool AOngseongEnemyWaveManager::SpawnEnemy()
{
	const EOngseongEnemyType EnemyType = ChooseNextEnemyType();
	AActorPool* SpawnPool = GetPoolForEnemyType(EnemyType);
	if (!IsSpawnConfigured() || SpawnedEnemyCount >= TotalEnemiesToSpawn || !IsValid(SpawnPool) ||
		ActiveEnemies.Num() >= FMath::Max(1, MaxActiveEnemies))
	{
		return false;
	}

	AActor* AcquiredActor = SpawnPool->AcquireActor(BuildSpawnTransform());
	AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(AcquiredActor);
	if (!Enemy)
	{
		if (AcquiredActor)
		{
			SpawnPool->ReleaseActor(AcquiredActor);
		}
		return false;
	}

	if (UHealthComponent* HealthComponent = Enemy->GetHealthComponent())
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AOngseongEnemyWaveManager::HandleEnemyDeath);
	}
	Enemy->SetObjectiveTarget(ObjectiveTarget);
	if (EnemyType == EOngseongEnemyType::Archer)
	{
		UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>();
		if (!ArcherCombat)
		{
			ArcherCombat = NewObject<UOngseongArcherCombatComponent>(Enemy, TEXT("OngseongArcherCombat"));
			ArcherCombat->RegisterComponent();
		}
		ArcherCombat->ConfigureCombat(ArcherPrimaryTarget, ObjectiveTarget, ArcherProjectilePool);
		ArcherCombat->ApplyTuning(ArcherHitChance, ArcherRange, ArcherFireInterval, ArcherMissRadius, ArcherDamage, ArcherProjectileSpeed);
		ArcherCombat->ActivateCombat();
		if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
		{
			AActor* ArcherTarget = ArcherCombat->GetCurrentTarget();
			Controller->SetCombatTarget(ArcherTarget);
			Controller->MoveToCombatActor(ArcherTarget, ArcherRange * 0.9f);
		}
	}
	ActiveEnemies.AddUnique(Enemy);
	EnemyPoolsByActor.Add(Enemy, SpawnPool);
	if (EnemyType == EOngseongEnemyType::Archer) ++SpawnedArcherCount;
	else ++SpawnedSwordsmanCount;
	++SpawnedEnemyCount;
	OnEnemySpawned.Broadcast(Enemy, SpawnedEnemyCount, TotalEnemiesToSpawn);
	if (SpawnedEnemyCount >= TotalEnemiesToSpawn) StopSpawning();
	return true;
}

bool AOngseongEnemyWaveManager::IsSpawnConfigured() const
{
	return IsValid(EnemyPool) && IsValid(ObjectiveTarget) && (SwordsmenToSpawn > 0 || ArchersToSpawn > 0);
}

void AOngseongEnemyWaveManager::HandleEnemyDeath(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage)
{
	if (!IsValid(EnemyPool) || !IsValid(HealthComponent))
	{
		return;
	}

	if (AActor* EnemyActor = HealthComponent->GetOwner())
	{
		if (AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(EnemyActor))
		{
			if (UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>()) ArcherCombat->DeactivateCombat();
			ActiveEnemies.Remove(Enemy);
			if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
			{
				Controller->OnMoveTargetReached.RemoveDynamic(this, &AOngseongEnemyWaveManager::HandleRetreatTargetReached);
			}
		}
		AActorPool* ReleasePool = EnemyPool;
		if (AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(EnemyActor))
		{
			if (TObjectPtr<AActorPool>* FoundPool = EnemyPoolsByActor.Find(Enemy)) ReleasePool = *FoundPool;
			EnemyPoolsByActor.Remove(Enemy);
		}
		if (IsValid(ReleasePool)) ReleasePool->ReleaseActor(EnemyActor);
		++DefeatedEnemyCount;
		OnWaveProgress.Broadcast(DefeatedEnemyCount, TotalEnemiesToSpawn);
		if (AreAllEnemiesDefeated())
		{
			StopSpawning();
			OnAllEnemiesDefeated.Broadcast(TotalEnemiesToSpawn);
		}
		if (bRetreating && ActiveEnemies.IsEmpty())
		{
			bRetreating = false;
			OnAllEnemiesRetreated.Broadcast();
		}
	}
}

void AOngseongEnemyWaveManager::HandleRetreatTargetReached(APawn* EnemyPawn)
{
	if (!bRetreating || !IsValid(EnemyPawn)) return;
	if (AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(EnemyPawn))
	{
		if (UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>()) ArcherCombat->DeactivateCombat();
		if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
		{
			Controller->OnMoveTargetReached.RemoveDynamic(this, &AOngseongEnemyWaveManager::HandleRetreatTargetReached);
		}
		ActiveEnemies.Remove(Enemy);
		AActorPool* ReleasePool = EnemyPool;
		if (TObjectPtr<AActorPool>* FoundPool = EnemyPoolsByActor.Find(Enemy)) ReleasePool = *FoundPool;
		EnemyPoolsByActor.Remove(Enemy);
		if (IsValid(ReleasePool)) ReleasePool->ReleaseActor(Enemy);
	}
	if (ActiveEnemies.IsEmpty())
	{
		bRetreating = false;
		OnAllEnemiesRetreated.Broadcast();
	}
}

void AOngseongEnemyWaveManager::SpawnScheduledEnemy()
{
	SpawnEnemy();
}

EOngseongEnemyType AOngseongEnemyWaveManager::ChooseNextEnemyType() const
{
	const bool bCanSpawnSwordsman = SpawnedSwordsmanCount < FMath::Max(0, SwordsmenToSpawn);
	const bool bCanSpawnArcher = SpawnedArcherCount < FMath::Max(0, ArchersToSpawn);
	if (!bCanSpawnSwordsman) return EOngseongEnemyType::Archer;
	if (!bCanSpawnArcher) return EOngseongEnemyType::Swordsman;
	return SpawnedEnemyCount % 2 == 0 ? EOngseongEnemyType::Swordsman : EOngseongEnemyType::Archer;
}

AActorPool* AOngseongEnemyWaveManager::GetPoolForEnemyType(const EOngseongEnemyType EnemyType) const
{
	return EnemyType == EOngseongEnemyType::Archer && IsValid(ArcherEnemyPool) ? ArcherEnemyPool : EnemyPool;
}

FTransform AOngseongEnemyWaveManager::BuildSpawnTransform()
{
	constexpr int32 FormationWidth = 3;
	const int32 FormationIndex = SpawnSequence++ % FormationWidth;
	const float CenteredIndex = static_cast<float>(FormationIndex - FormationWidth / 2);
	const FVector SpawnLocation = GetActorLocation() + GetActorRightVector() * CenteredIndex * SpawnSpacing;
	return FTransform(GetActorRotation(), SpawnLocation);
}
