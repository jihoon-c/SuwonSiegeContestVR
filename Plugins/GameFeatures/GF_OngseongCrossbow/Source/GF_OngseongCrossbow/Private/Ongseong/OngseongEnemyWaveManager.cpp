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

	// Swordsmen continuously refresh their individual slots around the moving ram. A location
	// command is used rather than MoveToActor so every soldier keeps a distinct destination.
	GetWorldTimerManager().ClearTimer(SwordsmanFormationTimerHandle);
	GetWorldTimerManager().SetTimer(
		SwordsmanFormationTimerHandle,
		this,
		&AOngseongEnemyWaveManager::UpdateSwordsmanFormation,
		FMath::Max(0.1f, SwordsmanFormationUpdateInterval),
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
		GetWorldTimerManager().ClearTimer(SwordsmanFormationTimerHandle);
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
		ReleaseCannonSlots(Enemy);
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
	SwordsmanFormationSlots.Reset();
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

	if (UHealthComponent* HealthComponent = Enemy->GetHealthComponent())
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AOngseongEnemyWaveManager::HandleEnemyDeath);
	}
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
		AssignSwordsmanFormationSlot(Enemy);
		ApplySwordsmanMarch(Enemy);
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
		SwordsmanFormationSlots.Remove(Enemy);
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
	ReleaseCannonSlots(Enemy);
	SwordsmanFormationSlots.Remove(Enemy);
	if (ACombatAIController* Controller = Cast<ACombatAIController>(Enemy->GetController()))
	{
		Controller->OnMoveTargetReached.RemoveDynamic(this, &AOngseongEnemyWaveManager::HandleRetreatTargetReached);
	}
}

void AOngseongEnemyWaveManager::SetArcherEscortTarget(AActor* NewEscortTarget)
{
	ArcherEscortTarget = NewEscortTarget;
	UpdateSwordsmanFormation();
}

AChongtongCannonActor* AOngseongEnemyWaveManager::ReserveCannonForArcher(AEnemyCombatCharacter* Archer)
{
	if (!IsValid(Archer) || !GetWorld())
	{
		return nullptr;
	}

	// Nearest first, so archers spread over the emplacements they are actually walking past.
	TArray<AChongtongCannonActor*> Candidates;
	for (TActorIterator<AChongtongCannonActor> It(GetWorld()); It; ++It)
	{
		AChongtongCannonActor* Cannon = *It;
		const UHealthComponent* Health = Cannon ? Cannon->FindComponentByClass<UHealthComponent>() : nullptr;
		if (IsValid(Cannon) && (!Health || !Health->IsDead()))
		{
			Candidates.Add(Cannon);
		}
	}
	const FVector ArcherLocation = Archer->GetActorLocation();
	Candidates.Sort([&ArcherLocation](const AChongtongCannonActor& A, const AChongtongCannonActor& B)
	{
		return FVector::DistSquared(A.GetActorLocation(), ArcherLocation)
			< FVector::DistSquared(B.GetActorLocation(), ArcherLocation);
	});

	for (AChongtongCannonActor* Cannon : Candidates)
	{
		if (Cannon->TryReserveAttackerSlot(Archer))
		{
			return Cannon;
		}
	}
	return nullptr;
}

