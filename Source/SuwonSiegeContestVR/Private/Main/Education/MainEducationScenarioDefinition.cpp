#include "Main/Education/MainEducationScenarioDefinition.h"

#include "Core/Experience/ExperienceDefinition.h"

#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

namespace MainEducationDefaults
{
	FScenarioInteraction MakeStep(
		const TCHAR* InteractionID,
		const EScenarioInteractionType Type,
		const TCHAR* TargetID,
		const TCHAR* NextInteractionID = TEXT(""))
	{
		FScenarioInteraction Step;
		Step.InteractionID = FName(InteractionID);
		Step.InteractionType = Type;
		Step.TargetID = FName(TargetID);
		Step.GuideAction = Type == EScenarioInteractionType::Quiz
			? EScenarioGuideAction::Speak : EScenarioGuideAction::Hidden;
		Step.NextInteractionID = FName(NextInteractionID);
		return Step;
	}

	FScenarioInteraction MakeNarrationStep(
		const TCHAR* InteractionID,
		const TCHAR* ContentID,
		const TCHAR* NarrationID,
		const TCHAR* NextInteractionID = TEXT(""))
	{
		FScenarioInteraction Step = MakeStep(
			InteractionID, EScenarioInteractionType::Narration, ContentID, NextInteractionID);
		Step.NarrationID = FName(NarrationID);
		return Step;
	}

	FScenarioStageDefinition MakeStage(
		const TCHAR* StageID,
		const TCHAR* StageName,
		const TCHAR* NextStageID,
		TArray<FScenarioInteraction>&& Interactions)
	{
		FScenarioStageDefinition Stage;
		Stage.StageID = FName(StageID);
		Stage.StageName = FText::FromString(StageName);
		Stage.StartInteractionID = Interactions.IsEmpty() ? NAME_None : Interactions[0].InteractionID;
		Stage.NextStageID = FName(NextStageID);
		Stage.Interactions = MoveTemp(Interactions);
		return Stage;
	}

	FMainEducationContent MakeContent(
		const TCHAR* ID,
		const EMainEducationContentType Type,
		const TCHAR* Title,
		const TCHAR* Body,
		const TCHAR* Highlight = TEXT(""),
		std::initializer_list<const TCHAR*> Callouts = {})
	{
		FMainEducationContent Content;
		Content.ContentID = FName(ID);
		Content.ContentType = Type;
		Content.Title = FText::FromString(Title);
		Content.SpeakerName = Type == EMainEducationContentType::Instructor
			? FText::FromString(TEXT("교관")) : FText::GetEmpty();
		Content.Body = FText::FromString(Body);
		Content.HighlightText = FText::FromString(Highlight);
		for (const TCHAR* Callout : Callouts)
		{
			Content.Callouts.Add(FText::FromString(Callout));
		}
		return Content;
	}

	FMainEducationContent MakeQuiz(
		const TCHAR* ID,
		const TCHAR* Question,
		const TCHAR* InitialConsonants,
		const TCHAR* Answer)
	{
		FMainEducationContent Content = MakeContent(
			ID, EMainEducationContentType::Quiz, TEXT("초성 퀴즈"), Question);
		Content.InitialConsonants = FText::FromString(InitialConsonants);
		Content.AcceptedAnswers.Add(FText::FromString(Answer));
		return Content;
	}
}

UMainEducationScenarioDefinition::UMainEducationScenarioDefinition()
{
	BuildDefaultContent();
	BuildEditorFlowFromRuntime();
	RebuildRuntimeFromEditorFlow();
}

void UMainEducationScenarioDefinition::RebuildScenarioFromEditorFlow()
{
#if WITH_EDITOR
	Modify();
#endif
	RebuildRuntimeFromEditorFlow();

	FString Error;
	if (!ValidateEducationScenario(Error))
	{
		UE_LOG(LogTemp, Error, TEXT("Main education Editor Flow rebuild failed validation: %s"), *Error);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Main education Editor Flow rebuilt: %d stages, %d presentation entries."),
			Stages.Num(), EducationContent.Num());
	}
#if WITH_EDITOR
	MarkPackageDirty();
#endif
}

void UMainEducationScenarioDefinition::ResetToSingijeonAndOngseongFlow()
{
 	ResetToMainGatePresentationFlow();
}

void UMainEducationScenarioDefinition::ResetToMainGatePresentationFlow()
{
#if WITH_EDITOR
	Modify();
#endif
	BuildDefaultContent();
	BuildEditorFlowFromRuntime();
	RebuildRuntimeFromEditorFlow();
#if WITH_EDITOR
	MarkPackageDirty();
#endif
}

