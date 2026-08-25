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
	TestEqual(TEXT("Intro, four subjects, and summary stages"), Definition->Stages.Num(), 6);
	TestEqual(TEXT("Editor Flow mirrors the six runtime stages"), Definition->EditorFlow.Num(), 6);
	TestEqual(TEXT("Five experience route slots"), Definition->ExperienceRoutes.Num(), 5);

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

	TMap<FName, FName> NarrationStarts;
	for (const FScenarioStageDefinition& Stage : Definition->Stages)
	{
		for (const FScenarioInteraction& Interaction : Stage.Interactions)
		{
			if (Interaction.InteractionType == EScenarioInteractionType::Narration)
			{
				NarrationStarts.Add(Interaction.InteractionID, Interaction.NarrationID);
			}
		}
	}
	TestEqual(TEXT("Fourteen education narration segments"), NarrationStarts.Num(), 14);
	TestEqual(TEXT("Intro starts at Main narration row 01"),
		NarrationStarts.FindRef(TEXT("INTRO_01")), FName(TEXT("MAIN_NA_01")));
	TestEqual(TEXT("Nokro explanation ends with row 33 segment"),
		NarrationStarts.FindRef(TEXT("NOKRO_IMAGE_02")), FName(TEXT("MAIN_NA_33")));

	FMainEducationContent Quiz;
	TestTrue(TEXT("Gongsimdon quiz exists"), Definition->FindEducationContent(TEXT("QUIZ_GONGSIMDON"), Quiz));
	TestEqual(TEXT("Gongsimdon initial consonants"), Quiz.InitialConsonants.ToString(), FString(TEXT("ㄱ ㅅ ㄷ")));
	TestTrue(TEXT("Canonical answer is accepted"),
		Definition->IsAcceptedQuizAnswer(TEXT("QUIZ_GONGSIMDON"), TEXT("공심돈")));
	TestTrue(TEXT("Whitespace around answer is ignored"),
		Definition->IsAcceptedQuizAnswer(TEXT("QUIZ_GONGSIMDON"), TEXT(" 공 심 돈 ")));
	TestFalse(TEXT("Wrong answer is rejected"),
		Definition->IsAcceptedQuizAnswer(TEXT("QUIZ_GONGSIMDON"), TEXT("옹성")));

	TestTrue(TEXT("Ongseong answer is accepted"),
		Definition->IsAcceptedQuizAnswer(TEXT("QUIZ_ONGSEONG"), TEXT("옹성")));
	TestTrue(TEXT("Nokro answer is accepted"),
		Definition->IsAcceptedQuizAnswer(TEXT("QUIZ_NOKRO"), TEXT("녹로")));
	TestTrue(TEXT("Geojunggi answer is accepted"),
		Definition->IsAcceptedQuizAnswer(TEXT("QUIZ_GEOJUNGGI"), TEXT("거중기")));
	return true;
}

#endif
