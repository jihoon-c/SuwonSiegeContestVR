#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/Scenario/ScenarioDefinition.h"
#include "Core/Scenario/ScenarioManagerActor.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Enemy/GongsimdonEnemyGroupActor.h"
#include "Interaction/GongsimdonCombatTargetActor.h"
#include "Interaction/GongsimdonObservationTargetActor.h"
#include "Interaction/GongsimdonReportActor.h"
#include "Scenario/GongsimdonScenarioDirectorActor.h"
#include "Shared/Characters/EnemySoldierActor.h"
#include "Shared/Combat/FactionComponent.h"
#include "Shared/Combat/LegacyHealthComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGongsimdonActionInteractionFlowTest,
	"SuwonSiegeContestVR.GF_Gongsimdon.Interaction.ActionFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGongsimdonEnemyGroupFlowTest,
	"SuwonSiegeContestVR.GF_Gongsimdon.Enemy.GroupFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGongsimdonEnemyGroupFlowTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Enemy test world is created"), World))
	{
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AGongsimdonEnemyGroupActor* Group = World->SpawnActor<AGongsimdonEnemyGroupActor>();
	TestNotNull(TEXT("Enemy Group is spawned"), Group);
	if (Group)
	{
		TestTrue(TEXT("Default enemy formation spawns"), Group->SpawnEnemies());
		TestEqual(TEXT("Default formation contains seven soldiers"), Group->GetSpawnedEnemyCount(), 7);
		AEnemySoldierActor* Soldier = Group->GetSpawnedEnemy(0);
		TestNotNull(TEXT("First soldier exists"), Soldier);
		if (Soldier)
		{
			TestNotNull(TEXT("Soldier has a visible placeholder Skeletal Mesh"),
				Soldier->GetMesh()->GetSkeletalMeshAsset());
			TestNotNull(TEXT("Soldier has Health"), Soldier->GetHealthComponent());
			TestNotNull(TEXT("Soldier has Faction"), Soldier->GetFactionComponent());
			if (Soldier->GetFactionComponent())
			{
				TestEqual(TEXT("Soldier belongs to Enemy faction"),
					Soldier->GetFactionComponent()->Faction, ELegacyCombatFaction::Enemy);
			}
		}

		Group->BeginApproach();
		TestEqual(TEXT("Reveal starts approach"),
			Group->GetGroupState(), EGongsimdonEnemyGroupState::Approaching);
		Group->Tick(30.0f);
		TestEqual(TEXT("Approach reaches holding state"),
			Group->GetGroupState(), EGongsimdonEnemyGroupState::Holding);

		Group->StartRetreat();
		Group->SetCombatArmed(true);
		TestEqual(TEXT("Retreat cue starts retreat movement"),
			Group->GetGroupState(), EGongsimdonEnemyGroupState::Retreating);
		if (Soldier && Soldier->GetHealthComponent())
		{
			TestTrue(TEXT("A valid shot applies soldier damage"),
				Soldier->GetHealthComponent()->ApplyHealthDamage(1.0f) > 0.0f);
			TestEqual(TEXT("One required hit completes and hides the group"),
				Group->GetGroupState(), EGongsimdonEnemyGroupState::Escaped);
		}
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

bool FGongsimdonActionInteractionFlowTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World))
	{
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AScenarioManagerActor* ManagerActor = World->SpawnActor<AScenarioManagerActor>();
	AGongsimdonReportActor* ReportActor = World->SpawnActor<AGongsimdonReportActor>();
	AGongsimdonCombatTargetActor* CombatActor = World->SpawnActor<AGongsimdonCombatTargetActor>();
	AGongsimdonObservationTargetActor* ObservationActor =
		World->SpawnActor<AGongsimdonObservationTargetActor>();
	AGongsimdonScenarioDirectorActor* Director =
		World->SpawnActor<AGongsimdonScenarioDirectorActor>();
	UScenarioManagerComponent* Manager = ManagerActor ? ManagerActor->GetScenarioManager() : nullptr;
	TestNotNull(TEXT("Scenario Manager is spawned"), Manager);
	TestNotNull(TEXT("Report Actor is spawned"), ReportActor);
	TestNotNull(TEXT("Combat Actor is spawned"), CombatActor);
	TestNotNull(TEXT("Observation Actor is spawned"), ObservationActor);
	TestNotNull(TEXT("Scenario Director is spawned"), Director);

	FScenarioStageDefinition Stage;
	Stage.StageID = TEXT("GONG_TEST_STAGE");
	Stage.StartInteractionID = TEXT("GONG_TEST_REPORT");

	FScenarioInteraction ReportInteraction;
	ReportInteraction.InteractionID = TEXT("GONG_TEST_REPORT");
	ReportInteraction.InteractionType = EScenarioInteractionType::Custom;
	ReportInteraction.TargetID = TEXT("REPORT_ENEMY");
	ReportInteraction.NextInteractionID = TEXT("GONG_TEST_COMBAT");
	Stage.Interactions.Add(ReportInteraction);

	FScenarioInteraction CombatInteraction;
	CombatInteraction.InteractionID = TEXT("GONG_TEST_COMBAT");
	CombatInteraction.InteractionType = EScenarioInteractionType::Combat;
	CombatInteraction.TargetID = TEXT("COMBAT_RETREATING");
	Stage.Interactions.Add(CombatInteraction);

	UScenarioDefinition* Scenario = NewObject<UScenarioDefinition>();
	Scenario->ScenarioID = TEXT("SCENARIO_GongsimdonTest");
	Scenario->StartStageID = Stage.StageID;
	Scenario->Stages.Add(Stage);

	if (Manager && ReportActor && CombatActor && Director)
	{
		TestTrue(TEXT("Action Scenario starts"), Manager->StartScenario(Scenario));
		TestTrue(TEXT("Director connects to the active Scenario"), Director->InitializeDirector());
		TestFalse(TEXT("Wrong direction is rejected"),
			ReportActor->SubmitReport(EGongsimdonReportDirection::West, 6));
		TestEqual(TEXT("Wrong report does not advance Scenario"),
			Manager->GetDebugSnapshot().InteractionID, FName(TEXT("GONG_TEST_REPORT")));
		TestTrue(TEXT("East and six enemies is accepted"),
			ReportActor->SubmitReport(EGongsimdonReportDirection::East, 6));
		TestEqual(TEXT("Correct report advances to Combat"),
			Manager->GetDebugSnapshot().InteractionID, FName(TEXT("GONG_TEST_COMBAT")));

		CombatActor->RequiredHits = 2;
		TestTrue(TEXT("First valid shot is counted"), CombatActor->RegisterHit());
		TestEqual(TEXT("Combat remains active before required hits"),
			Manager->GetDebugSnapshot().InteractionID, FName(TEXT("GONG_TEST_COMBAT")));
		TestTrue(TEXT("Second valid shot completes Combat"), CombatActor->RegisterHit());
		TestEqual(TEXT("Action Scenario completes"),
			Manager->GetDebugSnapshot().ScenarioState, EScenarioState::Completed);
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
