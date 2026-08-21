#include "Ongseong/OngseongEnemyWaveManager.h"

#include "Gameplay/Characters/EnemyCombatCharacter.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "TimerManager.h"

AOngseongEnemyWaveManager::AOngseongEnemyWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOngseongEnemyWaveManager::BeginPlay()
{
	Super::BeginPlay();
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
	if (!IsSpawnConfigured() || !GetWorld())
	{
		return;
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

bool AOngseongEnemyWaveManager::SpawnEnemy()
{
	if (!IsSpawnConfigured() || SpawnedEnemyCount >= FMath::Max(1, TotalEnemiesToSpawn) ||
		EnemyPool->GetActiveCount() >= FMath::Max(1, MaxActiveEnemies))
	{
		return false;
	}

	AActor* AcquiredActor = EnemyPool->AcquireActor(BuildSpawnTransform());
	AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(AcquiredActor);
	if (!Enemy)
	{
		if (AcquiredActor)
		{
			EnemyPool->ReleaseActor(AcquiredActor);
		}
		return false;
	}

	if (UHealthComponent* HealthComponent = Enemy->GetHealthComponent())
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AOngseongEnemyWaveManager::HandleEnemyDeath);
	}
	Enemy->SetObjectiveTarget(ObjectiveTarget);
	++SpawnedEnemyCount;
	if (SpawnedEnemyCount >= TotalEnemiesToSpawn) StopSpawning();
	return true;
}

bool AOngseongEnemyWaveManager::IsSpawnConfigured() const
{
	return IsValid(EnemyPool) && IsValid(ObjectiveTarget);
}

void AOngseongEnemyWaveManager::HandleEnemyDeath(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage)
{
	if (!IsValid(EnemyPool) || !IsValid(HealthComponent))
	{
		return;
	}

	if (AActor* EnemyActor = HealthComponent->GetOwner())
	{
		EnemyPool->ReleaseActor(EnemyActor);
		++DefeatedEnemyCount;
		OnWaveProgress.Broadcast(DefeatedEnemyCount, TotalEnemiesToSpawn);
		if (AreAllEnemiesDefeated())
		{
			StopSpawning();
			OnAllEnemiesDefeated.Broadcast(TotalEnemiesToSpawn);
		}
	}
}

void AOngseongEnemyWaveManager::SpawnScheduledEnemy()
{
	SpawnEnemy();
}

FTransform AOngseongEnemyWaveManager::BuildSpawnTransform()
{
	constexpr int32 FormationWidth = 3;
	const int32 FormationIndex = SpawnSequence++ % FormationWidth;
	const float CenteredIndex = static_cast<float>(FormationIndex - FormationWidth / 2);
	const FVector SpawnLocation = GetActorLocation() + GetActorRightVector() * CenteredIndex * SpawnSpacing;
	return FTransform(GetActorRotation(), SpawnLocation);
}
