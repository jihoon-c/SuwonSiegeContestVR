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
	const FMainEducationAuthoringStage* GeojunggiAndNokroStage = Definition->EditorFlow.FindByPredicate(
		[](const FMainEducationAuthoringStage& Stage)
		{
			return Stage.StageID == TEXT("GEO_NOKRO");
		});
	TestNotNull(TEXT("Geojunggi and Nokro stage exists"), GeojunggiAndNokroStage);
	if (GeojunggiAndNokroStage)
	{
		TestEqual(TEXT("Geojunggi and Nokro include their narration beats"),
			GeojunggiAndNokroStage->Steps.Num(), 4);
		if (GeojunggiAndNokroStage->Steps.Num() == 4)
		{
			TestEqual(TEXT("Geojunggi is presented first"),
				GeojunggiAndNokroStage->Steps[0].StepID, FName(TEXT("GEOJUNGGI_IMAGE")));
			TestEqual(TEXT("Geojunggi narration starts from Narration2 row 02"),
				GeojunggiAndNokroStage->Steps[1].NarrationStartRow, FName(TEXT("MAIN_NA_02")));
			TestEqual(TEXT("Nokro is presented after the Geojunggi narration"),
				GeojunggiAndNokroStage->Steps[2].StepID, FName(TEXT("NOKRO_IMAGE")));
			TestEqual(TEXT("Nokro narration starts from Narration2 row 05"),
				GeojunggiAndNokroStage->Steps[3].NarrationStartRow, FName(TEXT("MAIN_NA_05")));
			TestTrue(TEXT("Nokro study image is assigned"),
				GeojunggiAndNokroStage->Steps[2].Content.Image.ToSoftObjectPath().ToString().Contains(TEXT("/study/study_nokro")));
		}
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
	const TArray<FName> ExpectedNarrationStarts = {
		TEXT("MAIN_NA_01"), TEXT("MAIN_NA_02"), TEXT("MAIN_NA_05"),
		TEXT("MAIN_NA_08"), TEXT("MAIN_NA_13"), TEXT("MAIN_NA_18"), TEXT("MAIN_NA_25")};
	TestEqual(TEXT("Narration2 is divided at the presentation transition points"), NarrationIDs.Num(), ExpectedNarrationStarts.Num());
	if (NarrationIDs.Num() == ExpectedNarrationStarts.Num())
	{
		for (int32 Index = 0; Index < ExpectedNarrationStarts.Num(); ++Index)
		{
			TestEqual(TEXT("Narration2 starts are in presentation order"), NarrationIDs[Index], ExpectedNarrationStarts[Index]);
		}
	}
	FMainEducationContent RemovedContent;
	TestFalse(TEXT("Gongsimdon content is absent"), Definition->FindEducationContent(TEXT("QUIZ_GONGSIMDON"), RemovedContent));
	TestFalse(TEXT("Nokro content is absent"), Definition->FindEducationContent(TEXT("QUIZ_NOKRO"), RemovedContent));
	return true;
}

#endif
