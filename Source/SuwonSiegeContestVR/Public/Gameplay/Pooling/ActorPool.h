#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActorPool.generated.h"

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
};
