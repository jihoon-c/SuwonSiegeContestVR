#include "Ongseong/OngseongEnemyWaveManager.h"

#include "GF_OngseongCrossbow.h"

#include "Components/SceneComponent.h"
#include "Gameplay/AI/CombatAIController.h"
#include "Gameplay/Characters/EnemyCombatCharacter.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/ChongtongCannonActor.h"
#include "Ongseong/OngseongArcherCombatComponent.h"
#include "Ongseong/OngseongSpawnPointActor.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AOngseongEnemyWaveManager::AOngseongEnemyWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
	// Spawn transforms are built from this actor's transform. Without a root component the actor
	// cannot be moved in the editor at all, which pins every spawn to the world origin.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AOngseongEnemyWaveManager::BeginPlay()
{
	Super::BeginPlay();
	ResolveSpawnPoints();
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
	if (!IsValid(ArcherPlayerTarget))
	{
		ArcherPlayerTarget = UGameplayStatics::GetPlayerPawn(this, 0);
	}
	if (bResizePoolsToSlotCounts)
	{
		ResizePoolsToSlotCounts();
	}
	if (bAutoStart)
	{
		StartSpawning();
	}
}

void AOngseongEnemyWaveManager::ResizePoolsToSlotCounts()
{
	const int32 Headroom = FMath::Max(0, EnemyPoolHeadroom);
	const bool bSeparateArcherPool = IsValid(ArcherEnemyPool) && ArcherEnemyPool != EnemyPool;
	if (IsValid(EnemyPool))
	{
		// Without a dedicated archer pool this one has to cover both types.
		const int32 Needed = bSeparateArcherPool
			? GetSwordsmanSlots() + Headroom
			: GetMaxConcurrentEnemies() + Headroom;
		EnemyPool->EnsurePoolSize(Needed);
	}
	if (bSeparateArcherPool)
	{
		ArcherEnemyPool->EnsurePoolSize(GetArcherSlots() + Headroom);
	}
	if (IsValid(ArcherProjectilePool))
	{
		ArcherProjectilePool->EnsurePoolSize(GetArcherSlots() * FMath::Max(1, ArrowsPerArcher));
	}
}

void AOngseongEnemyWaveManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void AOngseongEnemyWaveManager::StartSpawning()
{
	ResolveSpawnPoints();
	if (!IsSpawnConfigured() || !GetWorld())
	{
		return;
	}

	if (!bSpawningActive)
	{
		bSpawningActive = true;
		OnSpawningStarted.Broadcast(GetMaxConcurrentEnemies());
		UE_LOG(LogOngseong, Display, TEXT("Enemy spawning started at %s: %d swordsmen + %d archers, respawn %.1fs (+/-%.1f)."),
			*GetActorLocation().ToCompactString(), GetSwordsmanSlots(), GetArcherSlots(), RespawnDelay, RespawnDelayJitter);
		if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
		{
			FNavLocation Projected;
			const bool bOnNavMesh = NavigationSystem->ProjectPointToNavigation(GetActorLocation(), Projected, FVector(300.0f));
			UE_LOG(LogOngseong, Display, TEXT("Spawn point %s the navmesh%s. Enemies cannot advance without one."),
				bOnNavMesh ? TEXT("is on") : TEXT("is NOT on"),
				bOnNavMesh ? *FString::Printf(TEXT(" (projected to %s)"), *Projected.Location.ToCompactString()) : TEXT(""));
		}
		else
		{
			UE_LOG(LogOngseong, Warning, TEXT("No navigation system in this world; enemies cannot path."));
		}
	}
	bRetreating = false;

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AOngseongEnemyWaveManager::TickPopulationFill,
		FMath::Max(0.05f, InitialSpawnInterval),
		true,
		FMath::Max(0.0f, InitialDelay));

	// Archers that found every emplacement full keep watching for a place to open up.
	GetWorldTimerManager().ClearTimer(ArcherSlotRetryHandle);
	GetWorldTimerManager().SetTimer(
		ArcherSlotRetryHandle,
		this,
		&AOngseongEnemyWaveManager::RetryArcherSlotAssignments,
		FMath::Max(0.5f, ArcherSlotRetryInterval),
		true);

	// Swordsmen no longer escort the ram. They pick reachable points inside the fortress and
	// keep moving between them, so the courtyard stays populated wherever the player looks.
	GetWorldTimerManager().ClearTimer(SwordsmanBehaviorTimerHandle);
	GetWorldTimerManager().SetTimer(
		SwordsmanBehaviorTimerHandle,
		this,
		&AOngseongEnemyWaveManager::UpdateSwordsmanRoamBehavior,
		FMath::Max(0.1f, SwordsmanBehaviorUpdateInterval),
		true,
		0.0f);
}

