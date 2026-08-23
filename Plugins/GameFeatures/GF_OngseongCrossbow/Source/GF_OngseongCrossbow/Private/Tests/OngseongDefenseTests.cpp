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
	// AActor::ProcessEvent drops every UFUNCTION call, including dynamic delegates, until the world
	// reports its actors as initialized. Without this the death/objective callbacks never arrive.
	FURL URL;
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AOngseongGateActor* Gate = World->SpawnActor<AOngseongGateActor>();
	AOngseongEnemyWaveManager* Wave = World->SpawnActor<AOngseongEnemyWaveManager>();
	AOngseongDefenseScenarioManager* Scenario = World->SpawnActor<AOngseongDefenseScenarioManager>();
	AOngseongRamActor* Ram = World->SpawnActor<AOngseongRamActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (TestNotNull(TEXT("Gate is spawned"), Gate) && TestNotNull(TEXT("Scenario is spawned"), Scenario))
	{
		TestEqual(TEXT("The ongseong holds fifteen enemies"), Wave->GetMaxConcurrentEnemies(), 15);
		TestTrue(TEXT("Defeated enemies are replaced during the defense"), Wave->IsMaintainingPopulation());
		TestFalse(TEXT("The defense is cleared by the ram, not by a timer"), Scenario->IsDefenseTimeLimited());
		TestFalse(TEXT("No ram has been destroyed while the scenario is idle"), Scenario->IsRamDestroyed());

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

		Gate->ResetGate();
		TestTrue(TEXT("Configured defense starts"), Scenario->StartDefense());
		TestEqual(TEXT("Starting the defense enters the defending state"), Scenario->GetDefenseState(), EOngseongDefenseState::Defending);
		TestEqual(TEXT("No enemy has been defeated yet"), Scenario->GetTotalDefeatedEnemies(), 0);

		AOngseongRamActor* FirstRam = Scenario->GetActiveRam();
		if (TestNotNull(TEXT("The defense starts with a ram"), FirstRam))
		{
			FCombatDamageSpec FatalRamDamage;
			FatalRamDamage.Amount = 1000.0f;
			FatalRamDamage.bIgnoreFaction = true;
			TestTrue(TEXT("Fatal damage reaches the ram"), FirstRam->ReceiveCombatDamage_Implementation(FatalRamDamage));
			TestTrue(TEXT("Destroying the ram clears the experience"), Scenario->IsRamDestroyed());
			TestNull(TEXT("A destroyed ram leaves no active ram"), Scenario->GetActiveRam());
			TestEqual(TEXT("Destroying the ram succeeds the defense"), Scenario->GetDefenseState(), EOngseongDefenseState::Succeeded);
		}

		if (UFunction* GateDestroyedFunction = Scenario->FindFunction(TEXT("HandleGateDestroyed")))
		{
			Scenario->ProcessEvent(GateDestroyedFunction, nullptr);
			TestEqual(TEXT("A late gate report cannot overwrite a cleared defense"), Scenario->GetDefenseState(), EOngseongDefenseState::Succeeded);

			// Restart, then lose the gate before the ram dies.
			Scenario->RetryDefense();
			TestEqual(TEXT("Retry restarts the defense"), Scenario->GetDefenseState(), EOngseongDefenseState::Defending);
			TestFalse(TEXT("Retry clears the previous ram result"), Scenario->IsRamDestroyed());
			TestNotNull(TEXT("Retry sends in a single new ram"), Scenario->GetActiveRam());

			Scenario->ProcessEvent(GateDestroyedFunction, nullptr);
			TestEqual(TEXT("Gate destruction fails the defense"), Scenario->GetDefenseState(), EOngseongDefenseState::Failed);
			TestFalse(TEXT("Failure stops enemy spawning"), Wave->IsSpawningActive());
			TestEqual(TEXT("Failure clears the population"), Wave->GetLivingEnemyCount(), 0);
		}
		else
		{
			AddError(TEXT("HandleGateDestroyed must remain bound as a scenario event handler"));
		}
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
