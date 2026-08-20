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
		TestFalse(TEXT("Unconfigured manager is rejected"), Manager->IsSpawnConfigured());
		Manager->SetEnemyPool(Pool);
		Manager->SetObjectiveTarget(Target);
		TestTrue(TEXT("Pool and target form a valid configuration"), Manager->IsSpawnConfigured());
		TestFalse(TEXT("A pool without an enemy class cannot spawn"), Manager->SpawnEnemy());
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