void AOngseongEnemyWaveManager::StopSpawning()
{
	bSpawningActive = false;
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorldTimerManager().ClearTimer(ArcherSlotRetryHandle);
		GetWorldTimerManager().ClearTimer(SwordsmanBehaviorTimerHandle);
	}
	ClearPendingRespawns();
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
		ReleaseArcherAttackPosition(Enemy);
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
		DetachEnemy(Enemy);
		if (TObjectPtr<AActorPool>* Pool = EnemyPoolsByActor.Find(Enemy); Pool && IsValid(*Pool))
		{
			(*Pool)->ReleaseActor(Enemy);
		}
	}
	EnemyPoolsByActor.Reset();
	EnemyTypesByActor.Reset();
	ArcherAttackPositionsByEnemy.Reset();
	SwordsmanMoveDestinations.Reset();
	SwordsmanNextRoamTimes.Reset();
	OnPopulationChanged.Broadcast(0, GetMaxConcurrentEnemies());
}

void AOngseongEnemyWaveManager::ResetWave()
{
	ReleaseAllEnemies();
	SpawnSequence = 0;
	TotalSpawnedEnemies = 0;
	TotalDefeatedEnemies = 0;
	bSpawningActive = false;
}

bool AOngseongEnemyWaveManager::SpawnEnemy()
{
	EOngseongEnemyType EnemyType = EOngseongEnemyType::Swordsman;
	if (!ChooseNextEnemyType(EnemyType))
	{
		return false;
	}
	return SpawnEnemyOfType(EnemyType);
}

bool AOngseongEnemyWaveManager::SpawnEnemyOfType(const EOngseongEnemyType EnemyType, const bool bUseRespawnPoint)
{
	AActorPool* SpawnPool = GetPoolForEnemyType(EnemyType);
	if (!IsSpawnConfigured() || !IsValid(SpawnPool) || ActiveEnemies.Num() >= GetMaxConcurrentEnemies())
	{
		return false;
	}

	AActor* AcquiredActor = SpawnPool->AcquireActor(BuildSpawnTransform(bUseRespawnPoint));
	AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(AcquiredActor);
	if (!Enemy)
	{
		if (AcquiredActor)
		{
			SpawnPool->ReleaseActor(AcquiredActor);
		}
		return false;
	}

	Enemy->OnDeathPresentationFinished.AddUniqueDynamic(this, &AOngseongEnemyWaveManager::HandleEnemyDeathPresentationFinished);
	Enemy->SetObjectiveTarget(ObjectiveTarget);
	ActiveEnemies.AddUnique(Enemy);
	EnemyPoolsByActor.Add(Enemy, SpawnPool);
	EnemyTypesByActor.Add(Enemy, EnemyType);
	if (EnemyType == EOngseongEnemyType::Archer)
	{
		UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>();
		if (!ArcherCombat)
		{
			ArcherCombat = NewObject<UOngseongArcherCombatComponent>(Enemy, TEXT("OngseongArcherCombat"));
			ArcherCombat->RegisterComponent();
		}
		ArcherCombat->ApplyTuning(ArcherHitChance, ArcherRange, ArcherFireInterval, ArcherMissRadius, ArcherDamage, ArcherProjectileSpeed);
		ArcherCombat->ActivateCombat();
		ApplyArcherEngagement(Enemy);
	}
	else
	{
		// SetObjectiveTarget above issued a MoveToCombatActor toward the gate. Cancel it before
		// roaming, or the first roam destination competes with a march on the objective.
		if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
		{
			Controller->StopCombatMovement();
		}
		ApplySwordsmanRoamBehavior(Enemy);
	}
	++TotalSpawnedEnemies;
	OnEnemySpawned.Broadcast(Enemy, ActiveEnemies.Num(), GetMaxConcurrentEnemies());
	OnPopulationChanged.Broadcast(ActiveEnemies.Num(), GetMaxConcurrentEnemies());
	UE_LOG(LogOngseong, Verbose, TEXT("Spawned %s (%s) at %s. Living %d/%d."),
		*Enemy->GetName(),
		EnemyType == EOngseongEnemyType::Archer ? TEXT("archer") : TEXT("swordsman"),
		*Enemy->GetActorLocation().ToCompactString(),
		ActiveEnemies.Num(),
		GetMaxConcurrentEnemies());
	return true;
}

