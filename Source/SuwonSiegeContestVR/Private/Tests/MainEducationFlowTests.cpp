#if WITH_DEV_AUTOMATION_TESTS

#include "Core/Text/HangulTextLibrary.h"
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

	const TArray<FName> ExpectedStages = {
		TEXT("MAIN_GATE"), TEXT("SINGIJEON"), TEXT("ONGSEONG"), TEXT("SUMMARY")};
	TestEqual(TEXT("Gate, two experiences and the closing stage"), Definition->Stages.Num(), ExpectedStages.Num());
	TestEqual(TEXT("Editor Flow mirrors the runtime stages"), Definition->EditorFlow.Num(), Definition->Stages.Num());
	for (int32 Index = 0; Index < ExpectedStages.Num() && Index < Definition->Stages.Num(); ++Index)
	{
		TestEqual(TEXT("Stages run in the authored order"), Definition->Stages[Index].StageID, ExpectedStages[Index]);
	}

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

	// Each experience is reached the same way: explain it, ask its quiz, then travel.
	struct FExpectedRoute
	{
		FName StageID;
		FName NarrationStart;
		FName QuizID;
		FName RouteID;
		FString Consonants;
		FString Answer;
	};
	const TArray<FExpectedRoute> ExpectedRoutes = {
		{TEXT("SINGIJEON"), TEXT("MAIN_NA_13"), TEXT("QUIZ_HWACHA"), TEXT("Travel_Singijeon"), TEXT("ㅎ ㅊ"), TEXT("화차")},
		{TEXT("ONGSEONG"), TEXT("MAIN_NA_18"), TEXT("QUIZ_ONGSEONG"), TEXT("Travel_Ongseong"), TEXT("ㅇ ㅅ"), TEXT("옹성")}
	};

	for (const FExpectedRoute& Expected : ExpectedRoutes)
	{
		const FScenarioStageDefinition* Stage = Definition->Stages.FindByPredicate(
			[&Expected](const FScenarioStageDefinition& Candidate) { return Candidate.StageID == Expected.StageID; });
		if (!TestNotNull(TEXT("The experience stage exists"), Stage))
		{
			continue;
		}

		const int32 NarrationIndex = Stage->Interactions.IndexOfByPredicate(
			[](const FScenarioInteraction& Interaction)
			{
				return Interaction.InteractionType == EScenarioInteractionType::Narration;
			});
		const int32 QuizIndex = Stage->Interactions.IndexOfByPredicate(
			[](const FScenarioInteraction& Interaction)
			{
				return Interaction.InteractionType == EScenarioInteractionType::Quiz;
			});
		const int32 TravelIndex = Stage->Interactions.IndexOfByPredicate(
			[&Expected](const FScenarioInteraction& Interaction) { return Interaction.TargetID == Expected.RouteID; });

		TestTrue(TEXT("The stage explains, quizzes and then travels, in that order"),
			NarrationIndex != INDEX_NONE && QuizIndex == NarrationIndex + 1 && TravelIndex == QuizIndex + 1);
		if (NarrationIndex != INDEX_NONE)
		{
			TestEqual(TEXT("The explanation narration starts at the authored row"),
				Stage->Interactions[NarrationIndex].NarrationID, Expected.NarrationStart);
		}
		if (QuizIndex != INDEX_NONE)
		{
			TestEqual(TEXT("The quiz step points at its quiz content"),
				Stage->Interactions[QuizIndex].TargetID, Expected.QuizID);
		}
		if (TravelIndex != INDEX_NONE)
		{
			// BeginExperienceTravel stores the following step as the return checkpoint, so a Travel
			// step that ends its stage cannot travel at all.
			TestTrue(TEXT("A return step follows the travel step"),
				TravelIndex + 1 < Stage->Interactions.Num());
			TestFalse(TEXT("The travel step carries a return checkpoint"),
				Stage->Interactions[TravelIndex].NextInteractionID.IsNone());
		}

		FMainEducationContent Quiz;
		if (TestTrue(TEXT("The quiz content exists"), Definition->FindEducationContent(Expected.QuizID, Quiz)))
		{
			TestEqual(TEXT("The quiz is authored as a quiz"), Quiz.ContentType, EMainEducationContentType::Quiz);
			TestEqual(TEXT("The quiz shows the expected consonants"),
				Quiz.InitialConsonants.ToString(), Expected.Consonants);
			TestEqual(TEXT("The consonants match the ones derived from the answer"),
				UHangulTextLibrary::ExtractInitialConsonants(Expected.Answer), Expected.Consonants);
			TestTrue(TEXT("The spoken answer is accepted"),
				Definition->IsAcceptedQuizAnswer(Expected.QuizID, Expected.Answer));
			TestFalse(TEXT("Another word is not accepted"),
				Definition->IsAcceptedQuizAnswer(Expected.QuizID, TEXT("성벽")));
		}

		FMainEducationExperienceRoute Route;
		if (TestTrue(TEXT("The travel route exists"), Definition->FindExperienceRoute(Expected.RouteID, Route)))
		{
			TestFalse(TEXT("The route points at an Experience asset"), Route.Experience.IsNull());
		}
	}

	TestEqual(TEXT("Only the two connected experiences are routed"), Definition->ExperienceRoutes.Num(), 2);

	FMainEducationContent RemovedContent;
	TestFalse(TEXT("Gongsimdon content is absent"), Definition->FindEducationContent(TEXT("GONGSIMDON_IMAGE"), RemovedContent));
	TestFalse(TEXT("Nokro content is absent"), Definition->FindEducationContent(TEXT("NOKRO_IMAGE"), RemovedContent));
	return true;
}

#endif