void UMainEducationScenarioDefinition::BuildEditorFlowFromRuntime()
{
	EditorFlow.Reset();
	for (const FScenarioStageDefinition& RuntimeStage : Stages)
	{
		FMainEducationAuthoringStage AuthoringStage;
		AuthoringStage.StageID = RuntimeStage.StageID;
		AuthoringStage.StageName = RuntimeStage.StageName;
		for (const FScenarioInteraction& Interaction : RuntimeStage.Interactions)
		{
			FMainEducationAuthoringStep Step;
			Step.StepID = Interaction.InteractionID;
			Step.NarrationStartRow = Interaction.NarrationID;
			Step.WorldGuideAction = Interaction.GuideAction;
			Step.WorldGuideText = Interaction.GuideText;
			Step.DelayBeforeStart = Interaction.DelayBeforeStart;
			Step.DelayAfterComplete = Interaction.DelayAfterComplete;
			Step.bRequired = Interaction.bRequired;

			if (Interaction.InteractionType == EScenarioInteractionType::Narration)
			{
				Step.StepType = EMainEducationAuthoringStepType::Narration;
				Step.PlayerAction = FText::FromString(TEXT("교관 설명을 듣고 현재 교육 화면을 확인합니다."));
				Step.CompletionCondition = FText::FromString(TEXT("지정된 나레이션 Row 구간 재생 완료"));
			}
			else if (Interaction.InteractionType == EScenarioInteractionType::Quiz)
			{
				Step.StepType = EMainEducationAuthoringStepType::Quiz;
				Step.PlayerAction = FText::FromString(TEXT("초성을 보고 정답을 말하거나 입력합니다."));
				Step.CompletionCondition = FText::FromString(TEXT("Accepted Answers 중 하나와 일치"));
			}
			else if (Interaction.TargetID.ToString().StartsWith(TEXT("Travel_")))
			{
				Step.StepType = EMainEducationAuthoringStepType::Travel;
				Step.ExperienceRouteID = Interaction.TargetID;
				Step.PlayerAction = FText::FromString(TEXT("연결된 체험 레벨로 이동합니다."));
				Step.CompletionCondition = FText::FromString(TEXT("체험 완료 후 다음 Step 체크포인트로 복귀"));
			}
			else
			{
				Step.StepType = EMainEducationAuthoringStepType::Presentation;
				Step.PlayerAction = FText::FromString(TEXT("이미지와 설명을 확인하고 확인 입력을 누릅니다."));
				Step.CompletionCondition = FText::FromString(TEXT("Continue Presentation 호출"));
			}

			if (Step.StepType != EMainEducationAuthoringStepType::Travel)
			{
				FindEducationContent(Interaction.TargetID, Step.Content);
				if (Step.Content.ContentID.IsNone())
				{
					Step.Content.ContentID = Interaction.TargetID.IsNone()
						? Interaction.InteractionID : Interaction.TargetID;
				}
				if (Step.Content.InteractionGuideText.IsEmpty())
				{
					switch (Step.StepType)
					{
					case EMainEducationAuthoringStepType::Narration:
						Step.Content.InteractionGuideText = FText::FromString(TEXT("나레이션을 들어보세요."));
						break;
					case EMainEducationAuthoringStepType::Quiz:
						Step.Content.InteractionGuideText = FText::FromString(TEXT("정답을 말하거나 입력하세요."));
						break;
					default:
						Step.Content.InteractionGuideText = FText::FromString(TEXT("내용을 확인한 뒤 확인을 누르세요."));
						break;
					}
				}
			}
			AuthoringStage.Steps.Add(MoveTemp(Step));
		}
		EditorFlow.Add(MoveTemp(AuthoringStage));
	}
}

void UMainEducationScenarioDefinition::RebuildRuntimeFromEditorFlow()
{
	if (bIsRebuildingEditorFlow)
	{
		return;
	}
	TGuardValue<bool> RebuildGuard(bIsRebuildingEditorFlow, true);

	Stages.Reset();
	EducationContent.Reset();
	for (int32 StageIndex = 0; StageIndex < EditorFlow.Num(); ++StageIndex)
	{
		const FMainEducationAuthoringStage& SourceStage = EditorFlow[StageIndex];
		FScenarioStageDefinition RuntimeStage;
		RuntimeStage.StageID = SourceStage.StageID;
		RuntimeStage.StageName = SourceStage.StageName;
		RuntimeStage.StartInteractionID = SourceStage.Steps.IsEmpty()
			? NAME_None : SourceStage.Steps[0].StepID;
		RuntimeStage.NextStageID = StageIndex + 1 < EditorFlow.Num()
			? EditorFlow[StageIndex + 1].StageID : NAME_None;

		for (int32 StepIndex = 0; StepIndex < SourceStage.Steps.Num(); ++StepIndex)
		{
			const FMainEducationAuthoringStep& SourceStep = SourceStage.Steps[StepIndex];
			FScenarioInteraction Interaction;
			Interaction.InteractionID = SourceStep.StepID;
			Interaction.NextInteractionID = StepIndex + 1 < SourceStage.Steps.Num()
				? SourceStage.Steps[StepIndex + 1].StepID : NAME_None;
			Interaction.GuideAction = SourceStep.WorldGuideAction;
			Interaction.GuideText = SourceStep.WorldGuideText;
			Interaction.DelayBeforeStart = SourceStep.DelayBeforeStart;
			Interaction.DelayAfterComplete = SourceStep.DelayAfterComplete;
			Interaction.bRequired = SourceStep.bRequired;

			if (SourceStep.StepType == EMainEducationAuthoringStepType::Travel)
			{
				Interaction.InteractionType = EScenarioInteractionType::Custom;
				Interaction.TargetID = SourceStep.ExperienceRouteID;
			}
			else
			{
				FMainEducationContent Content = SourceStep.Content;
				if (Content.ContentID.IsNone())
				{
					Content.ContentID = SourceStep.StepID;
				}
				Interaction.TargetID = Content.ContentID;
				// Neighbouring steps intentionally share one screen (an image and its narration),
				// so the same ContentID arrives more than once. Lookups already resolve to the
				// first entry; keeping only that one also keeps ValidateEducationScenario honest.
				if (!EducationContent.ContainsByPredicate(
					[&Content](const FMainEducationContent& Existing) { return Existing.ContentID == Content.ContentID; }))
				{
					EducationContent.Add(Content);
				}

				switch (SourceStep.StepType)
				{
				case EMainEducationAuthoringStepType::Narration:
					Interaction.InteractionType = EScenarioInteractionType::Narration;
					Interaction.NarrationID = SourceStep.NarrationStartRow;
					break;
				case EMainEducationAuthoringStepType::Quiz:
					Interaction.InteractionType = EScenarioInteractionType::Quiz;
					break;
				case EMainEducationAuthoringStepType::Presentation:
				default:
					Interaction.InteractionType = EScenarioInteractionType::Custom;
					break;
				}
			}
			RuntimeStage.Interactions.Add(MoveTemp(Interaction));
		}
		Stages.Add(MoveTemp(RuntimeStage));
	}
	StartStageID = EditorFlow.IsEmpty() ? NAME_None : EditorFlow[0].StageID;
}

#if WITH_EDITOR
void UMainEducationScenarioDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (bAutoRebuildFromEditorFlow && !bIsRebuildingEditorFlow &&
		PropertyChangedEvent.GetMemberPropertyName() ==
		GET_MEMBER_NAME_CHECKED(UMainEducationScenarioDefinition, EditorFlow))
	{
		RebuildRuntimeFromEditorFlow();
		MarkPackageDirty();
	}
}
#endif