void AOngseongEnemyWaveManager::TickPopulationFill()
{
	if (!bSpawningActive || bRetreating)
	{
		return;
	}

	if (!HasPopulationDeficit())
	{
		// The population is full. Replacements are driven by respawn timers from here on.
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		UE_LOG(LogOngseong, Display, TEXT("Ongseong population filled: %d living enemies."), ActiveEnemies.Num());
		return;
	}

	SpawnEnemy();
}

void AOngseongEnemyWaveManager::ScheduleRespawn(const EOngseongEnemyType EnemyType)
{
	if (!bMaintainPopulation || !bSpawningActive || bRetreating || !GetWorld())
	{
		return;
	}

	PruneFinishedRespawnTimers();
	const float Jitter = FMath::Max(0.0f, RespawnDelayJitter);
	const float Delay = FMath::Max(0.05f, FMath::Max(0.0f, RespawnDelay) + FMath::FRandRange(-Jitter, Jitter));
	FTimerHandle RespawnHandle;
	GetWorldTimerManager().SetTimer(
		RespawnHandle,
		FTimerDelegate::CreateUObject(this, &AOngseongEnemyWaveManager::HandleRespawnTimer, EnemyType),
		Delay,
		false);
	RespawnTimerHandles.Add(RespawnHandle);
}

void AOngseongEnemyWaveManager::HandleRespawnTimer(const EOngseongEnemyType EnemyType)
{
	PruneFinishedRespawnTimers();
	if (!bSpawningActive || bRetreating)
	{
		return;
	}

	if (GetLivingEnemyCountOfType(EnemyType) >= GetSlotsForType(EnemyType) || ActiveEnemies.Num() >= GetMaxConcurrentEnemies())
	{
		return;
	}

	if (!SpawnEnemyOfType(EnemyType, true))
	{
		// The pool was exhausted or misconfigured. Try again on the next respawn cycle instead of
		// silently shrinking the population.
		ScheduleRespawn(EnemyType);
	}
}

void AOngseongEnemyWaveManager::ClearPendingRespawns()
{
	if (UWorld* World = GetWorld())
	{
		for (FTimerHandle& Handle : RespawnTimerHandles)
		{
			World->GetTimerManager().ClearTimer(Handle);
		}
	}
	RespawnTimerHandles.Reset();
}

void AOngseongEnemyWaveManager::PruneFinishedRespawnTimers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	FTimerManager& TimerManager = World->GetTimerManager();
	RespawnTimerHandles.RemoveAll([&TimerManager](const FTimerHandle& Handle)
	{
		return !TimerManager.IsTimerActive(Handle);
	});
}

bool AOngseongEnemyWaveManager::IsSpawnConfigured() const
{
	return IsValid(EnemyPool) && IsValid(ObjectiveTarget) && (GetSwordsmanSlots() > 0 || GetArcherSlots() > 0);
}

int32 AOngseongEnemyWaveManager::GetLivingEnemyCountOfType(const EOngseongEnemyType EnemyType) const
{
	int32 Count = 0;
	for (const TPair<TObjectPtr<AEnemyCombatCharacter>, EOngseongEnemyType>& Pair : EnemyTypesByActor)
	{
		if (Pair.Value == EnemyType && IsValid(Pair.Key))
		{
			++Count;
		}
	}
	return Count;
}

int32 AOngseongEnemyWaveManager::GetSlotsForType(const EOngseongEnemyType EnemyType) const
{
	return EnemyType == EOngseongEnemyType::Archer ? GetArcherSlots() : GetSwordsmanSlots();
}

