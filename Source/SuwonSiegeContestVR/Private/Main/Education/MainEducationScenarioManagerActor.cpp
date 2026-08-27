#include "Main/Education/MainEducationScenarioManagerActor.h"

#include "Core/Experience/ExperienceDefinition.h"
#include "Core/Experience/ExperienceSubsystem.h"
#include "Core/Quiz/InitialConsonantQuizComponent.h"
#include "Core/Scenario/ScenarioExperienceBridgeComponent.h"
#include "Core/Scenario/ScenarioManagerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "Gameplay/UI/VRHUDComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Main/Education/MainEducationScenarioDefinition.h"
#include "Main/Education/MainEducationPresentationWidget.h"
#include "Main/Education/MainEducationWidgetComponent.h"

AMainEducationScenarioManagerActor::AMainEducationScenarioManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PresentationWidgetComponent = CreateDefaultSubobject<UMainEducationWidgetComponent>(TEXT("EducationPresentationPanel"));
	PresentationWidgetComponent->SetupAttachment(GetRootComponent());
	PresentationWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	PresentationWidgetComponent->SetDrawSize(PresentationPanelDrawSize);
	PresentationWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	PresentationWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
	PresentationWidgetComponent->SetTwoSided(true);
	PresentationWidgetComponent->SetTranslucentSortPriority(9999);
	PresentationWidgetComponent->SetWidgetClass(UMainEducationPresentationWidget::StaticClass());
	PresentationWidgetComponent->SetVisibility(false);
	PresentationWidgetComponent->SetHiddenInGame(true);

	// The quiz runtime is Core. Main only supplies the question, taken from the current step's content.
	EducationQuiz = CreateDefaultSubobject<UInitialConsonantQuizComponent>(TEXT("EducationQuiz"));
}

void AMainEducationScenarioManagerActor::BeginPlay()
{
	// Bind before the base actor auto-starts the configured Scenario.
	if (UScenarioManagerComponent* Manager = GetScenarioManager())
	{
		Manager->OnInteractionRequested.AddDynamic(this, &ThisClass::HandleInteractionRequested);
	}
	Super::BeginPlay();
	HidePresentationPanel();
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>())
		{
			PresentationWidgetComponent->AttachToComponent(Camera, FAttachmentTransformRules::KeepRelativeTransform);
			PresentationWidgetComponent->SetRelativeLocation(PresentationPanelOffset);
			PresentationWidgetComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
			PresentationWidgetComponent->SetRelativeScale3D(FVector(PresentationPanelWorldScale));
			PresentationWidgetComponent->SetDrawSize(PresentationPanelDrawSize);
			PresentationWidgetComponent->InitWidget();
		}
	}

	if (UMainEducationScenarioDefinition* Definition = GetEducationDefinition())
	{
		FString Error;
		if (!Definition->ValidateEducationScenario(Error))
		{
			UE_LOG(LogTemp, Error, TEXT("Main education definition is invalid: %s"), *Error);
		}
	}
}

void AMainEducationScenarioManagerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(EducationQuiz))
	{
		EducationQuiz->OnQuizFinished.RemoveDynamic(this, &ThisClass::HandleEducationQuizFinished);
		// A quiz never outlives the level that asked it, and neither does the microphone.
		EducationQuiz->CancelQuiz();
	}
	Super::EndPlay(EndPlayReason);
}

bool AMainEducationScenarioManagerActor::ContinuePresentation()
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || CurrentContent.ContentID.IsNone())
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.InteractionType == EScenarioInteractionType::Quiz ||
		Interaction.InteractionType == EScenarioInteractionType::Narration ||
		Interaction.TargetID != CurrentContent.ContentID)
	{
		return false;
	}

	CurrentContent = FMainEducationContent();
	HidePresentationPanel();
	if (UVRHUDComponent* HUD = ResolveVRHUD())
	{
		HUD->ClearPrompt();
	}
	return Manager->CompleteInteraction(Interaction.InteractionID);
}

