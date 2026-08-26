#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongEnemyWaveManager.generated.h"

class AActorPool;
class APawn;
class AEnemyCombatCharacter;
class UHealthComponent;
class UOngseongArcherCombatComponent;
class AOngseongSpawnPointActor;
class ATargetPoint;

UENUM(BlueprintType)
enum class EOngseongEnemyType : uint8
{
	Swordsman,
	Archer
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongSpawningStarted, int32, MaxConcurrentEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOngseongEnemySpawned, AEnemyCombatCharacter*, Enemy, int32, LivingEnemies, int32, MaxConcurrentEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOngseongEnemyDefeated, int32, TotalDefeated, int32, LivingEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOngseongPopulationChanged, int32, LivingEnemies, int32, MaxConcurrentEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOngseongAllEnemiesRetreated);

/**
 * Keeps a fixed enemy population inside the ongseong instead of running discrete waves.
 * Enemies are acquired from shared pools, and every defeated enemy is returned to its pool and
 * replaced after a delay so the assault never stops while the defense timer runs.
 */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongEnemyWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AOngseongEnemyWaveManager();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Begins filling the ongseong up to the configured population and keeps it filled. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void StartSpawning();

	/** Stops filling and cancels every pending respawn. Living enemies are left alone. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void StopSpawning();

	/** Stops spawning and sends every living pooled enemy toward the configured retreat point. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void RetreatAllEnemies(FVector RetreatLocation);

	/** Immediately returns every living enemy to the shared pool (failure/teardown path). */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void ReleaseAllEnemies();

	/** Clears living enemies, pending respawns and run-time counters. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void ResetWave();

	/** Spawns one enemy of whichever type is furthest below its slot count. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	bool SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void SetEnemyPool(AActorPool* NewEnemyPool) { EnemyPool = NewEnemyPool; }

	/** Where surplus archers gather once every cannon is fully engaged. Normally the active ram. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning|Archer")
	void SetArcherEscortTarget(AActor* NewEscortTarget);

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void SetObjectiveTarget(AActor* NewObjectiveTarget) { ObjectiveTarget = NewObjectiveTarget; }
	/** Optional player target for archers. When unset, player 0 is discovered at runtime. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning|Archer")
	void SetArcherPlayerTarget(AActor* NewArcherPlayerTarget) { ArcherPlayerTarget = NewArcherPlayerTarget; }

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void SetInitialSpawnPoint(AOngseongSpawnPointActor* NewSpawnPoint) { InitialSpawnPoint = NewSpawnPoint; }

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void SetSoldierRespawnPoint(AOngseongSpawnPointActor* NewSpawnPoint) { SoldierRespawnPoint = NewSpawnPoint; }

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Spawning")
	void SetArcherPrimaryTarget(AActor* NewArcherPrimaryTarget) { ArcherPrimaryTarget = NewArcherPrimaryTarget; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	bool IsSpawnConfigured() const;

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	bool IsSpawningActive() const { return bSpawningActive; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	bool IsMaintainingPopulation() const { return bMaintainPopulation; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetLivingEnemyCount() const { return ActiveEnemies.Num(); }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetLivingEnemyCountOfType(EOngseongEnemyType EnemyType) const;

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetTotalDefeatedEnemies() const { return TotalDefeatedEnemies; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetMaxConcurrentEnemies() const { return FMath::Max(1, MaxConcurrentEnemies); }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetSwordsmanSlots() const { return FMath::Max(0, SwordsmanSlots); }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetArcherSlots() const { return FMath::Max(0, ArcherSlots); }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	float GetRespawnDelay() const { return RespawnDelay; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	int32 GetPendingRespawnCount() const { return RespawnTimerHandles.Num(); }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Spawning")
	AActor* GetObjectiveTarget() const { return ObjectiveTarget; }

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Spawning")
	FOnOngseongSpawningStarted OnSpawningStarted;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Spawning")
	FOnOngseongEnemySpawned OnEnemySpawned;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Spawning")
	FOnOngseongEnemyDefeated OnEnemyDefeated;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Spawning")
	FOnOngseongPopulationChanged OnPopulationChanged;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Spawning")
	FOnOngseongAllEnemiesRetreated OnAllEnemiesRetreated;

protected:
	UFUNCTION()
	void HandleEnemyDeath(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage);
	UFUNCTION()
	void HandleEnemyDeathPresentationFinished(AEnemyCombatCharacter* Enemy);
	UFUNCTION()
	void HandleRetreatTargetReached(APawn* EnemyPawn);

	void TickPopulationFill();
	bool SpawnEnemyOfType(EOngseongEnemyType EnemyType, bool bUseRespawnPoint = false);
	void ScheduleRespawn(EOngseongEnemyType EnemyType);
	void HandleRespawnTimer(EOngseongEnemyType EnemyType);
	void ClearPendingRespawns();
	void PruneFinishedRespawnTimers();
	bool HasPopulationDeficit() const;
	int32 GetSlotsForType(EOngseongEnemyType EnemyType) const;
	bool ChooseNextEnemyType(EOngseongEnemyType& OutEnemyType) const;
	AActorPool* GetPoolForEnemyType(EOngseongEnemyType EnemyType) const;
	/** Reserves the nearest free authored firing position and returns its linked cannon. */
	ATargetPoint* ReserveAttackPositionForArcher(
		AEnemyCombatCharacter* Archer,
		class AChongtongCannonActor*& OutCannon);
	void ReleaseArcherAttackPosition(AActor* Archer);
	void ApplyArcherEngagement(AEnemyCombatCharacter* Archer);
	/** Registers a swordsman as a roamer and sends it toward its first destination. */
	void ApplySwordsmanRoamBehavior(AEnemyCombatCharacter* Swordsman);
	FVector ResolveSwordsmanRoamCentre() const;
	/** Picks a reachable navmesh point inside the roaming area. */
	bool BuildSwordsmanRoamDestination(const AEnemyCombatCharacter* Swordsman, FVector& OutDestination) const;
	void CommandSwordsmanMove(AEnemyCombatCharacter* Swordsman, const FVector& Destination, float Speed);
	UFUNCTION()
	void UpdateSwordsmanRoamBehavior();
	UFUNCTION()
	void RetryArcherSlotAssignments();
	void ResizePoolsToSlotCounts();
	void DetachEnemy(AEnemyCombatCharacter* Enemy);
	void ResolveSpawnPoints();
	FTransform BuildSpawnTransform(bool bUseRespawnPoint);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning")
	TObjectPtr<AActorPool> EnemyPool;

	/** Optional dedicated archer pool. When unset, archers use EnemyPool. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning")
	TObjectPtr<AActorPool> ArcherEnemyPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning")
	TObjectPtr<AActor> ObjectiveTarget;

	/** Optional explicit marker for the first population fill. Falls back to this manager's transform. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Spawning|Points")
	TObjectPtr<AOngseongSpawnPointActor> InitialSpawnPoint;

	/** Optional explicit marker used only for replacements after a soldier is defeated. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Spawning|Points")
	TObjectPtr<AOngseongSpawnPointActor> SoldierRespawnPoint;

	/** Allied cannon preferred by archers. Falls back to ObjectiveTarget when unavailable. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer")
	TObjectPtr<AActor> ArcherPrimaryTarget;

	/** The player may be assigned explicitly; otherwise the first player pawn is used. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer")
	TObjectPtr<AActor> ArcherPlayerTarget;

	/**
	 * Where archers gather when every allied cannon already has its attack slots filled.
	 * The scenario points this at the active ram, so the surplus escorts the siege engine.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer")
	TObjectPtr<AActor> ArcherEscortTarget;

	/** How often archers without a cannon slot retry, so a defender's death frees a place. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="0.5"))
	float ArcherSlotRetryInterval = 3.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer")
	TObjectPtr<AActorPool> ArcherProjectilePool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ArcherHitChance = 0.65f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="100.0"))
	float ArcherRange = 2000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="0.1"))
	float ArcherFireInterval = 2.75f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="0.0"))
	float ArcherMissRadius = 275.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="0.0"))
	float ArcherDamage = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="1.0"))
	float ArcherProjectileSpeed = 6500.0f;

	/**
	 * Acceptance radius around an authored archer attack position.
	 * A tight radius makes path following run the final correction in a single frame, which reads
	 * as the archer snapping onto the slot. Stopping anywhere in a wide ring removes the snap.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Archer", meta=(ClampMin="100.0"))
	float ArcherApproachRadius = 450.0f;

	/**
	 * Walk speed used while a swordsman crosses the courtyard to its next roam point.
	 * Zero leaves the character's own MaxWalkSpeed alone, which is what archers do, so the two
	 * enemy types move at the same pace. Set a positive value only to deliberately differ.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="0.0"))
	float SwordsmanRoamSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="0.1"))
	float SwordsmanBehaviorUpdateInterval = 0.5f;

	/**
	 * Radius of the roaming area around the roam anchor. Swordsmen no longer escort the ram;
	 * they wander the inside of the fortress, so this needs to cover the whole courtyard.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="100.0"))
	float SwordsmanRoamRadius = 3000.0f;

	/**
	 * Centre of the roaming area. Falls back to an Actor tagged Ongseong.SwordsmanRoamAnchor,
	 * then to the centre of the authored archer attack positions, then to this manager.
	 * The objective is deliberately NOT a fallback: anchoring on the gate is what pulled every
	 * swordsman into a clump around the ram.
	 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman")
	TObjectPtr<AActor> SwordsmanRoamAnchor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="0.0"))
	float SwordsmanMoveAcceptanceRadius = 120.0f;

	/** A roam point is abandoned and re-picked once the soldier is this close to it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="0.0"))
	float SwordsmanRoamArrivalDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="0.1"))
	float SwordsmanRoamIntervalMin = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Swordsman", meta=(ClampMin="0.1"))
	float SwordsmanRoamIntervalMax = 7.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning", meta = (ClampMin = "0.0"))
	float InitialDelay = 1.0f;

	/** Interval used while filling up to the population. Keeps the initial spawns off a single frame. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning", meta = (ClampMin = "0.05"))
	float InitialSpawnInterval = 0.2f;

	/** Editor-adjustable maximum number of living enemy soldiers. Set the two type slot values to totals that can fill this cap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ongseong|Spawning", meta = (ClampMin = "1", DisplayName = "Maximum Spawned Enemy Soldiers"))
	int32 MaxConcurrentEnemies = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ongseong|Spawning", meta = (ClampMin = "0"))
	int32 SwordsmanSlots = 22;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ongseong|Spawning", meta = (ClampMin = "0"))
	int32 ArcherSlots = 18;

	/** Each living allied cannon can have this many reserved archer attack positions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ongseong|Spawning|Archer", meta = (ClampMin = "1"))
	int32 ArcherAttackSlotsPerCannon = 3;

	/** Seconds between a defeat and the replacement enemy leaving the spawn point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning", meta = (ClampMin = "0.0"))
	float RespawnDelay = 2.0f;

	/** Random deviation applied to RespawnDelay so replacements do not arrive in lockstep. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning", meta = (ClampMin = "0.0"))
	float RespawnDelayJitter = 0.75f;

	/** Turn off to spawn the initial population once without replacements (debug). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning")
	bool bMaintainPopulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning", meta = (ClampMin = "0.0"))
	float SpawnSpacing = 250.0f;

	/** Search volume used to place authored spawn markers onto reachable navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Points")
	FVector SpawnNavProjectionExtent = FVector(500.0f, 500.0f, 2000.0f);

	/** Character actor height above the projected navmesh floor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Points", meta = (ClampMin = "0.0"))
	float SpawnHeightAboveNavmesh = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning")
	bool bAutoStart = true;

	/**
	 * Grows the enemy and arrow pools at BeginPlay so the population can actually be filled.
	 * Level-authored InitialPoolSize values do not track this manager's slot counts, and a pool
	 * that runs dry just logs "exhausted" and quietly stops replacing the dead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Pooling")
	bool bResizePoolsToSlotCounts = true;

	/** Spare actors kept above the slot count, covering enemies still playing their death animation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Pooling", meta = (ClampMin = "0"))
	int32 EnemyPoolHeadroom = 6;

	/** Arrows alive at once: every archer can have several in flight during their 8s lifespan. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Spawning|Pooling", meta = (ClampMin = "1"))
	int32 ArrowsPerArcher = 4;

	int32 SpawnSequence = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Spawning")
	int32 TotalSpawnedEnemies = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Spawning")
	int32 TotalDefeatedEnemies = 0;
	bool bSpawningActive = false;
	bool bRetreating = false;
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyCombatCharacter>> ActiveEnemies;
	UPROPERTY(Transient)
	TMap<TObjectPtr<AEnemyCombatCharacter>, TObjectPtr<AActorPool>> EnemyPoolsByActor;
	UPROPERTY(Transient)
	TMap<TObjectPtr<AEnemyCombatCharacter>, EOngseongEnemyType> EnemyTypesByActor;
	UPROPERTY(Transient)
	TMap<TObjectPtr<AEnemyCombatCharacter>, TObjectPtr<ATargetPoint>> ArcherAttackPositionsByEnemy;
	UPROPERTY(Transient)
	TMap<TObjectPtr<AEnemyCombatCharacter>, FVector> SwordsmanMoveDestinations;
	UPROPERTY(Transient)
	TMap<TObjectPtr<AEnemyCombatCharacter>, float> SwordsmanNextRoamTimes;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle ArcherSlotRetryHandle;
	FTimerHandle SwordsmanBehaviorTimerHandle;
	TArray<FTimerHandle> RespawnTimerHandles;
};