bool AOngseongEnemyWaveManager::HasPopulationDeficit() const
{
	if (ActiveEnemies.Num() >= GetMaxConcurrentEnemies())
	{
		return false;
	}
	EOngseongEnemyType UnusedType = EOngseongEnemyType::Swordsman;
	return ChooseNextEnemyType(UnusedType);
}

bool AOngseongEnemyWaveManager::ChooseNextEnemyType(EOngseongEnemyType& OutEnemyType) const
{
	const int32 SwordsmanDeficit = GetSwordsmanSlots() - GetLivingEnemyCountOfType(EOngseongEnemyType::Swordsman);
	const int32 ArcherDeficit = GetArcherSlots() - GetLivingEnemyCountOfType(EOngseongEnemyType::Archer);
	if (SwordsmanDeficit <= 0 && ArcherDeficit <= 0)
	{
		return false;
	}

	OutEnemyType = ArcherDeficit > SwordsmanDeficit ? EOngseongEnemyType::Archer : EOngseongEnemyType::Swordsman;
	return true;
}

void AOngseongEnemyWaveManager::HandleEnemyDeath(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage)
{
	if (!IsValid(HealthComponent))
	{
		return;
	}

	AActor* EnemyActor = HealthComponent->GetOwner();
	AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(EnemyActor);
	if (!Enemy)
	{
		return;
	}

	EOngseongEnemyType DefeatedType = EOngseongEnemyType::Swordsman;
	if (const EOngseongEnemyType* FoundType = EnemyTypesByActor.Find(Enemy))
	{
		DefeatedType = *FoundType;
	}

	AActorPool* ReleasePool = EnemyPool;
	if (TObjectPtr<AActorPool>* FoundPool = EnemyPoolsByActor.Find(Enemy))
	{
		ReleasePool = *FoundPool;
	}

	DetachEnemy(Enemy);
	ActiveEnemies.Remove(Enemy);
	EnemyPoolsByActor.Remove(Enemy);
	EnemyTypesByActor.Remove(Enemy);
	if (IsValid(ReleasePool))
	{
		ReleasePool->ReleaseActor(Enemy);
	}

	++TotalDefeatedEnemies;
	UE_LOG(LogOngseong, Verbose, TEXT("Enemy defeated (total %d). Living %d/%d. Fell %.0f cm from the objective."),
		TotalDefeatedEnemies, ActiveEnemies.Num(), GetMaxConcurrentEnemies(),
		IsValid(ObjectiveTarget) ? FVector::Dist(EnemyActor->GetActorLocation(), ObjectiveTarget->GetActorLocation()) : -1.0f);
	OnEnemyDefeated.Broadcast(TotalDefeatedEnemies, ActiveEnemies.Num());
	OnPopulationChanged.Broadcast(ActiveEnemies.Num(), GetMaxConcurrentEnemies());

	if (bRetreating)
	{
		if (ActiveEnemies.IsEmpty())
		{
			bRetreating = false;
			OnAllEnemiesRetreated.Broadcast();
		}
		return;
	}

	ScheduleRespawn(DefeatedType);
}

void AOngseongEnemyWaveManager::HandleEnemyDeathPresentationFinished(AEnemyCombatCharacter* Enemy)
{
	if (!IsValid(Enemy) || !Enemy->GetHealthComponent() || !Enemy->GetHealthComponent()->IsDead())
	{
		return;
	}
	HandleEnemyDeath(Enemy->GetHealthComponent(), FCombatDamageSpec());
}

void AOngseongEnemyWaveManager::HandleRetreatTargetReached(APawn* EnemyPawn)
{
	if (!bRetreating || !IsValid(EnemyPawn)) return;
	if (AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(EnemyPawn))
	{
		AActorPool* ReleasePool = EnemyPool;
		if (TObjectPtr<AActorPool>* FoundPool = EnemyPoolsByActor.Find(Enemy)) ReleasePool = *FoundPool;
		DetachEnemy(Enemy);
		ActiveEnemies.Remove(Enemy);
		EnemyPoolsByActor.Remove(Enemy);
		EnemyTypesByActor.Remove(Enemy);
		SwordsmanMoveDestinations.Remove(Enemy);
		SwordsmanNextRoamTimes.Remove(Enemy);
		if (IsValid(ReleasePool)) ReleasePool->ReleaseActor(Enemy);
		OnPopulationChanged.Broadcast(ActiveEnemies.Num(), GetMaxConcurrentEnemies());
	}
	if (ActiveEnemies.IsEmpty())
	{
		bRetreating = false;
		OnAllEnemiesRetreated.Broadcast();
	}
}

