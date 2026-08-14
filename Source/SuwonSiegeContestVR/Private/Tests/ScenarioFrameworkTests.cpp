#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Animation/AnimInstance.h"
#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Scenario/ScenarioDefinition.h"
#include "Core/Scenario/ScenarioManagerActor.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Core/Scenario/ScenarioSceneData.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVRPawnInputAndHandAnimationConfigurationTest,
	"SuwonSiegeContestVR.Core.VR.InputAndHandAnimationConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVRPawnInputAndHandAnimationConfigurationTest::RunTest(const FString& Parameters)
{
	const UClass* HandAnimClass = LoadClass<UAnimInstance>(
		nullptr,
		TEXT("/Game/XRMannequins/Meshes/ABP_MannequinsXR.ABP_MannequinsXR_C"));
	if (!TestNotNull(TEXT("Hand animation Blueprint class loads"), HandAnimClass))
	{
		return false;
	}

	const FNumericProperty* GraspProperty = FindFProperty<FNumericProperty>(HandAnimClass, TEXT("PoseAlphaGrasp"));
	TestNotNull(TEXT("Hand animation exposes PoseAlphaGrasp as a numeric property"), GraspProperty);
	if (GraspProperty)
	{
		TestTrue(TEXT("PoseAlphaGrasp is a floating-point property"), GraspProperty->IsFloatingPoint());
	}
	TestTrue(
		TEXT("Quest left thumbstick X key is registered"),
		EKeys::GetKeyDetails(FKey(TEXT("OculusTouch_Left_Thumbstick_X"))).IsValid());
	TestTrue(
		TEXT("Quest right thumbstick X key is registered"),
		EKeys::GetKeyDetails(FKey(TEXT("OculusTouch_Right_Thumbstick_X"))).IsValid());
	TestTrue(
		TEXT("Quest right thumbstick Y key is registered"),
		EKeys::GetKeyDetails(FKey(TEXT("OculusTouch_Right_Thumbstick_Y"))).IsValid());
	TestTrue(
		TEXT("Quest left trigger axis key is registered"),
		EKeys::GetKeyDetails(FKey(TEXT("OculusTouch_Left_Trigger_Axis"))).IsValid());
	TestTrue(
		TEXT("Quest right trigger axis key is registered"),
		EKeys::GetKeyDetails(FKey(TEXT("OculusTouch_Right_Trigger_Axis"))).IsValid());
	TestTrue(TEXT("PIE movement key W is registered"), EKeys::GetKeyDetails(EKeys::W).IsValid());
	TestTrue(TEXT("PIE left grab key F is registered"), EKeys::GetKeyDetails(EKeys::F).IsValid());
	TestNotNull(
		TEXT("Startup-registered right thumbstick action exists"),
		LoadObject<UInputAction>(nullptr, TEXT("/Game/XRFramework/Input/Actions/IA_Menu_Cursor_Right.IA_Menu_Cursor_Right")));
	TestNotNull(
		TEXT("Startup-registered left trigger action exists"),
		LoadObject<UInputAction>(nullptr, TEXT("/Game/XRFramework/Input/Actions/Hands/IA_Hand_IndexCurl_Left.IA_Hand_IndexCurl_Left")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExperienceDefinitionAndProgressTest,
	"SuwonSiegeContestVR.Core.Experience.DefinitionAndProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExperienceDefinitionAndProgressTest::RunTest(const FString& Parameters)
{
	UExperienceDefinition* Definition = NewObject<UExperienceDefinition>();
	FString ValidationError;
	TestFalse(TEXT("Experience without an ID is rejected"), Definition->ValidateDefinition(ValidationError));

	Definition->ExperienceID = TEXT("EXP_Test");
	Definition->ExperienceLevel = TSoftObjectPtr<UWorld>(
		FSoftObjectPath(TEXT("/Game/Tests/L_ExperienceTest.L_ExperienceTest")));
	TestTrue(TEXT("Experience with ID and level is valid"), Definition->ValidateDefinition(ValidationError));

	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	GameInstance->Init();
	UExperienceSubsystem* ExperienceSubsystem = GameInstance->GetSubsystem<UExperienceSubsystem>();
	if (TestNotNull(TEXT("Experience subsystem is created for the game instance"), ExperienceSubsystem))
	{
		TestTrue(TEXT("Direct-level experience activation succeeds"),
			ExperienceSubsystem->ActivateExperienceForCurrentLevel(Definition));
		TestEqual(TEXT("Experience becomes active"),
			ExperienceSubsystem->GetExperienceState(), EExperienceState::Active);
		TestTrue(TEXT("Experience completion succeeds"),
			ExperienceSubsystem->CompleteCurrentExperience(false));
		TestTrue(TEXT("Completed experience is retained for the session"),
			ExperienceSubsystem->IsExperienceCompleted(Definition->ExperienceID));
		TestEqual(TEXT("Experience becomes completed"),
			ExperienceSubsystem->GetExperienceState(), EExperienceState::Completed);

		ExperienceSubsystem->ResetSessionProgress();
		TestFalse(TEXT("Reset clears session progress"),
			ExperienceSubsystem->IsExperienceCompleted(Definition->ExperienceID));
	}
	GameInstance->Shutdown();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScenarioDefinitionValidationTest,
	"SuwonSiegeContestVR.Core.Scenario.DefinitionValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScenarioDefinitionValidationTest::RunTest(const FString& Parameters)
{
	UScenarioSceneData* Scene = NewObject<UScenarioSceneData>();
	Scene->SceneID = TEXT("SCENE_Test");
	Scene->StartInteractionID = TEXT("INT_Start");

	FScenarioInteraction Start;
	Start.InteractionID = TEXT("INT_Start");
	Start.NextInteractionID = TEXT("INT_Missing");
	Scene->Interactions.Add(Start);

	FString Error;
	TestFalse(TEXT("A missing branch target is rejected"), Scene->ValidateScene(Error));
	TestTrue(TEXT("Validation identifies the missing interaction"), Error.Contains(TEXT("INT_Missing")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScenarioSynchronousFlowTest,
	"SuwonSiegeContestVR.Core.Scenario.SynchronousFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScenarioSynchronousFlowTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World))
	{
		return false;
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	AScenarioManagerActor* ManagerActor = World->SpawnActor<AScenarioManagerActor>();
	UScenarioManagerComponent* Manager = ManagerActor ? ManagerActor->GetScenarioManager() : nullptr;
	TestNotNull(TEXT("Manager component is available"), Manager);

	UScenarioSceneData* Scene = NewObject<UScenarioSceneData>();
	Scene->SceneID = TEXT("SCENE_Test");
	Scene->StartInteractionID = TEXT("INT_Objective");

	FScenarioInteraction Objective;
	Objective.InteractionID = TEXT("INT_Objective");
	Objective.InteractionType = EScenarioInteractionType::Objective;
	Objective.bCompleteOnStart = true;
	Objective.NextInteractionID = TEXT("INT_Wait");
	Scene->Interactions.Add(Objective);

	FScenarioInteraction Wait;
	Wait.InteractionID = TEXT("INT_Wait");
	Wait.InteractionType = EScenarioInteractionType::Wait;
	Wait.Duration = 0.0f;
	Scene->Interactions.Add(Wait);

	UScenarioDefinition* Scenario = NewObject<UScenarioDefinition>();
	Scenario->ScenarioID = TEXT("SCENARIO_Test");
	Scenario->StartSceneID = Scene->SceneID;
	Scenario->Scenes.Add(Scene);

	if (Manager)
	{
		ManagerActor->SetScenarioDefinition(Scenario);
		TestEqual(TEXT("Actor configuration reaches manager component"), Manager->ScenarioDefinition.Get(), Scenario);
		TestTrue(TEXT("Configured scenario starts"), ManagerActor->StartConfiguredScenario());
		const FScenarioDebugSnapshot Snapshot = Manager->GetDebugSnapshot();
		TestEqual(TEXT("Scenario completes"), Snapshot.ScenarioState, EScenarioState::Completed);
		TestEqual(TEXT("Scene completes"), Snapshot.SceneState, EScenarioSceneState::Completed);
		TestEqual(TEXT("Objective completes"), Manager->GetInteractionState(Objective.InteractionID), EScenarioInteractionState::Completed);
		TestEqual(TEXT("Wait completes"), Manager->GetInteractionState(Wait.InteractionID), EScenarioInteractionState::Completed);
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FScenarioProjectAssetConfigurationTest,
	"SuwonSiegeContestVR.Core.Scenario.ProjectAssetConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScenarioProjectAssetConfigurationTest::RunTest(const FString& Parameters)
{
	UScenarioDefinition* Scenario = LoadObject<UScenarioDefinition>(
		nullptr, TEXT("/Game/Data/DA_Scenario_Singijeon.DA_Scenario_Singijeon"));
	UScenarioSceneData* Scene = LoadObject<UScenarioSceneData>(
		nullptr, TEXT("/Game/Data/DA_Scene_Singijeon.DA_Scene_Singijeon"));
	UDataTable* NarrationTable = LoadObject<UDataTable>(
		nullptr, TEXT("/Game/Data/DT_Narration.DT_Narration"));
	UExperienceDefinition* ExperienceDefinition = LoadObject<UExperienceDefinition>(
		nullptr,
		TEXT("/Game/Core/Experience/Definitions/DA_Experience_Singijeon.DA_Experience_Singijeon"));

	if (!TestNotNull(TEXT("Singijeon Scenario Definition exists"), Scenario) ||
		!TestNotNull(TEXT("Singijeon Scene exists"), Scene) ||
		!TestNotNull(TEXT("Narration Data Table exists"), NarrationTable) ||
		!TestNotNull(TEXT("Singijeon Experience Definition exists"), ExperienceDefinition))
	{
		return false;
	}

	FString ValidationError;
	TestTrue(TEXT("Singijeon Scenario data is valid"), Scenario->ValidateScenario(ValidationError));
	if (!ValidationError.IsEmpty())
	{
		AddError(ValidationError);
	}
	TestEqual(TEXT("Scenario starts from the configured Scene"), Scenario->StartSceneID, Scene->SceneID);
	for (const FScenarioInteraction& Interaction : Scene->Interactions)
	{
		if (Interaction.InteractionType == EScenarioInteractionType::Narration && !Interaction.NarrationID.IsNone())
		{
			TestTrue(
				*FString::Printf(TEXT("Narration row %s referenced by the Scene exists"),
					*Interaction.NarrationID.ToString()),
				NarrationTable->GetRowMap().Contains(Interaction.NarrationID));
		}
	}

	UWorld* LevelWorld = LoadObject<UWorld>(
		nullptr, TEXT("/GF_Singijeon/Maps/LV_Singijeon.LV_Singijeon"));
	if (!TestNotNull(TEXT("LV_Singijeon loads"), LevelWorld) ||
		!TestNotNull(TEXT("LV_Singijeon persistent level exists"), LevelWorld->PersistentLevel.Get()))
	{
		return false;
	}

	AScenarioManagerActor* PlacedManager = nullptr;
	for (AActor* Actor : LevelWorld->PersistentLevel->Actors)
	{
		if (AScenarioManagerActor* Candidate = Cast<AScenarioManagerActor>(Actor))
		{
			PlacedManager = Candidate;
			break;
		}
	}
	if (TestNotNull(TEXT("LV_Singijeon contains a Scenario Manager"), PlacedManager))
	{
		TestEqual(TEXT("Placed Manager references DA_Scenario_Singijeon"),
			PlacedManager->ScenarioDefinition.Get(), Scenario);
		TestEqual(TEXT("Placed Manager references DT_Narration"),
			PlacedManager->NarrationTable.Get(), NarrationTable);
		TestEqual(TEXT("Placed Manager references DA_Experience_Singijeon"),
			PlacedManager->ExperienceDefinition.Get(), ExperienceDefinition);
		TestTrue(TEXT("Placed Manager auto-start is enabled"), PlacedManager->bAutoStartScenario);
		TestTrue(TEXT("Placed Manager completes Experience after Scenario"),
			PlacedManager->bCompleteExperienceOnScenarioFinished);
	}
	return true;
}

#endif
