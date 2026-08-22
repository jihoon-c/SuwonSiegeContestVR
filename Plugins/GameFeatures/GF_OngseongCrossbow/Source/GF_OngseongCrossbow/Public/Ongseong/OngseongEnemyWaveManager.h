#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongEnemyWaveManager.generated.h"

class AActorPool;
class AEnemyCombatCharacter;
class UHealthComponent;

UENUM(BlueprintType)
enum class EOngseongEnemyType : uint8
{
	Swordsman,
	Archer
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongWaveStarted, int32, TotalEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOngseongEnemySpawned, AEnemyCombatCharacter*, Enemy, int32, SpawnedEnemies, int32, TotalEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOngseongWaveProgress, int32, DefeatedEnemies, int32, TotalEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOngseongAllEnemiesDefeated, int32, TotalEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOngseongAllEnemiesRetreated);

/**
 * Minimal Ongseong wave loop that acquires enemies from a shared pool, assigns the gate objective,
 * and returns defeated enemies to the pool. Feature-specific presentation remains in Blueprint.
 */
UCLASS(Blueprintable)
class GF_ONGSEONGCROSSBOW_API AOngseongEnemyWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AOngseongEnemyWaveManager();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void StopSpawning();

	/** Stops the wave and sends every living pooled enemy toward the configured retreat point. */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void RetreatAllEnemies(FVector RetreatLocation);

	/** Immediately returns every living enemy to the shared pool (failure/teardown path). */
	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void ReleaseAllEnemies();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void ResetWave();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	bool SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void SetEnemyPool(AActorPool* NewEnemyPool) { EnemyPool = NewEnemyPool; }

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void SetObjectiveTarget(AActor* NewObjectiveTarget) { ObjectiveTarget = NewObjectiveTarget; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	bool IsSpawnConfigured() const;

	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	bool AreAllEnemiesDefeated() const { return TotalEnemiesToSpawn > 0 && DefeatedEnemyCount >= TotalEnemiesToSpawn; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	bool HasWaveStarted() const { return bWaveStarted; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	int32 GetDefeatedEnemyCount() const { return DefeatedEnemyCount; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	int32 GetTotalEnemiesToSpawn() const { return TotalEnemiesToSpawn; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	int32 GetSwordsmenToSpawn() const { return SwordsmenToSpawn; }
	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	int32 GetArchersToSpawn() const { return ArchersToSpawn; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	AActor* GetObjectiveTarget() const { return ObjectiveTarget; }

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Wave")
	FOnOngseongWaveStarted OnWaveStarted;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Wave")
	FOnOngseongEnemySpawned OnEnemySpawned;

	UPROPERTY(BlueprintAssignable, Category="Ongseong|Wave")
	FOnOngseongWaveProgress OnWaveProgress;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Wave")
	FOnOngseongAllEnemiesDefeated OnAllEnemiesDefeated;
	UPROPERTY(BlueprintAssignable, Category="Ongseong|Wave")
	FOnOngseongAllEnemiesRetreated OnAllEnemiesRetreated;

protected:
	UFUNCTION()
	void HandleEnemyDeath(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage);
	UFUNCTION()
	void HandleRetreatTargetReached(AActor* EnemyActor);

	void SpawnScheduledEnemy();
	EOngseongEnemyType ChooseNextEnemyType() const;
	AActorPool* GetPoolForEnemyType(EOngseongEnemyType EnemyType) const;
	FTransform BuildSpawnTransform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	TObjectPtr<AActorPool> EnemyPool;

	/** Optional dedicated archer pool. When unset, archers use EnemyPool with an ArcherAdvance behavior state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	TObjectPtr<AActorPool> ArcherEnemyPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	TObjectPtr<AActor> ObjectiveTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0.0"))
	float InitialDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0.1"))
	float SpawnInterval = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "1"))
	int32 MaxActiveEnemies = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0"))
	int32 SwordsmenToSpawn = 3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0"))
	int32 ArchersToSpawn = 2;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ongseong|Wave")
	int32 TotalEnemiesToSpawn = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0.0"))
	float SpawnSpacing = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	bool bAutoStart = true;

	int32 SpawnSequence = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Wave")
	int32 SpawnedEnemyCount = 0;
	int32 SpawnedSwordsmanCount = 0;
	int32 SpawnedArcherCount = 0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Ongseong|Wave")
	int32 DefeatedEnemyCount = 0;
	bool bWaveStarted = false;
	bool bRetreating = false;
	UPROPERTY(Transient)
	TArray<TObjectPtr<AEnemyCombatCharacter>> ActiveEnemies;
	UPROPERTY(Transient)
	TMap<TObjectPtr<AEnemyCombatCharacter>, TObjectPtr<AActorPool>> EnemyPoolsByActor;
	FTimerHandle SpawnTimerHandle;
};