void AOngseongEnemyWaveManager::DetachEnemy(AEnemyCombatCharacter* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}
	if (UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>())
	{
		ArcherCombat->DeactivateCombat();
	}
	ReleaseArcherAttackPosition(Enemy);
	SwordsmanMoveDestinations.Remove(Enemy);
	SwordsmanNextRoamTimes.Remove(Enemy);
	if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
	{
		Controller->OnMoveTargetReached.RemoveDynamic(this, &AOngseongEnemyWaveManager::HandleRetreatTargetReached);
	}
	Enemy->OnDeathPresentationFinished.RemoveDynamic(this, &AOngseongEnemyWaveManager::HandleEnemyDeathPresentationFinished);
}

void AOngseongEnemyWaveManager::SetArcherEscortTarget(AActor* NewEscortTarget)
{
	// Only surplus archers without an attack slot use this. Swordsmen roam independently.
	ArcherEscortTarget = NewEscortTarget;
}

ATargetPoint* AOngseongEnemyWaveManager::ReserveAttackPositionForArcher(
	AEnemyCombatCharacter* Archer,
	AChongtongCannonActor*& OutCannon)
{
	OutCannon = nullptr;
	if (!IsValid(Archer) || !GetWorld())
	{
		return nullptr;
	}

	ReleaseArcherAttackPosition(Archer);
	TSet<ATargetPoint*> OccupiedPositions;
	for (const TPair<TObjectPtr<AEnemyCombatCharacter>, TObjectPtr<ATargetPoint>>& Pair : ArcherAttackPositionsByEnemy)
	{
		if (IsValid(Pair.Key) && IsValid(Pair.Value))
		{
			OccupiedPositions.Add(Pair.Value);
		}
	}
	TArray<ATargetPoint*> Candidates;
	for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
	{
		ATargetPoint* Position = *It;
		if (IsValid(Position)
			&& Position->ActorHasTag(TEXT("Ongseong.ArcherAttackPosition"))
			&& !OccupiedPositions.Contains(Position))
		{
			Candidates.Add(Position);
		}
	}
	const FVector ArcherLocation = Archer->GetActorLocation();
	Candidates.Sort([&ArcherLocation](const ATargetPoint& A, const ATargetPoint& B)
	{
		return FVector::DistSquared(A.GetActorLocation(), ArcherLocation)
			< FVector::DistSquared(B.GetActorLocation(), ArcherLocation);
	});

	auto FindNearestLivingCannon = [this](const ATargetPoint* Position)
	{
		AChongtongCannonActor* NearestCannon = nullptr;
		float NearestDistanceSquared = TNumericLimits<float>::Max();
		for (TActorIterator<AChongtongCannonActor> It(GetWorld()); It; ++It)
		{
			AChongtongCannonActor* CandidateCannon = *It;
			const UHealthComponent* Health = CandidateCannon
				? CandidateCannon->FindComponentByClass<UHealthComponent>() : nullptr;
			if (!IsValid(CandidateCannon) || (Health && Health->IsDead()))
			{
				continue;
			}
			const float DistanceSquared = FVector::DistSquared(
				Position->GetActorLocation(), CandidateCannon->GetActorLocation());
			if (DistanceSquared < NearestDistanceSquared)
			{
				NearestCannon = CandidateCannon;
				NearestDistanceSquared = DistanceSquared;
			}
		}
		return NearestCannon;
	};

	for (ATargetPoint* Position : Candidates)
	{
		AChongtongCannonActor* NearestCannon = FindNearestLivingCannon(Position);
		if (!IsValid(NearestCannon))
		{
			continue;
		}
		int32 ReservedSlotsForCannon = 0;
		for (const TPair<TObjectPtr<AEnemyCombatCharacter>, TObjectPtr<ATargetPoint>>& Pair : ArcherAttackPositionsByEnemy)
		{
			if (IsValid(Pair.Key) && IsValid(Pair.Value) && FindNearestLivingCannon(Pair.Value) == NearestCannon)
			{
				++ReservedSlotsForCannon;
			}
		}
		if (ReservedSlotsForCannon >= FMath::Max(1, ArcherAttackSlotsPerCannon))
		{
			continue;
		}
		OutCannon = NearestCannon;
		ArcherAttackPositionsByEnemy.Add(Archer, Position);
		UE_LOG(LogOngseong, Verbose, TEXT("Archer position %s reserved by %s for %s."),
			*Position->GetName(), *Archer->GetName(), *NearestCannon->GetName());
		return Position;
	}
	return nullptr;
}