bool UMainEducationScenarioDefinition::FindEducationContent(
	const FName ContentID, FMainEducationContent& OutContent) const
{
	if (const FMainEducationContent* Found = EducationContent.FindByPredicate(
		[ContentID](const FMainEducationContent& Item) { return Item.ContentID == ContentID; }))
	{
		OutContent = *Found;
		return true;
	}
	OutContent = FMainEducationContent();
	return false;
}

bool UMainEducationScenarioDefinition::FindExperienceRoute(
	const FName RouteID, FMainEducationExperienceRoute& OutRoute) const
{
	if (const FMainEducationExperienceRoute* Found = ExperienceRoutes.FindByPredicate(
		[RouteID](const FMainEducationExperienceRoute& Item) { return Item.RouteID == RouteID; }))
	{
		OutRoute = *Found;
		return true;
	}
	OutRoute = FMainEducationExperienceRoute();
	return false;
}

bool UMainEducationScenarioDefinition::IsAcceptedQuizAnswer(
	const FName QuizID, const FString& Answer) const
{
	FMainEducationContent Quiz;
	if (!FindEducationContent(QuizID, Quiz) || Quiz.ContentType != EMainEducationContentType::Quiz)
	{
		return false;
	}

	const FString NormalizedAnswer = NormalizeAnswer(Answer);
	return !NormalizedAnswer.IsEmpty() && Quiz.AcceptedAnswers.ContainsByPredicate(
		[&NormalizedAnswer](const FText& Candidate)
		{
			return NormalizeAnswer(Candidate.ToString()) == NormalizedAnswer;
		});
}

bool UMainEducationScenarioDefinition::ValidateEducationScenario(FString& OutError) const
{
	if (!ValidateScenario(OutError))
	{
		return false;
	}

	TSet<FName> ContentIDs;
	for (const FMainEducationContent& Content : EducationContent)
	{
		if (Content.ContentID.IsNone() || ContentIDs.Contains(Content.ContentID))
		{
			OutError = FString::Printf(TEXT("Education content has an empty or duplicate ID: %s"),
				*Content.ContentID.ToString());
			return false;
		}
		if (Content.ContentType == EMainEducationContentType::Quiz && Content.AcceptedAnswers.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Quiz %s has no accepted answer."), *Content.ContentID.ToString());
			return false;
		}
		ContentIDs.Add(Content.ContentID);
	}

	TSet<FName> RouteIDs;
	for (const FMainEducationExperienceRoute& Route : ExperienceRoutes)
	{
		if (Route.RouteID.IsNone() || RouteIDs.Contains(Route.RouteID))
		{
			OutError = FString::Printf(TEXT("Experience route has an empty or duplicate ID: %s"),
				*Route.RouteID.ToString());
			return false;
		}
		RouteIDs.Add(Route.RouteID);
	}

	for (const FScenarioStageDefinition& Stage : Stages)
	{
		for (const FScenarioInteraction& Interaction : Stage.Interactions)
		{
			if (Interaction.InteractionType == EScenarioInteractionType::Narration &&
				Interaction.NarrationID.IsNone())
			{
				OutError = FString::Printf(TEXT("Narration interaction %s has no NarrationStartRow."),
					*Interaction.InteractionID.ToString());
				return false;
			}
			if (Interaction.InteractionType == EScenarioInteractionType::Quiz)
			{
				FMainEducationContent Content;
				if (!FindEducationContent(Interaction.TargetID, Content) ||
					Content.ContentType != EMainEducationContentType::Quiz)
				{
					OutError = FString::Printf(TEXT("Quiz interaction %s has no matching quiz content %s."),
						*Interaction.InteractionID.ToString(), *Interaction.TargetID.ToString());
					return false;
				}
			}
			else if (Interaction.TargetID.ToString().StartsWith(TEXT("Travel_")))
			{
				if (!RouteIDs.Contains(Interaction.TargetID))
				{
					OutError = FString::Printf(TEXT("Travel interaction %s has no route %s."),
						*Interaction.InteractionID.ToString(), *Interaction.TargetID.ToString());
					return false;
				}
			}
			else if (!ContentIDs.Contains(Interaction.TargetID))
			{
				OutError = FString::Printf(TEXT("Interaction %s has no presentation content %s."),
					*Interaction.InteractionID.ToString(), *Interaction.TargetID.ToString());
				return false;
			}
		}
	}

	if (!EditorFlow.IsEmpty())
	{
		if (EditorFlow.Num() != Stages.Num())
		{
			OutError = TEXT("Editor Flow stage count differs from generated Runtime Stages. Use Rebuild Scenario From Editor Flow.");
			return false;
		}
		for (int32 StageIndex = 0; StageIndex < EditorFlow.Num(); ++StageIndex)
		{
			if (EditorFlow[StageIndex].Steps.Num() != Stages[StageIndex].Interactions.Num())
			{
				OutError = FString::Printf(TEXT("Editor Flow step count differs in stage %s. Use Rebuild Scenario From Editor Flow."),
					*EditorFlow[StageIndex].StageID.ToString());
				return false;
			}
			for (const FMainEducationAuthoringStep& Step : EditorFlow[StageIndex].Steps)
			{
				if (Step.PlayerAction.IsEmpty() || Step.CompletionCondition.IsEmpty())
				{
					OutError = FString::Printf(TEXT("Editor Flow step %s needs PlayerAction and CompletionCondition."),
						*Step.StepID.ToString());
					return false;
				}
				if (Step.StepType != EMainEducationAuthoringStepType::Travel &&
					Step.Content.InteractionGuideText.IsEmpty())
				{
					OutError = FString::Printf(TEXT("Editor Flow step %s has no player-facing interaction guide."),
						*Step.StepID.ToString());
					return false;
				}
			}
		}
	}

	OutError.Reset();
	return true;
}

