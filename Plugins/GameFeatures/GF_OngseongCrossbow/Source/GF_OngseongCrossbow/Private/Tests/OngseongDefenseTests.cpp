#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Gameplay/Combat/HealthComponent.h"
#include "Ongseong/OngseongDefenseScenarioManager.h"
#include "Ongseong/OngseongEnemyWaveManager.h"
#include "Ongseong/OngseongGateActor.h"
#include "Ongseong/OngseongRamActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOngseongDefenseContractsTest,
	"SuwonSiegeContestVR.Ongseong.Defense.Contracts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOngseongDefenseContractsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World)) return false;
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);

	AOngseongGateActor* Gate = World->SpawnActor<AOngseongGateActor>();
	AOngseongEnemyWaveManager* Wave = World->SpawnActor<AOngseongEnemyWaveManager>();
	AOngseongDefenseScenarioManager* Scenario = World->SpawnActor<AOngseongDefenseScenarioManager>();
	AOngseongRamActor* Ram = World->SpawnActor<AOngseongRamActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (TestNotNull(TEXT("Gate is spawned"), Gate) && TestNotNull(TEXT("Scenario is spawned"), Scenario))
	{
		TestEqual(TEXT("Default wave contains three swordsmen"), Wave->GetSwordsmenToSpawn(), 3);
		TestEqual(TEXT("Default wave contains two archers"), Wave->GetArchersToSpawn(), 2);

		FCombatDamageSpec Damage;
		Damage.Amount = 25.0f;
		Damage.bIgnoreFaction = true;
		TestTrue(TEXT("Gate accepts shared combat damage"), Gate->ReceiveCombatDamage_Implementation(Damage));
		TestEqual(TEXT("Gate health changes"), Gate->GetHealthComponent()->GetCurrentHealth(), 75.0f);
		Gate->ResetGate();
		TestEqual(TEXT("Gate reset restores health"), Gate->GetHealthComponent()->GetCurrentHealth(), 100.0f);

		TestFalse(TEXT("Scenario rejects missing references"), Scenario->StartDefense());
		Scenario->SetGateActor(Gate);
		Scenario->SetWaveManager(Wave);
		TestEqual(TEXT("Scenario remains idle until explicitly started"), Scenario->GetDefenseState(), EOngseongDefenseState::Idle);

		if (TestNotNull(TEXT("Ram is spawned"), Ram))
		{
			Gate->SetActorLocation(FVector(1000.0f, 0.0f, 0.0f));
			Ram->ActivateRam(Gate);
			TestEqual(TEXT("Completed ram advances immediately"), Ram->GetRamState(), EOngseongRamState::Advancing);
			Ram->Tick(10.0f);
			TestEqual(TEXT("Ram charges after reaching its staging point"), Ram->GetRamState(), EOngseongRamState::Charging);
			Ram->Tick(10.0f);
			TestEqual(TEXT("Ram returns after impacting the gate"), Ram->GetRamState(), EOngseongRamState::Returning);
			TestEqual(TEXT("Ram impact damages the gate once"), Gate->GetHealthComponent()->GetCurrentHealth(), 25.0f);
			Ram->Tick(10.0f);
			TestEqual(TEXT("Ram begins another charge after returning"), Ram->GetRamState(), EOngseongRamState::Charging);
		}
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