void AOngseongEnemyWaveManager::ReleaseArcherAttackPosition(AActor* Archer)
{
	AEnemyCombatCharacter* Enemy = Cast<AEnemyCombatCharacter>(Archer);
	if (!Enemy)
	{
		return;
	}
	if (TObjectPtr<ATargetPoint>* Position = ArcherAttackPositionsByEnemy.Find(Enemy))
	{
		if (IsValid(*Position))
		{
			UE_LOG(LogOngseong, Verbose, TEXT("Archer position %s released by %s."),
				*(*Position)->GetName(), *Archer->GetName());
		}
		ArcherAttackPositionsByEnemy.Remove(Enemy);
	}
}

void AOngseongEnemyWaveManager::ApplyArcherEngagement(AEnemyCombatCharacter* Archer)
{
	UOngseongArcherCombatComponent* ArcherCombat = IsValid(Archer)
		? Archer->FindComponentByClass<UOngseongArcherCombatComponent>() : nullptr;
	if (!ArcherCombat)
	{
		return;
	}

	AChongtongCannonActor* Cannon = nullptr;
	ATargetPoint* AttackPosition = ReserveAttackPositionForArcher(Archer, Cannon);
	if (!IsValid(ArcherPlayerTarget))
	{
		ArcherPlayerTarget = UGameplayStatics::GetPlayerPawn(this, 0);
	}
	// When every authored position is occupied, surplus archers continue toward the ram.
	AActor* MoveTarget = AttackPosition ? static_cast<AActor*>(AttackPosition)
		: (IsValid(ArcherEscortTarget) ? ArcherEscortTarget.Get() : ObjectiveTarget.Get());
	// Cannon is still reserved for positioning (see ReserveAttackPositionForArcher above), but archers
	// no longer aim at it -- they always shoot at the player (PlayerPhone anchor once that lands).
	ArcherCombat->ConfigureCombat(ArcherPlayerTarget, ArcherPlayerTarget, ArcherProjectilePool);
	ArcherCombat->SetReservedCannon(Cannon);

	if (ACombatAIController* Controller = Cast<ACombatAIController>(Archer->GetController()))
	{
		// The Behavior Tree walks to TargetActor, so this is the move destination, not the aim point.
		Controller->SetCombatTarget(MoveTarget);
		const float ApproachRadius = ArcherApproachRadius;
		const bool bMoveAccepted = Controller->MoveToCombatActor(MoveTarget, ApproachRadius);
		UE_LOG(LogOngseong, VeryVerbose, TEXT("%s archer move request to %s accepted=%d radius=%.0f."),
			*Archer->GetName(), *GetNameSafe(MoveTarget), bMoveAccepted ? 1 : 0, ApproachRadius);
	}
	UE_LOG(LogOngseong, Verbose, TEXT("%s engages %s (escorting=%d)."),
		*Archer->GetName(), *GetNameSafe(MoveTarget), AttackPosition ? 0 : 1);
}

void AOngseongEnemyWaveManager::RetryArcherSlotAssignments()
{
	for (AEnemyCombatCharacter* Enemy : ActiveEnemies)
	{
		if (!IsValid(Enemy))
		{
			continue;
		}
		UOngseongArcherCombatComponent* ArcherCombat = Enemy->FindComponentByClass<UOngseongArcherCombatComponent>();
		// Only the archers still without an emplacement retry; the engaged ones keep their slot.
		if (ArcherCombat && !IsValid(ArcherCombat->GetReservedCannon()))
		{
			ApplyArcherEngagement(Enemy);
		}
	}
}