void AOngseongEnemyWaveManager::ReleaseCannonSlots(AActor* Archer)
{
	if (!GetWorld())
	{
		return;
	}
	for (TActorIterator<AChongtongCannonActor> It(GetWorld()); It; ++It)
	{
		It->ReleaseAttackerSlot(Archer);
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

	AChongtongCannonActor* Cannon = ReserveCannonForArcher(Archer);
	// A full emplacement is simply ignored: the surplus walks on to the ram instead of queueing.
	AActor* MoveTarget = Cannon ? static_cast<AActor*>(Cannon)
		: (IsValid(ArcherEscortTarget) ? ArcherEscortTarget.Get() : ObjectiveTarget.Get());
	ArcherCombat->ConfigureCombat(Cannon, ObjectiveTarget, ArcherProjectilePool);

	if (ACombatAIController* Controller = Cast<ACombatAIController>(Archer->GetController()))
	{
		// The Behavior Tree walks to TargetActor, so this is the move destination, not the aim point.
		Controller->SetCombatTarget(MoveTarget);
		Controller->MoveToCombatActor(MoveTarget, FMath::Min(ArcherRange * 0.9f, ArcherApproachRadius));
	}
	UE_LOG(LogOngseong, Verbose, TEXT("%s engages %s (escorting=%d)."),
		*Archer->GetName(), *GetNameSafe(MoveTarget), Cannon ? 0 : 1);
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

int32 AOngseongEnemyWaveManager::AssignSwordsmanFormationSlot(AEnemyCombatCharacter* Swordsman)
{
	if (!IsValid(Swordsman))
	{
		return INDEX_NONE;
	}
	if (const int32* ExistingSlot = SwordsmanFormationSlots.Find(Swordsman))
	{
		return *ExistingSlot;
	}

	TSet<int32> OccupiedSlots;
	for (const TPair<TObjectPtr<AEnemyCombatCharacter>, int32>& Pair : SwordsmanFormationSlots)
	{
		if (IsValid(Pair.Key))
		{
			OccupiedSlots.Add(Pair.Value);
		}
	}
	int32 SlotIndex = 0;
	while (OccupiedSlots.Contains(SlotIndex))
	{
		++SlotIndex;
	}
	SwordsmanFormationSlots.Add(Swordsman, SlotIndex);
	return SlotIndex;
}

FVector AOngseongEnemyWaveManager::GetSwordsmanFormationLocation(const int32 SlotIndex) const
{
	const AActor* MarchTarget = IsValid(ArcherEscortTarget) ? ArcherEscortTarget.Get() : ObjectiveTarget.Get();
	if (!IsValid(MarchTarget))
	{
		return GetActorLocation();
	}

	FVector Forward = IsValid(ObjectiveTarget)
		? ObjectiveTarget->GetActorLocation() - MarchTarget->GetActorLocation()
		: MarchTarget->GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		Forward = FVector::ForwardVector;
	}
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
	constexpr int32 FormationColumns = 4;
	const int32 Column = FMath::Max(0, SlotIndex) % FormationColumns;
	const int32 Row = FMath::Max(0, SlotIndex) / FormationColumns;
	const float LateralOffset = (static_cast<float>(Column) - 1.5f) * SwordsmanFormationSpacing;
	const float TrailingOffset = SwordsmanTrailingDistance + static_cast<float>(Row) * SwordsmanFormationSpacing;
	FVector FormationLocation = MarchTarget->GetActorLocation() - Forward * TrailingOffset + Right * LateralOffset;
	FormationLocation.Z = MarchTarget->GetActorLocation().Z;
	return FormationLocation;
}

void AOngseongEnemyWaveManager::ApplySwordsmanMarch(AEnemyCombatCharacter* Swordsman)
{
	if (!IsValid(Swordsman))
	{
		return;
	}
	const int32 SlotIndex = AssignSwordsmanFormationSlot(Swordsman);
	if (SlotIndex == INDEX_NONE)
	{
		return;
	}
	const FVector FormationLocation = GetSwordsmanFormationLocation(SlotIndex);
	const float DistanceToSlot = FVector::Dist2D(Swordsman->GetActorLocation(), FormationLocation);
	if (UCharacterMovementComponent* Movement = Swordsman->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = DistanceToSlot > SwordsmanCatchUpDistance
			? SwordsmanCatchUpSpeed : SwordsmanMarchSpeed;
	}
	if (ACombatAIController* Controller = Cast<ACombatAIController>(Swordsman->GetController()))
	{
		Controller->MoveToCombatLocation(FormationLocation, SwordsmanFormationAcceptanceRadius);
	}
}

void AOngseongEnemyWaveManager::UpdateSwordsmanFormation()
{
	SwordsmanFormationSlots.RemoveAll([](const TPair<TObjectPtr<AEnemyCombatCharacter>, int32>& Pair)
	{
		return !IsValid(Pair.Key);
	});
	for (const TPair<TObjectPtr<AEnemyCombatCharacter>, int32>& Pair : SwordsmanFormationSlots)
	{
		ApplySwordsmanMarch(Pair.Key);
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
