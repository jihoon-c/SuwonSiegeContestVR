#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActorPool.generated.h"

class AActorPool;

SUWONSIEGECONTESTVR_API DECLARE_LOG_CATEGORY_EXTERN(LogActorPool, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorPoolExhausted, AActorPool*, Pool, int32, ActiveCount);

/** Prewarms and reuses Actors to avoid spawn/destruction spikes on standalone VR hardware. */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AActorPool : public AActor
{
	GENERATED_BODY()

public:
	AActorPool();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Pooling")
	AActor* AcquireActor(const FTransform& SpawnTransform);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Pooling")
	bool ReleaseActor(AActor* ActorToRelease);

	UFUNCTION(BlueprintCallable, Category = "Gameplay|Pooling")
	void PrewarmPool();

	UFUNCTION(BlueprintPure, Category = "Gameplay|Pooling")
	int32 GetAvailableCount() const { return AvailableActors.Num(); }

	UFUNCTION(BlueprintPure, Category = "Gameplay|Pooling")
	int32 GetActiveCount() const { return ActiveActors.Num(); }

	UFUNCTION(BlueprintPure, Category = "Gameplay|Pooling")
	int32 GetTotalCount() const { return AvailableActors.Num() + ActiveActors.Num(); }

	UFUNCTION(BlueprintPure, Category = "Gameplay|Pooling")
	TSubclassOf<AActor> GetPooledActorClass() const { return PooledActorClass; }

	UFUNCTION(BlueprintPure, Category = "Gameplay|Pooling")
	int32 GetExhaustedRequestCount() const { return ExhaustedRequestCount; }

	/** Raised whenever a request could not be served. Pools stay fixed size, so this means the budget is too small. */
	UPROPERTY(BlueprintAssignable, Category = "Gameplay|Pooling")
	FOnActorPoolExhausted OnPoolExhausted;

protected:
	AActor* CreatePooledActor();
	void DeactivateActor(AActor* ActorToDeactivate);
	void PruneInvalidActors();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Pooling")
	TSubclassOf<AActor> PooledActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Pooling", meta = (ClampMin = "0"))
	int32 InitialPoolSize = 8;

	/** Keep false for predictable CPU/memory use; designers should size pools from the scenario's concurrency budget. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay|Pooling")
	bool bAllowPoolExpansion = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Pooling")
	TArray<TObjectPtr<AActor>> AvailableActors;

	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Pooling")
	TArray<TObjectPtr<AActor>> ActiveActors;

	/** BeginPlay order between a pool and its consumers is not guaranteed, so the first acquire prewarms. */
	bool bHasPrewarmed = false;

	/** Counts silent acquisition failures so undersized pools are visible in logs and tests. */
	UPROPERTY(VisibleInstanceOnly, Category = "Gameplay|Pooling")
	int32 ExhaustedRequestCount = 0;
};