AActorPool* AOngseongEnemyWaveManager::GetPoolForEnemyType(const EOngseongEnemyType EnemyType) const
{
	return EnemyType == EOngseongEnemyType::Archer && IsValid(ArcherEnemyPool) ? ArcherEnemyPool : EnemyPool;
}

void AOngseongEnemyWaveManager::ResolveSpawnPoints()
{
	if (!GetWorld() || (IsValid(InitialSpawnPoint) && IsValid(SoldierRespawnPoint)))
	{
		return;
	}
	for (TActorIterator<AOngseongSpawnPointActor> It(GetWorld()); It; ++It)
	{
		AOngseongSpawnPointActor* Point = *It;
		if (!IsValid(InitialSpawnPoint) && Point->MatchesRole(EOngseongSpawnPointRole::EnemyInitial))
		{
			InitialSpawnPoint = Point;
		}
		else if (!IsValid(SoldierRespawnPoint) && Point->MatchesRole(EOngseongSpawnPointRole::SoldierRespawn))
		{
			SoldierRespawnPoint = Point;
		}
	}
}

FVector AOngseongEnemyWaveManager::ResolveSwordsmanRoamCentre() const
{
	if (IsValid(SwordsmanRoamAnchor))
	{
		return SwordsmanRoamAnchor->GetActorLocation();
	}
	TArray<AActor*> TaggedAnchors;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("Ongseong.SwordsmanRoamAnchor"), TaggedAnchors);
	if (!TaggedAnchors.IsEmpty() && IsValid(TaggedAnchors[0]))
	{
		return TaggedAnchors[0]->GetActorLocation();
	}

	// The authored archer firing positions are the one set of points guaranteed to sit on
	// navigable ground inside the walls, so their centre is a safe courtyard anchor.
	if (GetWorld())
	{
		FVector Sum = FVector::ZeroVector;
		int32 Count = 0;
		for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
		{
			const ATargetPoint* Position = *It;
			if (IsValid(Position) && Position->ActorHasTag(TEXT("Ongseong.ArcherAttackPosition")))
			{
				Sum += Position->GetActorLocation();
				++Count;
			}
		}
		if (Count > 0)
		{
			return Sum / static_cast<float>(Count);
		}
	}
	// Never ObjectiveTarget: that is the gate the ram is battering, which is exactly the spot
	// the swordsmen must stop piling into.
	return GetActorLocation();
}

bool AOngseongEnemyWaveManager::BuildSwordsmanRoamDestination(
	const AEnemyCombatCharacter* Swordsman,
	FVector& OutDestination) const
{
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavigationSystem || !IsValid(Swordsman))
	{
		return false;
	}
	const FVector AnchorLocation = ResolveSwordsmanRoamCentre();
	const float Radius = FMath::Max(100.0f, SwordsmanRoamRadius);

	// Reachable, not merely navigable: a point across a wall would leave the soldier
	// walking into geometry until the next roam interval fires.
	FNavLocation RoamPoint;
	if (NavigationSystem->GetRandomReachablePointInRadius(AnchorLocation, Radius, RoamPoint))
	{
		OutDestination = RoamPoint.Location;
		return true;
	}
	// Fall back to a point reachable from the soldier itself when the anchor is off-mesh.
	if (NavigationSystem->GetRandomReachablePointInRadius(Swordsman->GetActorLocation(), Radius, RoamPoint))
	{
		OutDestination = RoamPoint.Location;
		return true;
	}
	return false;
}

void AOngseongEnemyWaveManager::CommandSwordsmanMove(
	AEnemyCombatCharacter* Swordsman,
	const FVector& Destination,
	const float Speed)
{
	if (!IsValid(Swordsman))
	{
		return;
	}
	// Speed <= 0 means "leave the character alone", which is what archers get. Overriding
	// MaxWalkSpeed here is what made swordsmen visibly slower than the archers beside them.
	if (Speed > 0.0f)
	{
		if (UCharacterMovementComponent* Movement = Swordsman->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = Speed;
		}
	}
	if (ACombatAIController* Controller = Cast<ACombatAIController>(Swordsman->GetController()))
	{
		const bool bMoveAccepted = Controller->MoveToCombatLocation(Destination, SwordsmanMoveAcceptanceRadius);
		SwordsmanMoveDestinations.Add(Swordsman, Destination);
		UE_LOG(LogOngseong, VeryVerbose, TEXT("%s swordsman roam request accepted=%d speed=%.0f destination=%s."),
			*Swordsman->GetName(), bMoveAccepted ? 1 : 0, Speed, *Destination.ToCompactString());
	}
}

