#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/CombatFactionComponent.h"
#include "Gameplay/Combat/GameplayProjectileActor.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Gameplay/Pooling/PoolableActorInterface.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSharedGameplayFactionRulesTest,
	"SuwonSiegeContestVR.Gameplay.Combat.FactionRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSharedGameplayFactionRulesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Enemy is hostile to Player"),
		UCombatFactionComponent::AreHostile(ECombatFaction::Enemy, ECombatFaction::Player));
	TestTrue(TEXT("Enemy is hostile to Ally"),
		UCombatFactionComponent::AreHostile(ECombatFaction::Enemy, ECombatFaction::Ally));
	TestFalse(TEXT("Player is not hostile to Ally"),
		UCombatFactionComponent::AreHostile(ECombatFaction::Player, ECombatFaction::Ally));
	TestFalse(TEXT("Neutral is protected"),
		UCombatFactionComponent::AreHostile(ECombatFaction::Neutral, ECombatFaction::Enemy));
	TestFalse(TEXT("Same faction is protected"),
		UCombatFactionComponent::AreHostile(ECombatFaction::Enemy, ECombatFaction::Enemy));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSharedGameplayHealthResetTest,
	"SuwonSiegeContestVR.Gameplay.Combat.HealthReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSharedGameplayHealthResetTest::RunTest(const FString& Parameters)
{
	UHealthComponent* Health = NewObject<UHealthComponent>();
	if (!TestNotNull(TEXT("Health component is created"), Health))
	{
		return false;
	}

	FCombatDamageSpec FatalDamage;
	FatalDamage.Amount = Health->GetMaxHealth();
	TestTrue(TEXT("Fatal damage is applied"), Health->ApplyDamage(FatalDamage));
	TestTrue(TEXT("Fatal damage marks the component dead"), Health->IsDead());

	Health->ResetHealth();
	TestFalse(TEXT("Pool reset revives the component"), Health->IsDead());
	TestEqual(TEXT("Pool reset restores maximum health"), Health->GetCurrentHealth(), Health->GetMaxHealth());

	Health->SetCurrentHealth(0.0f);
	TestTrue(TEXT("Direct zero-health override produces a consistent dead state"), Health->IsDead());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSharedGameplayPoolCapacityTest,
	"SuwonSiegeContestVR.Gameplay.Pooling.CapacityInvariant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSharedGameplayPoolCapacityTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World))
	{
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AActorPool* Pool = World->SpawnActor<AActorPool>();
	const FClassProperty* ActorClassProperty = FindFProperty<FClassProperty>(AActorPool::StaticClass(), TEXT("PooledActorClass"));
	const FIntProperty* PoolSizeProperty = FindFProperty<FIntProperty>(AActorPool::StaticClass(), TEXT("InitialPoolSize"));
	if (TestNotNull(TEXT("Actor pool is spawned"), Pool) &&
		TestNotNull(TEXT("PooledActorClass property exists"), ActorClassProperty) &&
		TestNotNull(TEXT("InitialPoolSize property exists"), PoolSizeProperty))
	{
		const_cast<FClassProperty*>(ActorClassProperty)->SetPropertyValue_InContainer(Pool, AActor::StaticClass());
		const_cast<FIntProperty*>(PoolSizeProperty)->SetPropertyValue_InContainer(Pool, 2);

		Pool->PrewarmPool();
		TestEqual(TEXT("Initial prewarm creates the configured capacity"), Pool->GetTotalCount(), 2);

		AActor* FirstActor = Pool->AcquireActor(FTransform::Identity);
		TestNotNull(TEXT("An actor can be acquired"), FirstActor);
		TestEqual(TEXT("Acquire moves one actor to the active set"), Pool->GetActiveCount(), 1);

		Pool->PrewarmPool();
		TestEqual(TEXT("Repeated prewarm does not exceed total capacity"), Pool->GetTotalCount(), 2);
		TestTrue(TEXT("The acquired actor can be released"), Pool->ReleaseActor(FirstActor));
		TestEqual(TEXT("Release returns all actors to the available set"), Pool->GetAvailableCount(), 2);
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSharedGameplayProjectilePoolingContractTest,
	"SuwonSiegeContestVR.Gameplay.Pooling.ProjectileContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSharedGameplayProjectilePoolingContractTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Shared projectiles implement the pool reset contract"),
		AGameplayProjectileActor::StaticClass()->ImplementsInterface(UPoolableActorInterface::StaticClass()));
	return true;
}

#endif
