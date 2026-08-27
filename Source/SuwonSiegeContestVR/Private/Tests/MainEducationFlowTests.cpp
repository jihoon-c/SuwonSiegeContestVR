#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Main/Education/MainEducationScenarioDefinition.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMainEducationDefaultFlowTest,
	"Suwon.Main.Education.DefaultFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMainEducationDefaultFlowTest::RunTest(const FString& Parameters)
{
	const UMainEducationScenarioDefinition* Definition =
		NewObject<UMainEducationScenarioDefinition>(GetTransientPackage());
	TestNotNull(TEXT("Default definition is created"), Definition);
	if (!Definition)
	{
		return false;
	}

	FString ValidationError;
	TestTrue(TEXT("Default education flow validates"), Definition->ValidateEducationScenario(ValidationError));
	if (!ValidationError.IsEmpty())
	{
		AddError(ValidationError);
	}
	TestEqual(TEXT("Gate and five presentation stages"), Definition->Stages.Num(), 6);
	TestEqual(TEXT("Editor Flow mirrors the six runtime stages"), Definition->EditorFlow.Num(), 6);
	TestEqual(TEXT("Main presentation flow has no experience routes"), Definition->ExperienceRoutes.Num(), 0);

	for (int32 StageIndex = 0; StageIndex < Definition->EditorFlow.Num(); ++StageIndex)
	{
		const FMainEducationAuthoringStage& EditorStage = Definition->EditorFlow[StageIndex];
		const FScenarioStageDefinition& RuntimeStage = Definition->Stages[StageIndex];
		TestEqual(TEXT("Editor and runtime Stage IDs match"), EditorStage.StageID, RuntimeStage.StageID);
		TestEqual(TEXT("Editor and runtime Step counts match"),
			EditorStage.Steps.Num(), RuntimeStage.Interactions.Num());
		for (int32 StepIndex = 0; StepIndex < EditorStage.Steps.Num(); ++StepIndex)
		{
			const FName ExpectedNext = StepIndex + 1 < EditorStage.Steps.Num()
				? EditorStage.Steps[StepIndex + 1].StepID : NAME_None;
			TestEqual(TEXT("Runtime Next ID follows Editor Flow array order"),
				RuntimeStage.Interactions[StepIndex].NextInteractionID, ExpectedNext);
			if (EditorStage.Steps[StepIndex].StepType != EMainEducationAuthoringStepType::Travel)
			{
				TestFalse(TEXT("Every presentation Step has a player-facing guide"),
					EditorStage.Steps[StepIndex].Content.InteractionGuideText.IsEmpty());
			}
		}
	}

	TArray<FName> TravelRouteIDs;
	TArray<FName> NarrationIDs;
	for (const FScenarioStageDefinition& Stage : Definition->Stages)
	{
		for (const FScenarioInteraction& Interaction : Stage.Interactions)
		{
			if (Interaction.InteractionType == EScenarioInteractionType::Narration)
			{
				NarrationIDs.Add(Interaction.NarrationID);
			}
			if (Interaction.TargetID.ToString().StartsWith(TEXT("Travel_")))
			{
				TravelRouteIDs.Add(Interaction.TargetID);
			}
		}
	}
	TestEqual(TEXT("No travel routes remain"), TravelRouteIDs.Num(), 0);
	TestEqual(TEXT("Only one opening narration remains"), NarrationIDs.Num(), 1);
	if (NarrationIDs.Num() == 1)
	{
		TestEqual(TEXT("Opening narration starts at row 01"), NarrationIDs[0], FName(TEXT("MAIN_NA_01")));
	}
	FMainEducationContent RemovedContent;
	TestFalse(TEXT("Gongsimdon content is absent"), Definition->FindEducationContent(TEXT("QUIZ_GONGSIMDON"), RemovedContent));
	TestFalse(TEXT("Nokro content is absent"), Definition->FindEducationContent(TEXT("QUIZ_NOKRO"), RemovedContent));
	return true;
}

#endif
