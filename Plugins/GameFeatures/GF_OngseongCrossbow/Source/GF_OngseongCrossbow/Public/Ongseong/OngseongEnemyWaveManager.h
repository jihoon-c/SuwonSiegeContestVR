#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatTypes.h"
#include "OngseongEnemyWaveManager.generated.h"

class AActorPool;
class UHealthComponent;

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

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	bool SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void SetEnemyPool(AActorPool* NewEnemyPool) { EnemyPool = NewEnemyPool; }

	UFUNCTION(BlueprintCallable, Category = "Ongseong|Wave")
	void SetObjectiveTarget(AActor* NewObjectiveTarget) { ObjectiveTarget = NewObjectiveTarget; }

	UFUNCTION(BlueprintPure, Category = "Ongseong|Wave")
	bool IsSpawnConfigured() const;

protected:
	UFUNCTION()
	void HandleEnemyDeath(UHealthComponent* HealthComponent, const FCombatDamageSpec& KillingDamage);

	void SpawnScheduledEnemy();
	FTransform BuildSpawnTransform();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	TObjectPtr<AActorPool> EnemyPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	TObjectPtr<AActor> ObjectiveTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0.0"))
	float InitialDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0.1"))
	float SpawnInterval = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "1"))
	int32 MaxActiveEnemies = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave", meta = (ClampMin = "0.0"))
	float SpawnSpacing = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ongseong|Wave")
	bool bAutoStart = true;

	int32 SpawnSequence = 0;
	FTimerHandle SpawnTimerHandle;
};
