#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Pooling/ActorPool.h"
#include "Ongseong/OngseongEnemyWaveManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOngseongWaveManagerConfigurationTest,
	"SuwonSiegeContestVR.Ongseong.WaveManager.Configuration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOngseongWaveManagerConfigurationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World))
	{
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AOngseongEnemyWaveManager* Manager = World->SpawnActor<AOngseongEnemyWaveManager>();
	AActorPool* Pool = World->SpawnActor<AActorPool>();
	AActor* Target = World->SpawnActor<AActor>();
	if (TestNotNull(TEXT("Wave manager is spawned"), Manager) &&
		TestNotNull(TEXT("Enemy pool is spawned"), Pool) &&
		TestNotNull(TEXT("Objective target is spawned"), Target))
	{
		TestEqual(TEXT("The ongseong holds forty enemies"), Manager->GetMaxConcurrentEnemies(), 40);
		TestEqual(TEXT("Twenty-two of them are swordsmen"), Manager->GetSwordsmanSlots(), 22);
		TestEqual(TEXT("Eighteen of them are archers"), Manager->GetArcherSlots(), 18);
		TestEqual(TEXT("Slot counts match the concurrency cap"),
			Manager->GetSwordsmanSlots() + Manager->GetArcherSlots(), Manager->GetMaxConcurrentEnemies());
		TestTrue(TEXT("Defeated enemies are replaced by default"), Manager->IsMaintainingPopulation());
		TestEqual(TEXT("Replacements arrive two seconds later"), Manager->GetRespawnDelay(), 2.0f);
		TestEqual(TEXT("Nothing is alive before spawning starts"), Manager->GetLivingEnemyCount(), 0);
		TestEqual(TEXT("Nothing has been defeated before spawning starts"), Manager->GetTotalDefeatedEnemies(), 0);
		TestFalse(TEXT("Spawning is inactive before it is started"), Manager->IsSpawningActive());

		TestFalse(TEXT("Unconfigured manager is rejected"), Manager->IsSpawnConfigured());
		Manager->SetEnemyPool(Pool);
		Manager->SetObjectiveTarget(Target);
		TestTrue(TEXT("Pool and target form a valid configuration"), Manager->IsSpawnConfigured());
		TestFalse(TEXT("A pool without an enemy class cannot spawn"), Manager->SpawnEnemy());
		TestEqual(TEXT("A failed spawn is reported by the pool"), Pool->GetExhaustedRequestCount(), 1);
		TestEqual(TEXT("A failed spawn adds nobody to the population"), Manager->GetLivingEnemyCount(), 0);

		Manager->ResetWave();
		TestEqual(TEXT("Reset clears the population"), Manager->GetLivingEnemyCount(), 0);
		TestEqual(TEXT("Reset cancels pending respawns"), Manager->GetPendingRespawnCount(), 0);
		TestFalse(TEXT("Reset stops spawning"), Manager->IsSpawningActive());
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