bool AMainEducationScenarioManagerActor::SubmitQuizAnswer(const FString& Answer)
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || CurrentContent.ContentType != EMainEducationContentType::Quiz)
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.InteractionType != EScenarioInteractionType::Quiz ||
		Interaction.TargetID != CurrentContent.ContentID)
	{
		return false;
	}

	++QuizAttemptCount;
	UMainEducationScenarioDefinition* Definition = GetEducationDefinition();
	const bool bCorrect = Definition && Definition->IsAcceptedQuizAnswer(CurrentContent.ContentID, Answer);

	if (!bCorrect)
	{
		const FText Feedback = FText::FromString(TEXT("다시 생각해 보십시오."));
		OnQuizFeedback.Broadcast(false, Feedback);
		if (UVRHUDComponent* HUD = ResolveVRHUD())
		{
			HUD->ShowNotification(Feedback, EVRHUDNotificationType::Warning);
		}
		return false;
	}

	const FText CanonicalAnswer = CurrentContent.AcceptedAnswers.IsEmpty()
		? FText::GetEmpty() : CurrentContent.AcceptedAnswers[0];
	const FText Feedback = FText::Format(
		FText::FromString(TEXT("정답입니다: {0}")), CanonicalAnswer);
	OnQuizFeedback.Broadcast(true, Feedback);
	if (UVRHUDComponent* HUD = ResolveVRHUD())
	{
		HUD->ShowNotification(Feedback, EVRHUDNotificationType::Success);
	}
	CancelVoiceRecognition();
	return CompleteQuizInteraction();
}

bool AMainEducationScenarioManagerActor::CompleteQuizInteraction()
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || CurrentContent.ContentID.IsNone())
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.InteractionType != EScenarioInteractionType::Quiz ||
		Interaction.TargetID != CurrentContent.ContentID)
	{
		return false;
	}

	CurrentContent = FMainEducationContent();
	if (UVRHUDComponent* HUD = ResolveVRHUD())
	{
		HUD->ClearPrompt();
	}
	return Manager->CompleteInteraction(Interaction.InteractionID);
}

bool AMainEducationScenarioManagerActor::SkipUnavailableExperience()
{
	UScenarioManagerComponent* Manager = GetScenarioManager();
	if (!Manager || UnavailableRouteID.IsNone())
	{
		return false;
	}

	const FScenarioInteraction Interaction = Manager->GetCurrentInteraction();
	if (Interaction.TargetID != UnavailableRouteID)
	{
		return false;
	}

	UnavailableRouteID = NAME_None;
	return Manager->CompleteInteraction(Interaction.InteractionID);
}