void AOngseongEnemyWaveManager::ApplySwordsmanRoamBehavior(AEnemyCombatCharacter* Swordsman)
{
	if (!IsValid(Swordsman) || !GetWorld())
	{
		return;
	}
	const float Now = GetWorld()->GetTimeSeconds();
	const float* NextRoamTime = SwordsmanNextRoamTimes.Find(Swordsman);
	const FVector* CurrentDestination = SwordsmanMoveDestinations.Find(Swordsman);
	const bool bArrived = CurrentDestination
		&& FVector::Dist2D(Swordsman->GetActorLocation(), *CurrentDestination)
			<= FMath::Max(0.0f, SwordsmanRoamArrivalDistance);
	const bool bDue = !NextRoamTime || Now >= *NextRoamTime;
	if (NextRoamTime && CurrentDestination && !bArrived && !bDue)
	{
		return;
	}

	FVector Destination;
	if (BuildSwordsmanRoamDestination(Swordsman, Destination))
	{
		CommandSwordsmanMove(Swordsman, Destination, SwordsmanRoamSpeed);
	}
	const float IntervalMin = FMath::Max(0.1f, SwordsmanRoamIntervalMin);
	SwordsmanNextRoamTimes.Add(Swordsman,
		Now + FMath::FRandRange(IntervalMin, FMath::Max(IntervalMin, SwordsmanRoamIntervalMax)));
}

void AOngseongEnemyWaveManager::UpdateSwordsmanRoamBehavior()
{
	for (auto It = SwordsmanNextRoamTimes.CreateIterator(); It; ++It)
	{
		if (!IsValid(It.Key()))
		{
			SwordsmanMoveDestinations.Remove(It.Key());
			It.RemoveCurrent();
		}
	}
	// Iterate a copy: ApplySwordsmanRoamBehavior writes back into the same map.
	TArray<TObjectPtr<AEnemyCombatCharacter>> Roamers;
	SwordsmanNextRoamTimes.GetKeys(Roamers);
	for (AEnemyCombatCharacter* Swordsman : Roamers)
	{
		ApplySwordsmanRoamBehavior(Swordsman);
	}
}

FTransform AOngseongEnemyWaveManager::BuildSpawnTransform(const bool bUseRespawnPoint)
{
	constexpr int32 FormationWidth = 3;
	const int32 FormationIndex = SpawnSequence++ % FormationWidth;
	const float CenteredIndex = static_cast<float>(FormationIndex - FormationWidth / 2);
	const AActor* SpawnOrigin = bUseRespawnPoint && IsValid(SoldierRespawnPoint)
		? static_cast<const AActor*>(SoldierRespawnPoint)
		: (IsValid(InitialSpawnPoint) ? static_cast<const AActor*>(InitialSpawnPoint) : static_cast<const AActor*>(this));
	FVector SpawnLocation = SpawnOrigin->GetActorLocation() + SpawnOrigin->GetActorRightVector() * CenteredIndex * SpawnSpacing;
	if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation ProjectedLocation;
		if (NavigationSystem->ProjectPointToNavigation(SpawnLocation, ProjectedLocation, SpawnNavProjectionExtent))
		{
			const FVector AuthoredLocation = SpawnLocation;
			SpawnLocation = ProjectedLocation.Location + FVector::UpVector * SpawnHeightAboveNavmesh;
			UE_LOG(LogOngseong, VeryVerbose, TEXT("Projected authored spawn %s onto navigation at %s."),
				*AuthoredLocation.ToCompactString(), *SpawnLocation.ToCompactString());
		}
		else
		{
			UE_LOG(LogOngseong, Warning, TEXT("Could not project authored spawn %s onto navigation; spawned enemies may not move."),
				*SpawnLocation.ToCompactString());
		}
	}
	return FTransform(SpawnOrigin->GetActorRotation(), SpawnLocation);
}
