#include "Misc/AutomationTest.h"

#include "Nokro/NokroCraneActor.h"
#include "Nokro/NokroRepairTargetActor.h"
#include "Nokro/NokroScenarioManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNokroPlacementToleranceTest,
	"Project.GameFeatures.Nokro.PlacementTolerance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNokroPlacementToleranceTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	ANokroRepairTargetActor* Target = World->SpawnActor<ANokroRepairTargetActor>();
	TestNotNull(TEXT("Repair target spawned"), Target);
	if (!Target) { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); return false; }
	const FTransform Exact = Target->GetPlacementTransform();
	TestTrue(TEXT("Exact transform is accepted"), Target->IsStoneWithinTolerance(Exact));
	FTransform TooFar = Exact;
	TooFar.AddToTranslation(FVector(Target->PositionTolerance + 1.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Position outside tolerance is rejected"), Target->IsStoneWithinTolerance(TooFar));
	FTransform WrongYaw = Exact;
	WrongYaw.SetRotation(FRotator(0.0f, Target->YawTolerance + 1.0f, 0.0f).Quaternion());
	TestFalse(TEXT("Yaw outside tolerance is rejected"), Target->IsStoneWithinTolerance(WrongYaw));
	Target->CompleteRepair();
	TestFalse(TEXT("Completed target cannot accept another stone"), Target->IsStoneWithinTolerance(Exact));
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNokroCraneLimitsTest,
	"Project.GameFeatures.Nokro.CraneHeightLimits",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNokroCraneLimitsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	ANokroCraneActor* Crane = World->SpawnActor<ANokroCraneActor>();
	TestNotNull(TEXT("Crane spawned"), Crane);
	if (!Crane) { World->DestroyWorld(false); GEngine->DestroyWorldContext(World); return false; }
	Crane->ApplyHandleRotation(10000.0f);
	TestEqual(TEXT("Height clamps at maximum"), Crane->GetCurrentHeight(), Crane->MaximumHeight);
	Crane->ApplyHandleRotation(-10000.0f);
	TestEqual(TEXT("Height clamps at minimum"), Crane->GetCurrentHeight(), Crane->MinimumHeight);
	Crane->SetBoomYaw(450.0f);
	TestTrue(TEXT("Boom yaw is normalized"), FMath::IsNearlyEqual(Crane->GetCurrentBoomYaw(), 90.0f));
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNokroScenarioLoopTest,
	"Project.GameFeatures.Nokro.SuccessFailureLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNokroScenarioLoopTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	ANokroCraneActor* Crane = World->SpawnActor<ANokroCraneActor>();
	ANokroRepairTargetActor* Target = World->SpawnActor<ANokroRepairTargetActor>();
	ANokroScenarioManager* Scenario = World->SpawnActor<ANokroScenarioManager>();
	if (!TestNotNull(TEXT("Scenario fixtures spawned"), Scenario) || !Crane || !Target)
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
		return false;
	}
	Scenario->Crane = Crane;
	Scenario->RepairTargets = { Target };
	Scenario->bSpawnDefaultLayout = false;
	Scenario->bCompleteExperienceOnFinish = false;
	TestTrue(TEXT("Scenario starts with explicit fixtures"), Scenario->StartScenario());
	FTransform Miss = Crane->GetCarriedStoneTransform();
	Miss.AddToTranslation(FVector(1000.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Incorrect placement fails"), Scenario->TryPlaceStoneAtTransform(Miss));
	TestFalse(TEXT("Failure leaves target unrepaired"), Target->IsRepaired());
	Target->SetActorTransform(Crane->GetCarriedStoneTransform());
	TestTrue(TEXT("Correct placement succeeds"), Scenario->TryPlaceStoneAtTransform(Crane->GetCarriedStoneTransform()));
	TestTrue(TEXT("Success repairs target"), Target->IsRepaired());
	TestEqual(TEXT("Final stone completes scenario"), Scenario->GetScenarioState(), ENokroScenarioState::Completed);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}

#endif
