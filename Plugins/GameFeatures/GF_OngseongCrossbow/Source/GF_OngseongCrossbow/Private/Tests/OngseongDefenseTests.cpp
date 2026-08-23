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
		TestEqual(TEXT("Default wave contains ten swordsmen"), Wave->GetSwordsmenToSpawn(), 10);
		TestEqual(TEXT("Default wave contains ten archers"), Wave->GetArchersToSpawn(), 10);
		TestTrue(TEXT("Defense repeats cleared waves by default"), Scenario->ShouldRepeatWavesDuringDefense());
		TestEqual(TEXT("Default inter-wave delay is three seconds"), Scenario->GetInterWaveDelay(), 3.0f);
		TestEqual(TEXT("No wave is counted while the scenario is idle"), Scenario->GetCurrentWaveNumber(), 0);

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

		TestTrue(TEXT("Configured defense starts its first wave"), Scenario->StartDefense());
		TestEqual(TEXT("The initial group is wave one"), Scenario->GetCurrentWaveNumber(), 1);
		struct FWaveDefeatedParams
		{
			int32 DefeatedEnemies = 20;
		} WaveDefeatedParams;
		if (UFunction* WaveDefeatedFunction = Scenario->FindFunction(TEXT("HandleWaveDefeated")))
		{
			Scenario->ProcessEvent(WaveDefeatedFunction, &WaveDefeatedParams);
			TestFalse(TEXT("Infantry alone does not complete a wave that still has a ram"), Scenario->IsNextWavePending());
			AOngseongRamActor* FirstWaveRam = Scenario->GetActiveRam();
			if (TestNotNull(TEXT("Wave one contains a ram"), FirstWaveRam))
			{
				FCombatDamageSpec FatalRamDamage;
				FatalRamDamage.Amount = 1000.0f;
				FatalRamDamage.bIgnoreFaction = true;
				FirstWaveRam->ReceiveCombatDamage_Implementation(FatalRamDamage);
				TestTrue(TEXT("Defeating infantry and ram schedules the next wave"), Scenario->IsNextWavePending());
			}
			if (UFunction* StartNextWaveFunction = Scenario->FindFunction(TEXT("StartNextWave")))
			{
				Scenario->ProcessEvent(StartNextWaveFunction, nullptr);
				TestEqual(TEXT("The scheduled callback starts wave two"), Scenario->GetCurrentWaveNumber(), 2);
				TestNotNull(TEXT("Wave two spawns a fresh ram"), Scenario->GetActiveRam());
				TestFalse(TEXT("Starting the next wave consumes its pending timer"), Scenario->IsNextWavePending());
			}

			Scenario->ProcessEvent(WaveDefeatedFunction, &WaveDefeatedParams);
			if (AOngseongRamActor* SecondWaveRam = Scenario->GetActiveRam())
			{
				FCombatDamageSpec FatalRamDamage;
				FatalRamDamage.Amount = 1000.0f;
				FatalRamDamage.bIgnoreFaction = true;
				SecondWaveRam->ReceiveCombatDamage_Implementation(FatalRamDamage);
			}
			TestTrue(TEXT("A later fully cleared group schedules another wave"), Scenario->IsNextWavePending());
			if (UFunction* GateDestroyedFunction = Scenario->FindFunction(TEXT("HandleGateDestroyed")))
			{
				Scenario->ProcessEvent(GateDestroyedFunction, nullptr);
				TestFalse(TEXT("Failure cancels a pending repeated wave"), Scenario->IsNextWavePending());
			}
		}
		else
		{
			AddError(TEXT("HandleWaveDefeated must remain bound as a scenario event handler"));
		}
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