bool AMainEducationScenarioManagerActor::StartEducationAfterIntro()
{
	if (bEducationStartedByIntro)
	{
		return false;
	}

	const bool bStarted = StartConfiguredScenario();
	if (bStarted)
	{
		bEducationStartedByIntro = true;
		if (UScenarioExperienceBridgeComponent* Bridge = GetExperienceBridge())
		{
			Bridge->RestoreScenarioCheckpoint();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Main education failed to start after intro on %s."), *GetName());
	}
	return bStarted;
}

bool AMainEducationScenarioManagerActor::ShouldAutoStartScenario() const
{
	return !bWaitForIntroSequence;
}

void AMainEducationScenarioManagerActor::RequestVoiceRecognition_Implementation(const FName QuizID)
{
	if (!bUseVoiceQuiz || !IsValid(EducationQuiz))
	{
		return;
	}
	if (CurrentContent.ContentID != QuizID || CurrentContent.ContentType != EMainEducationContentType::Quiz)
	{
		return;
	}

	const FInitialConsonantQuizDefinition Quiz = BuildQuizFromContent(CurrentContent);
	EducationQuiz->OnQuizFinished.AddUniqueDynamic(this, &ThisClass::HandleEducationQuizFinished);
	if (EducationQuiz->StartQuizDefinition(Quiz))
	{
		return;
	}

	// Malformed quiz data must not strand the course in front of a panel nobody can dismiss.
	EducationQuiz->OnQuizFinished.RemoveDynamic(this, &ThisClass::HandleEducationQuizFinished);
	UE_LOG(LogTemp, Warning, TEXT("Main education quiz %s could not start; continuing the flow."),
		*QuizID.ToString());
	CompleteQuizInteraction();
}

void AMainEducationScenarioManagerActor::CancelVoiceRecognition_Implementation()
{
	if (IsValid(EducationQuiz))
	{
		// Ends the quiz on any path, which is also what releases the microphone.
		EducationQuiz->CancelQuiz();
	}
}

FInitialConsonantQuizDefinition AMainEducationScenarioManagerActor::BuildQuizFromContent(
	const FMainEducationContent& Content) const
{
	FInitialConsonantQuizDefinition Quiz;
	Quiz.QuizID = Content.ContentID;
	if (!Content.Title.IsEmpty())
	{
		Quiz.PromptTitle = Content.Title;
	}
	Quiz.QuestionText = Content.Body;
	// Left empty, the Core library derives the consonants from the answer.
	Quiz.InitialConsonants = Content.InitialConsonants;
	if (!Content.AcceptedAnswers.IsEmpty())
	{
		Quiz.Answer = Content.AcceptedAnswers[0];
		for (int32 Index = 1; Index < Content.AcceptedAnswers.Num(); ++Index)
		{
			Quiz.AcceptedAnswers.Add(Content.AcceptedAnswers[Index]);
		}
	}
	Quiz.HintText = Content.HighlightText;
	Quiz.ListenDuration = QuizListenDuration;
	Quiz.MaxAttempts = QuizMaxAttempts;
	// Education content: a wrong answer reveals the answer instead of blocking the course.
	Quiz.bRevealAnswerOnFail = true;
	return Quiz;
}

void AMainEducationScenarioManagerActor::HandleEducationQuizFinished(
	const FName QuizID, const bool bCorrect, const EInitialConsonantQuizOutcome Outcome)
{
	if (IsValid(EducationQuiz))
	{
		EducationQuiz->OnQuizFinished.RemoveDynamic(this, &ThisClass::HandleEducationQuizFinished);
	}
	// A cancel comes from a teardown, a restart, or an answer submitted elsewhere. Neither of those
	// should advance the Scenario from here.
	if (Outcome == EInitialConsonantQuizOutcome::Canceled)
	{
		return;
	}
	if (CurrentContent.ContentID != QuizID)
	{
		return;
	}

	if (bCorrect)
	{
		// Routed through the shared ingress so speech, a button and the console all report the same way.
		const FText CanonicalAnswer = CurrentContent.AcceptedAnswers.IsEmpty()
			? FText::GetEmpty() : CurrentContent.AcceptedAnswers[0];
		SubmitQuizAnswer(CanonicalAnswer.ToString());
		return;
	}

	++QuizAttemptCount;
	const FText Feedback = FText::Format(
		FText::FromString(TEXT("정답은 {0} 입니다.")),
		CurrentContent.AcceptedAnswers.IsEmpty() ? FText::GetEmpty() : CurrentContent.AcceptedAnswers[0]);
	OnQuizFeedback.Broadcast(false, Feedback);
	if (UVRHUDComponent* HUD = ResolveVRHUD())
	{
		HUD->ShowNotification(Feedback, EVRHUDNotificationType::Warning);
	}
	CompleteQuizInteraction();
}

void AMainEducationScenarioManagerActor::HandleInteractionRequested(FScenarioInteraction Interaction)
{
	UnavailableRouteID = NAME_None;
	QuizAttemptCount = 0;

	if (Interaction.TargetID.ToString().StartsWith(TEXT("Travel_")))
	{
		BeginExperienceTravel(Interaction);
		return;
	}

	UMainEducationScenarioDefinition* Definition = GetEducationDefinition();
	if (!Definition || !Definition->FindEducationContent(Interaction.TargetID, CurrentContent))
	{
		UE_LOG(LogTemp, Warning, TEXT("No Main education content for interaction %s (%s)."),
			*Interaction.InteractionID.ToString(), *Interaction.TargetID.ToString());
		return;
	}

	OnEducationContentRequested.Broadcast(CurrentContent);
	// Narration plays without a panel, and a quiz gets the Core quiz panel instead of this one.
	if (Interaction.InteractionType == EScenarioInteractionType::Narration ||
		Interaction.InteractionType == EScenarioInteractionType::Quiz)
	{
		HidePresentationPanel();
	}
	else
	{
		ShowPresentationPanel(CurrentContent);
	}
	if (bMirrorTextToVRHUD)
	{
		if (UVRHUDComponent* HUD = ResolveVRHUD())
		{
			HUD->SetObjective(CurrentContent.Title, CurrentContent.Body);
			FText Prompt = CurrentContent.InteractionGuideText;
			if (Prompt.IsEmpty() && Interaction.InteractionType == EScenarioInteractionType::Narration)
			{
				Prompt = FText::FromString(TEXT("나레이션 재생 중입니다."));
			}
			else if (Prompt.IsEmpty() && CurrentContent.ContentType == EMainEducationContentType::Quiz)
			{
				Prompt = FText::Format(
					FText::FromString(TEXT("정답을 말하거나 입력하세요: {0}")),
					CurrentContent.InitialConsonants);
			}
			else if (Prompt.IsEmpty())
			{
				Prompt = FText::FromString(TEXT("계속하려면 확인을 누르세요."));
			}
			HUD->ShowPrompt(Prompt);
		}
	}

	if (Interaction.InteractionType == EScenarioInteractionType::Quiz)
	{
		RequestVoiceRecognition(CurrentContent.ContentID);
	}
}

bool AMainEducationScenarioManagerActor::BeginExperienceTravel(const FScenarioInteraction& Interaction)
{
	UMainEducationScenarioDefinition* Definition = GetEducationDefinition();
	FMainEducationExperienceRoute Route;
	UExperienceDefinition* Experience = nullptr;
	if (Definition && Definition->FindExperienceRoute(Interaction.TargetID, Route) && !Route.Experience.IsNull())
	{
		Experience = Route.Experience.LoadSynchronous();
	}

	if (!Experience)
	{
		UnavailableRouteID = Interaction.TargetID;
		const FText Reason = FText::FromString(TEXT("연결된 체험 Level/Experience가 아직 없습니다."));
		OnExperienceUnavailable.Broadcast(Interaction.TargetID, Reason);
		if (UVRHUDComponent* HUD = ResolveVRHUD())
		{
			HUD->SetObjective(FText::FromName(Interaction.TargetID), Reason);
			HUD->ShowPrompt(FText::FromString(TEXT("개발 중에는 Skip Unavailable Experience를 사용하세요.")));
		}
		return false;
	}

	UScenarioManagerComponent* Manager = GetScenarioManager();
	UExperienceSubsystem* ExperienceSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UExperienceSubsystem>() : nullptr;
	const FScenarioDebugSnapshot Snapshot = Manager ? Manager->GetDebugSnapshot() : FScenarioDebugSnapshot();
	if (!Manager || !ExperienceSubsystem || Interaction.NextInteractionID.IsNone() ||
		!ExperienceSubsystem->SetScenarioResumeCheckpoint(
			Snapshot.ScenarioID, Snapshot.SceneID, Interaction.NextInteractionID))
	{
		UnavailableRouteID = Interaction.TargetID;
		const FText Reason = FText::FromString(TEXT("체험 복귀 체크포인트를 만들 수 없습니다."));
		OnExperienceUnavailable.Broadcast(Interaction.TargetID, Reason);
		return false;
	}

	if (!ExperienceSubsystem->StartExperience(Experience, false))
	{
		ExperienceSubsystem->ClearScenarioResumeCheckpoint(Snapshot.ScenarioID);
		UnavailableRouteID = Interaction.TargetID;
		const FText Reason = FText::FromString(TEXT("체험 이동 요청이 거부되었습니다."));
		OnExperienceUnavailable.Broadcast(Interaction.TargetID, Reason);
		return false;
	}
	return true;
}

UMainEducationScenarioDefinition* AMainEducationScenarioManagerActor::GetEducationDefinition() const
{
	return Cast<UMainEducationScenarioDefinition>(ScenarioDefinition);
}

UVRHUDComponent* AMainEducationScenarioManagerActor::ResolveVRHUD() const
{
	const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	return Pawn ? Pawn->FindComponentByClass<UVRHUDComponent>() : nullptr;
}

void AMainEducationScenarioManagerActor::ShowPresentationPanel(const FMainEducationContent& Content)
{
	if (!PresentationWidgetComponent)
	{
		return;
	}
	PresentationWidgetComponent->InitWidget();
	if (UMainEducationPresentationWidget* Widget =
		Cast<UMainEducationPresentationWidget>(PresentationWidgetComponent->GetUserWidgetObject()))
	{
		Widget->Configure(this, Content);
	}
	PresentationWidgetComponent->SetHiddenInGame(false, true);
	PresentationWidgetComponent->SetVisibility(true, true);
}

void AMainEducationScenarioManagerActor::HidePresentationPanel()
{
	if (PresentationWidgetComponent)
	{
		PresentationWidgetComponent->SetHiddenInGame(true, true);
		PresentationWidgetComponent->SetVisibility(false, true);
	}
}
