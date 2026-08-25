#include "Gameplay/Pooling/ActorPool.h"

#include "Gameplay/Pooling/PoolableActorInterface.h"

DEFINE_LOG_CATEGORY(LogActorPool);

AActorPool::AActorPool()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AActorPool::BeginPlay()
{
	Super::BeginPlay();
	PrewarmPool();
}

AActor* AActorPool::AcquireActor(const FTransform& SpawnTransform)
{
	if (!bHasPrewarmed)
	{
		// A consumer can run BeginPlay before this pool does. Fill on first use instead of failing.
		PrewarmPool();
	}
	PruneInvalidActors();
	AActor* PooledActor = nullptr;
	while (!AvailableActors.IsEmpty() && !IsValid(PooledActor))
	{
		PooledActor = AvailableActors.Pop();
	}
	if (!PooledActor && bAllowPoolExpansion)
	{
		PooledActor = CreatePooledActor();
	}
	if (!IsValid(PooledActor))
	{
		++ExhaustedRequestCount;
		UE_LOG(LogActorPool, Warning,
			TEXT("%s exhausted: %s, active %d, total %d, expansion %s. Increase InitialPoolSize or lower the concurrency budget."),
			*GetName(),
			PooledActorClass ? *PooledActorClass->GetName() : TEXT("<no class>"),
			ActiveActors.Num(),
			GetTotalCount(),
			bAllowPoolExpansion ? TEXT("on") : TEXT("off"));
		OnPoolExhausted.Broadcast(this, ActiveActors.Num());
		return nullptr;
	}

	PooledActor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	PooledActor->SetActorHiddenInGame(false);
	PooledActor->SetActorEnableCollision(true);
	PooledActor->SetActorTickEnabled(true);
	ActiveActors.Add(PooledActor);
	if (PooledActor->GetClass()->ImplementsInterface(UPoolableActorInterface::StaticClass()))
	{
		IPoolableActorInterface::Execute_OnAcquiredFromPool(PooledActor);
	}
	return PooledActor;
}

bool AActorPool::ReleaseActor(AActor* ActorToRelease)
{
	if (!IsValid(ActorToRelease) || !ActiveActors.RemoveSingle(ActorToRelease))
	{
		return false;
	}

	DeactivateActor(ActorToRelease);
	AvailableActors.Add(ActorToRelease);
	return true;
}

void AActorPool::PrewarmPool()
{
	bHasPrewarmed = true;
	PruneInvalidActors();
	const int32 DesiredAvailableCount = FMath::Max(0, InitialPoolSize - GetTotalCount());
	for (int32 Index = 0; Index < DesiredAvailableCount; ++Index)
	{
		if (AActor* NewActor = CreatePooledActor())
		{
			DeactivateActor(NewActor);
			AvailableActors.Add(NewActor);
		}
	}
}

AActor* AActorPool::CreatePooledActor()
{
	if (!PooledActorClass || !GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<AActor>(PooledActorClass, GetActorTransform(), SpawnParameters);
}

void AActorPool::DeactivateActor(AActor* ActorToDeactivate)
{
	if (ActorToDeactivate->GetClass()->ImplementsInterface(UPoolableActorInterface::StaticClass()))
	{
		IPoolableActorInterface::Execute_OnReleasedToPool(ActorToDeactivate);
	}
	ActorToDeactivate->SetActorEnableCollision(false);
	ActorToDeactivate->SetActorHiddenInGame(true);
	ActorToDeactivate->SetActorTickEnabled(false);
}

void AActorPool::PruneInvalidActors()
{
	AvailableActors.RemoveAll([](const TObjectPtr<AActor>& Actor)
	{
		return !IsValid(Actor);
	});
	ActiveActors.RemoveAll([](const TObjectPtr<AActor>& Actor)
	{
		return !IsValid(Actor);
	});
}