void UMainEducationScenarioDefinition::BuildDefaultContent()
{
	using namespace MainEducationDefaults;

	ScenarioID = TEXT("SCENARIO_MainEducation");
	ScenarioName = FText::FromString(TEXT("수원화성 신임 지휘관 교육"));
	StartStageID = TEXT("MAIN_GATE");

	// Each experience is reached the same way: explain it, ask its initial-consonant quiz by voice,
	// then travel. A Travel step is never last in its Stage - BeginExperienceTravel stores the
	// following step as the return checkpoint, so AFTER_* is what the player comes back to.
	Stages = {
		MakeStage(TEXT("MAIN_GATE"), TEXT("성문 앞 안내"), TEXT("SINGIJEON"), {
			MakeNarrationStep(TEXT("GATE_GREETING"), TEXT("GATE_GREETING"), TEXT("MAIN_NA_01"))
		}),
		MakeStage(TEXT("SINGIJEON"), TEXT("신기전과 화차"), TEXT("ONGSEONG"), {
			MakeStep(TEXT("SINGIJEON_IMAGE"), EScenarioInteractionType::Custom, TEXT("SINGIJEON_IMAGE"), TEXT("SINGIJEON_NARRATION")),
			MakeNarrationStep(TEXT("SINGIJEON_NARRATION"), TEXT("SINGIJEON_IMAGE"), TEXT("MAIN_NA_13"), TEXT("SINGIJEON_QUIZ")),
			MakeStep(TEXT("SINGIJEON_QUIZ"), EScenarioInteractionType::Quiz, TEXT("QUIZ_HWACHA"), TEXT("TRAVEL_SINGIJEON")),
			MakeStep(TEXT("TRAVEL_SINGIJEON"), EScenarioInteractionType::Custom, TEXT("Travel_Singijeon"), TEXT("AFTER_SINGIJEON")),
			MakeStep(TEXT("AFTER_SINGIJEON"), EScenarioInteractionType::Custom, TEXT("AFTER_SINGIJEON"))
		}),
		MakeStage(TEXT("ONGSEONG"), TEXT("옹성"), TEXT("SUMMARY"), {
			MakeStep(TEXT("ONGSEONG_IMAGE"), EScenarioInteractionType::Custom, TEXT("ONGSEONG_IMAGE"), TEXT("ONGSEONG_NARRATION")),
			MakeNarrationStep(TEXT("ONGSEONG_NARRATION"), TEXT("ONGSEONG_IMAGE"), TEXT("MAIN_NA_18"), TEXT("ONGSEONG_QUIZ")),
			MakeStep(TEXT("ONGSEONG_QUIZ"), EScenarioInteractionType::Quiz, TEXT("QUIZ_ONGSEONG"), TEXT("TRAVEL_ONGSEONG")),
			MakeStep(TEXT("TRAVEL_ONGSEONG"), EScenarioInteractionType::Custom, TEXT("Travel_Ongseong"), TEXT("AFTER_ONGSEONG")),
			MakeStep(TEXT("AFTER_ONGSEONG"), EScenarioInteractionType::Custom, TEXT("AFTER_ONGSEONG"))
		}),
		MakeStage(TEXT("SUMMARY"), TEXT("교육 마무리"), TEXT(""), {
			MakeStep(TEXT("SUMMARY_01"), EScenarioInteractionType::Custom, TEXT("SUMMARY_01"), TEXT("SUMMARY_NARRATION")),
			MakeNarrationStep(TEXT("SUMMARY_NARRATION"), TEXT("SUMMARY_01"), TEXT("MAIN_NA_25"))
		})
	};

	EducationContent = {
		MakeContent(TEXT("GATE_GREETING"), EMainEducationContentType::Instructor, TEXT("수원화성 성문 앞"), TEXT("신임 지휘관님, 수원화성에 오신 것을 환영합니다. 이곳에서 주요 시설과 장치를 하나씩 살펴보겠습니다."), TEXT("성문 앞 교육 시작")),
		MakeContent(TEXT("SINGIJEON_IMAGE"), EMainEducationContentType::Image, TEXT("신기전"), TEXT("신기전은 화약의 힘으로 화살을 멀리 보내는 무기로, 성벽 방어 상황에서 적의 접근을 막는 데 활용할 수 있습니다."), TEXT("화약 무기"), {TEXT("화약"), TEXT("화살"), TEXT("성벽 방어")} ),
		// The narration right before this quiz ends on the very sentence that names 화차 (row 16).
		MakeQuiz(TEXT("QUIZ_HWACHA"), TEXT("여러 발의 신기전을 한꺼번에 발사하기 위해 만든 이동식 발사대의 이름은?"), TEXT("ㅎ ㅊ"), TEXT("화차")),
		MakeContent(TEXT("AFTER_SINGIJEON"), EMainEducationContentType::Instructor, TEXT("신기전 체험 완료"), TEXT("화차에 장착한 신기전으로 성벽을 방어해 보았습니다. 이제 성문을 지키는 방어시설을 살펴보겠습니다."), TEXT("화약 무기 체험 완료")),
		MakeContent(TEXT("ONGSEONG_IMAGE"), EMainEducationContentType::Image, TEXT("옹성"), TEXT("옹성은 성문 바깥을 다시 둘러싼 방어시설입니다. 적이 성문으로 곧바로 접근하기 어렵게 만들어 방어에 유리했습니다."), TEXT("성문 보호"), {TEXT("옹성"), TEXT("성문"), TEXT("접근 지연")} ),
		MakeQuiz(TEXT("QUIZ_ONGSEONG"), TEXT("성문 바깥을 한 겹 더 둘러싸 지키는 이 방어시설의 이름은?"), TEXT("ㅇ ㅅ"), TEXT("옹성")),
		MakeContent(TEXT("AFTER_ONGSEONG"), EMainEducationContentType::Instructor, TEXT("옹성 체험 완료"), TEXT("옹성이 성문으로 향하는 적의 진입을 어떻게 지연시키는지 직접 확인했습니다."), TEXT("성문 방어 체험 완료")),
		MakeContent(TEXT("SUMMARY_01"), EMainEducationContentType::Summary, TEXT("교육 마무리"), TEXT("성문 앞에서 수원화성의 방어를 살펴보았습니다. 화차에 장착한 신기전으로 접근하는 적을 막고, 옹성으로 성문을 한 번 더 보호했던 방식을 기억해 두십시오."), TEXT("수원화성 방어 체계"), {TEXT("신기전과 화차"), TEXT("옹성")} )
	};
	auto AssignImage = [this](const FName ContentID, const TCHAR* AssetPath)
	{
		for (FMainEducationContent& Content : EducationContent)
		{
			if (Content.ContentID == ContentID)
			{
				Content.Image = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(AssetPath));
				break;
			}
		}
	};
	// The uploaded study artwork is used directly for the matching presentation beats.
	AssignImage(TEXT("SINGIJEON_IMAGE"), TEXT("/Game/Art/MainEducation/study/study_singijeon.study_singijeon"));
	AssignImage(TEXT("ONGSEONG_IMAGE"), TEXT("/Game/Art/MainEducation/study/study_ongsung.study_ongsung"));
	AssignImage(TEXT("SUMMARY_01"), TEXT("/Game/Art/MainEducation/Examples/T_MainEdu_Overview_Example.T_MainEdu_Overview_Example"));

	// Main reaches an experience through its Core Experience asset only; no Game Feature is named here.
	ExperienceRoutes.Reset();
	auto AddRoute = [this](const TCHAR* RouteID, const TCHAR* AssetPath)
	{
		FMainEducationExperienceRoute Route;
		Route.RouteID = FName(RouteID);
		Route.Experience = TSoftObjectPtr<UExperienceDefinition>(FSoftObjectPath(AssetPath));
		ExperienceRoutes.Add(Route);
	};
	AddRoute(TEXT("Travel_Singijeon"), TEXT("/Game/Core/Experience/Definitions/DA_Experience_Singijeon.DA_Experience_Singijeon"));
	AddRoute(TEXT("Travel_Ongseong"), TEXT("/Game/Core/Experience/Definitions/DA_Experience_Ongseong.DA_Experience_Ongseong"));
}

/* Legacy full-course data kept in source history only. */
#if 0
	Stages = {
		MakeStage(TEXT("MAIN_INTRO"), TEXT("부임과 성벽 방어"), TEXT("GONGSIMDON"), {
			MakeNarrationStep(TEXT("INTRO_01"), TEXT("INTRO_01"), TEXT("MAIN_NA_01"), TEXT("INTRO_OVERVIEW")),
			MakeStep(TEXT("INTRO_OVERVIEW"), EScenarioInteractionType::Custom, TEXT("INTRO_OVERVIEW"), TEXT("DEFENSE_01")),
			MakeNarrationStep(TEXT("DEFENSE_01"), TEXT("DEFENSE_01"), TEXT("MAIN_NA_04"), TEXT("DEFENSE_IMAGE")),
			MakeStep(TEXT("DEFENSE_IMAGE"), EScenarioInteractionType::Custom, TEXT("DEFENSE_IMAGE"), TEXT("SINGIJEON_BRIEF")),
			MakeNarrationStep(TEXT("SINGIJEON_BRIEF"), TEXT("SINGIJEON_BRIEF"), TEXT("MAIN_NA_08"), TEXT("TRAVEL_SINGIJEON")),
			MakeStep(TEXT("TRAVEL_SINGIJEON"), EScenarioInteractionType::Custom, TEXT("Travel_Singijeon"), TEXT("AFTER_SINGIJEON")),
			MakeStep(TEXT("AFTER_SINGIJEON"), EScenarioInteractionType::Custom, TEXT("AFTER_SINGIJEON"))
		}),
		MakeStage(TEXT("GONGSIMDON"), TEXT("공심돈"), TEXT("ONGSEONG"), {
			MakeNarrationStep(TEXT("GONG_SITUATION"), TEXT("GONG_SITUATION"), TEXT("MAIN_NA_09"), TEXT("GONG_QUIZ")),
			MakeStep(TEXT("GONG_QUIZ"), EScenarioInteractionType::Quiz, TEXT("QUIZ_GONGSIMDON"), TEXT("GONG_ANSWER")),
			MakeNarrationStep(TEXT("GONG_ANSWER"), TEXT("GONG_ANSWER"), TEXT("MAIN_NA_12"), TEXT("GONG_IMAGE_01")),
			MakeNarrationStep(TEXT("GONG_IMAGE_01"), TEXT("GONG_IMAGE_01"), TEXT("MAIN_NA_14"), TEXT("GONG_IMAGE_02")),
			MakeStep(TEXT("GONG_IMAGE_02"), EScenarioInteractionType::Custom, TEXT("GONG_IMAGE_02"), TEXT("GONG_TRAVEL_BRIEF")),
			MakeNarrationStep(TEXT("GONG_TRAVEL_BRIEF"), TEXT("GONG_TRAVEL_BRIEF"), TEXT("MAIN_NA_15"), TEXT("TRAVEL_GONGSIMDON")),
			MakeStep(TEXT("TRAVEL_GONGSIMDON"), EScenarioInteractionType::Custom, TEXT("Travel_Gongsimdon"), TEXT("AFTER_GONGSIMDON")),
			MakeStep(TEXT("AFTER_GONGSIMDON"), EScenarioInteractionType::Custom, TEXT("AFTER_GONGSIMDON"))
		}),
		MakeStage(TEXT("ONGSEONG"), TEXT("옹성"), TEXT("NOKRO"), {
			MakeNarrationStep(TEXT("ONG_SITUATION"), TEXT("ONG_SITUATION"), TEXT("MAIN_NA_17"), TEXT("ONG_QUIZ")),
			MakeStep(TEXT("ONG_QUIZ"), EScenarioInteractionType::Quiz, TEXT("QUIZ_ONGSEONG"), TEXT("ONG_ANSWER")),
			MakeNarrationStep(TEXT("ONG_ANSWER"), TEXT("ONG_ANSWER"), TEXT("MAIN_NA_21"), TEXT("ONG_IMAGE_01")),
			MakeStep(TEXT("ONG_IMAGE_01"), EScenarioInteractionType::Custom, TEXT("ONG_IMAGE_01"), TEXT("ONG_IMAGE_02")),
			MakeNarrationStep(TEXT("ONG_IMAGE_02"), TEXT("ONG_IMAGE_02"), TEXT("MAIN_NA_23"), TEXT("ONG_TRAVEL_BRIEF")),
			MakeNarrationStep(TEXT("ONG_TRAVEL_BRIEF"), TEXT("ONG_TRAVEL_BRIEF"), TEXT("MAIN_NA_25"), TEXT("TRAVEL_ONGSEONG")),
			MakeStep(TEXT("TRAVEL_ONGSEONG"), EScenarioInteractionType::Custom, TEXT("Travel_Ongseong"), TEXT("AFTER_ONGSEONG")),
			MakeStep(TEXT("AFTER_ONGSEONG"), EScenarioInteractionType::Custom, TEXT("AFTER_ONGSEONG"))
		}),
		MakeStage(TEXT("NOKRO"), TEXT("녹로"), TEXT("GEOJUNGGI"), {
			MakeNarrationStep(TEXT("NOKRO_SITUATION"), TEXT("NOKRO_SITUATION"), TEXT("MAIN_NA_27"), TEXT("NOKRO_QUIZ")),
			MakeStep(TEXT("NOKRO_QUIZ"), EScenarioInteractionType::Quiz, TEXT("QUIZ_NOKRO"), TEXT("NOKRO_ANSWER")),
			MakeNarrationStep(TEXT("NOKRO_ANSWER"), TEXT("NOKRO_ANSWER"), TEXT("MAIN_NA_31"), TEXT("NOKRO_IMAGE_01")),
			MakeStep(TEXT("NOKRO_IMAGE_01"), EScenarioInteractionType::Custom, TEXT("NOKRO_IMAGE_01"), TEXT("NOKRO_IMAGE_02")),
			MakeNarrationStep(TEXT("NOKRO_IMAGE_02"), TEXT("NOKRO_IMAGE_02"), TEXT("MAIN_NA_33"), TEXT("NOKRO_TRAVEL_BRIEF")),
			MakeStep(TEXT("NOKRO_TRAVEL_BRIEF"), EScenarioInteractionType::Custom, TEXT("NOKRO_TRAVEL_BRIEF"), TEXT("TRAVEL_NOKRO")),
			MakeStep(TEXT("TRAVEL_NOKRO"), EScenarioInteractionType::Custom, TEXT("Travel_Nokro"), TEXT("AFTER_NOKRO")),
			MakeStep(TEXT("AFTER_NOKRO"), EScenarioInteractionType::Custom, TEXT("AFTER_NOKRO"))
		}),
		MakeStage(TEXT("GEOJUNGGI"), TEXT("거중기"), TEXT("SUMMARY"), {
			MakeStep(TEXT("GEO_SITUATION"), EScenarioInteractionType::Custom, TEXT("GEO_SITUATION"), TEXT("GEO_QUIZ")),
			MakeStep(TEXT("GEO_QUIZ"), EScenarioInteractionType::Quiz, TEXT("QUIZ_GEOJUNGGI"), TEXT("GEO_ANSWER")),
			MakeStep(TEXT("GEO_ANSWER"), EScenarioInteractionType::Custom, TEXT("GEO_ANSWER"), TEXT("GEO_IMAGE_01")),
			MakeStep(TEXT("GEO_IMAGE_01"), EScenarioInteractionType::Custom, TEXT("GEO_IMAGE_01"), TEXT("GEO_IMAGE_02")),
			MakeStep(TEXT("GEO_IMAGE_02"), EScenarioInteractionType::Custom, TEXT("GEO_IMAGE_02"), TEXT("PULLEY_COMPARE")),
			MakeStep(TEXT("PULLEY_COMPARE"), EScenarioInteractionType::Custom, TEXT("PULLEY_COMPARE"), TEXT("GEO_TRAVEL_BRIEF")),
			MakeStep(TEXT("GEO_TRAVEL_BRIEF"), EScenarioInteractionType::Custom, TEXT("GEO_TRAVEL_BRIEF"), TEXT("TRAVEL_GEOJUNGGI")),
			MakeStep(TEXT("TRAVEL_GEOJUNGGI"), EScenarioInteractionType::Custom, TEXT("Travel_Geojunggi"), TEXT("AFTER_GEOJUNGGI")),
			MakeStep(TEXT("AFTER_GEOJUNGGI"), EScenarioInteractionType::Custom, TEXT("AFTER_GEOJUNGGI"))
		}),
		MakeStage(TEXT("SUMMARY"), TEXT("전체 학습 정리"), TEXT(""), {
			MakeStep(TEXT("SUMMARY_IMAGE"), EScenarioInteractionType::Custom, TEXT("SUMMARY_IMAGE"), TEXT("SUMMARY_01")),
			MakeStep(TEXT("SUMMARY_01"), EScenarioInteractionType::Custom, TEXT("SUMMARY_01"))
		})
	};

	EducationContent = {
		MakeContent(TEXT("INTRO_01"), EMainEducationContentType::Instructor, TEXT("신임 지휘관 부임"), TEXT("신임 지휘관님, 수원화성에 오신 것을 환영합니다. 수원화성은 조선 정조 때 건설된 성곽입니다. 이제 주요 시설과 장치를 하나씩 살펴보겠습니다.")),
		MakeContent(TEXT("INTRO_OVERVIEW"), EMainEducationContentType::Image, TEXT("수원화성 전체 구조"), TEXT("수원화성 전체가 하나의 성곽 체계임을 확인합니다."), TEXT("수원화성 전체 성곽 체계"), {TEXT("성벽"), TEXT("성문"), TEXT("공심돈"), TEXT("주요 체험 위치")}),
		MakeContent(TEXT("DEFENSE_01"), EMainEducationContentType::Instructor, TEXT("성벽 방어와 화약무기"), TEXT("적이 성으로 접근한다면 병사들은 성벽과 방어시설로 진입을 막아야 했습니다. 조선 후기에는 조총과 화포 같은 화약무기도 활용되었습니다.")),
		MakeContent(TEXT("DEFENSE_IMAGE"), EMainEducationContentType::Image, TEXT("조선 후기 성벽 방어"), TEXT("이번 교육에서는 조선의 화약 기술을 체험하기 위해 신기전을 가상의 수원화성 방어 상황에 활용합니다."), TEXT("이번 체험에서 활용할 조선의 화약 무기"), {TEXT("조총"), TEXT("화포"), TEXT("신기전")}),
		MakeContent(TEXT("SINGIJEON_BRIEF"), EMainEducationContentType::Instructor, TEXT("신기전 가상 방어 체험"), TEXT("가상의 적군이 수원화성으로 접근하는 상황에서 성벽을 방어해 보십시오.")),
		MakeContent(TEXT("AFTER_SINGIJEON"), EMainEducationContentType::Instructor, TEXT("신기전 체험 완료"), TEXT("화약무기의 활용을 확인했습니다. 이제 수원화성의 주요 시설을 살펴보겠습니다.")),

		MakeContent(TEXT("GONG_SITUATION"), EMainEducationContentType::Instructor, TEXT("공심돈 상황"), TEXT("성을 지키기 위해서는 먼저 적이 어디에서 접근하는지 확인해야 합니다. 높은 위치에서 주변을 살피고 내부에서 몸을 보호하며 공격할 수도 있는 시설의 이름은 무엇일까요?")),
		MakeQuiz(TEXT("QUIZ_GONGSIMDON"), TEXT("성 안에서 밖의 적을 살피고 공격할 수 있도록 만든 속이 빈 방어시설은 무엇일까요?"), TEXT("ㄱ ㅅ ㄷ"), TEXT("공심돈")),
		MakeContent(TEXT("GONG_ANSWER"), EMainEducationContentType::Instructor, TEXT("정답: 공심돈"), TEXT("공심돈은 내부가 비어 있는 높은 방어시설로, 병사들이 안에서 성 밖을 관찰하고 적을 공격할 수 있도록 만들어졌습니다.")),
		MakeContent(TEXT("GONG_IMAGE_01"), EMainEducationContentType::Image, TEXT("공심돈의 실제 모습"), TEXT("높은 위치 덕분에 주변을 넓게 살피고 시설 안에서 몸을 보호할 수 있었습니다."), TEXT("관찰 + 방어"), {TEXT("높은 관찰 위치"), TEXT("내부 공간"), TEXT("밖을 살피는 부분"), TEXT("공격할 수 있는 구멍")}),
		MakeContent(TEXT("GONG_IMAGE_02"), EMainEducationContentType::Image, TEXT("공심돈 단면도"), TEXT("병사 위치와 내부 공간, 성벽과의 관계를 단면으로 확인합니다."), TEXT("관찰 + 방어")),
		MakeContent(TEXT("GONG_TRAVEL_BRIEF"), EMainEducationContentType::Instructor, TEXT("공심돈 체험"), TEXT("직접 공심돈 안으로 들어가 주변을 살펴보겠습니다.")),
		MakeContent(TEXT("AFTER_GONGSIMDON"), EMainEducationContentType::Instructor, TEXT("공심돈 체험 완료"), TEXT("공심돈의 관찰과 방어 역할을 확인했습니다.")),

		MakeContent(TEXT("ONG_SITUATION"), EMainEducationContentType::Instructor, TEXT("옹성 상황"), TEXT("성문은 중요한 통로이지만 적이 집중 공격할 수 있는 곳입니다. 성문을 한 번 더 보호하기 위한 시설은 무엇일까요?")),
		MakeQuiz(TEXT("QUIZ_ONGSEONG"), TEXT("성문 바깥을 둘러싸도록 만든 방어시설은 무엇일까요?"), TEXT("ㅇ ㅅ"), TEXT("옹성")),
		MakeContent(TEXT("ONG_ANSWER"), EMainEducationContentType::Instructor, TEXT("정답: 옹성"), TEXT("옹성은 성문 바깥을 다시 성벽으로 둘러싸 적이 성문까지 곧바로 접근하기 어렵게 만든 방어시설입니다.")),
		MakeContent(TEXT("ONG_IMAGE_01"), EMainEducationContentType::Image, TEXT("옹성 평면 구조"), TEXT("적의 접근 방향과 옹성, 성문의 배치를 위에서 확인합니다."), TEXT("성문을 한 번 더 보호")),
		MakeContent(TEXT("ONG_IMAGE_02"), EMainEducationContentType::Comparison, TEXT("옹성 유무 비교"), TEXT("옹성이 있으면 적은 제한된 공간을 통과해야 하므로 병사들이 더 효과적으로 대응할 수 있었습니다."), TEXT("적 → 옹성 → 성문")),
		MakeContent(TEXT("ONG_TRAVEL_BRIEF"), EMainEducationContentType::Instructor, TEXT("옹성 체험"), TEXT("적이 성문으로 공격해 올 때 옹성이 어떤 역할을 하는지 직접 확인해 보겠습니다.")),
		MakeContent(TEXT("AFTER_ONGSEONG"), EMainEducationContentType::Instructor, TEXT("옹성 체험 완료"), TEXT("옹성이 성문 접근을 지연시키는 방식을 확인했습니다.")),

		MakeContent(TEXT("NOKRO_SITUATION"), EMainEducationContentType::Instructor, TEXT("녹로 상황"), TEXT("무거운 무기와 물자를 높은 곳까지 직접 들어 올리기는 어렵습니다. 이때 도르래를 이용한 장치를 활용할 수 있었습니다.")),
		MakeQuiz(TEXT("QUIZ_NOKRO"), TEXT("도르래의 원리를 이용해 무거운 물건을 위아래로 움직이는 장치는 무엇일까요?"), TEXT("ㄴ ㄹ"), TEXT("녹로")),
		MakeContent(TEXT("NOKRO_ANSWER"), EMainEducationContentType::Instructor, TEXT("정답: 녹로"), TEXT("녹로는 도르래와 줄을 이용해 무거운 물체를 들어 올리거나 이동하는 데 활용한 장치입니다.")),
		MakeContent(TEXT("NOKRO_IMAGE_01"), EMainEducationContentType::Image, TEXT("녹로 작동 구조"), TEXT("도르래, 줄, 이동하는 물체 사이의 연결을 확인합니다."), TEXT("도르래로 힘의 방향 전환"), {TEXT("도르래"), TEXT("줄"), TEXT("이동하는 물체")}),
		MakeContent(TEXT("NOKRO_IMAGE_02"), EMainEducationContentType::Comparison, TEXT("직접 들기와 녹로 비교"), TEXT("도르래는 힘의 방향을 바꾸거나 힘을 여러 줄에 나누어 무거운 물체를 효율적으로 움직이게 합니다."), TEXT("힘을 효율적으로 전달")),
		MakeContent(TEXT("NOKRO_TRAVEL_BRIEF"), EMainEducationContentType::Instructor, TEXT("녹로 체험"), TEXT("직접 녹로를 움직여 도르래가 어떤 도움을 주는지 확인해 보겠습니다.")),
		MakeContent(TEXT("AFTER_NOKRO"), EMainEducationContentType::Instructor, TEXT("녹로 체험 완료"), TEXT("녹로를 이용한 물자 이동 원리를 확인했습니다.")),

		MakeContent(TEXT("GEO_SITUATION"), EMainEducationContentType::Instructor, TEXT("거중기 상황"), TEXT("수원화성을 건설하려면 많은 돌과 무거운 건축 자재를 높은 곳으로 옮겨야 했습니다.")),
		MakeQuiz(TEXT("QUIZ_GEOJUNGGI"), TEXT("수원화성을 건설할 때 무거운 돌을 들어 올리는 데 활용한 장치는 무엇일까요?"), TEXT("ㄱ ㅈ ㄱ"), TEXT("거중기")),
		MakeContent(TEXT("GEO_ANSWER"), EMainEducationContentType::Instructor, TEXT("정답: 거중기"), TEXT("거중기는 여러 도르래의 원리를 활용해 무거운 물체를 적은 힘으로 들어 올리도록 만든 장치입니다.")),
		MakeContent(TEXT("GEO_IMAGE_01"), EMainEducationContentType::Image, TEXT("거중기 복원 모습"), TEXT("위쪽 도르래와 줄, 들어 올릴 돌, 사람이 힘을 가하는 부분을 확인합니다."), TEXT("수원화성 축성 기술"), {TEXT("위쪽 도르래"), TEXT("연결된 줄"), TEXT("들어 올릴 돌"), TEXT("힘을 가하는 부분")}),
		MakeContent(TEXT("GEO_IMAGE_02"), EMainEducationContentType::Image, TEXT("거중기 작동 원리"), TEXT("사람이 줄을 당기면 여러 도르래를 통해 힘이 전달되어 무거운 돌이 올라갑니다."), TEXT("여러 도르래로 힘 전달")),
		MakeContent(TEXT("PULLEY_COMPARE"), EMainEducationContentType::Comparison, TEXT("녹로와 거중기 비교"), TEXT("둘 다 도르래를 활용하지만 녹로는 운반과 이동, 거중기는 무거운 건축자재를 다루는 축성 작업에 활용되었습니다."), TEXT("녹로: 물자 이동 / 거중기: 축성 작업")),
		MakeContent(TEXT("GEO_TRAVEL_BRIEF"), EMainEducationContentType::Instructor, TEXT("거중기 체험"), TEXT("직접 거중기를 움직여 그 원리를 확인해 보겠습니다.")),
		MakeContent(TEXT("AFTER_GEOJUNGGI"), EMainEducationContentType::Instructor, TEXT("거중기 체험 완료"), TEXT("여러 도르래로 큰 힘을 얻는 축성 기술을 확인했습니다.")),

		MakeContent(TEXT("SUMMARY_IMAGE"), EMainEducationContentType::Summary, TEXT("수원화성 전체 학습 정리"), TEXT("처음의 조감도에 학습한 시설과 역할을 함께 표시합니다."), TEXT("방어시설 + 과학기술"), {TEXT("공심돈: 관찰·방어"), TEXT("옹성: 성문 보호"), TEXT("녹로: 물자 이동"), TEXT("거중기: 건축 자재 이동")}),
		MakeContent(TEXT("SUMMARY_01"), EMainEducationContentType::Instructor, TEXT("교육 완료"), TEXT("수원화성은 단순히 높은 성벽을 세운 것이 아니라 다양한 방어시설과 당시의 과학기술을 함께 활용해 만든 성곽입니다. 신임 지휘관님, 수원화성에 대한 교육을 모두 완료했습니다."))
	};

	ExperienceRoutes.Reset();
	auto AddRoute = [this](const TCHAR* RouteID, const TCHAR* AssetPath)
	{
		FMainEducationExperienceRoute Route;
		Route.RouteID = FName(RouteID);
		if (FCString::Strlen(AssetPath) > 0)
		{
			Route.Experience = TSoftObjectPtr<UExperienceDefinition>(FSoftObjectPath(AssetPath));
		}
		ExperienceRoutes.Add(Route);
	};
	AddRoute(TEXT("Travel_Singijeon"), TEXT("/Game/Core/Experience/Definitions/DA_Experience_Singijeon.DA_Experience_Singijeon"));
	AddRoute(TEXT("Travel_Gongsimdon"), TEXT("/Game/Core/Experience/Definitions/DA_Experience_Gongsimdon.DA_Experience_Gongsimdon"));
	AddRoute(TEXT("Travel_Ongseong"), TEXT("/Game/Core/Experience/Definitions/DA_Experience_Ongseong.DA_Experience_Ongseong"));
	AddRoute(TEXT("Travel_Nokro"), TEXT(""));
	AddRoute(TEXT("Travel_Geojunggi"), TEXT("/Game/Core/Experience/Definitions/DA_Experience_Geojunggi.DA_Experience_Geojunggi"));
}
#endif

FString UMainEducationScenarioDefinition::NormalizeAnswer(const FString& Answer)
{
	FString Result = Answer;
	Result.TrimStartAndEndInline();
	Result.ReplaceInline(TEXT(" "), TEXT(""));
	Result.ReplaceInline(TEXT("\t"), TEXT(""));
	Result.ReplaceInline(TEXT("\r"), TEXT(""));
	Result.ReplaceInline(TEXT("\n"), TEXT(""));
	return Result.ToLower();
}
