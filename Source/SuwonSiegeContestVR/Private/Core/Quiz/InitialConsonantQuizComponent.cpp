#include "Core/Quiz/InitialConsonantQuizComponent.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/Quiz/InitialConsonantQuizSet.h"
#include "Core/Quiz/InitialConsonantQuizWidget.h"
#include "Core/Scenario/ScenarioInteractionGuideComponent.h"
#include "Core/Text/HangulTextLibrary.h"
#include "Core/Voice/MockVoiceRecognitionComponent.h"
#include "Core/Voice/VoiceRecognitionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "InitialConsonantQuiz"

namespace
{
	const FLinearColor ListeningColor(0.95f, 0.85f, 0.35f, 1.0f);
	const FLinearColor CorrectColor(0.35f, 0.95f, 0.55f, 1.0f);
	const FLinearColor RetryColor(1.0f, 0.55f, 0.35f, 1.0f);
}

UInitialConsonantQuizComponent::UInitialConsonantQuizComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// The panel only re-aims at the player; 30Hz is smooth in VR without a per-frame update.
	PrimaryComponentTick.TickInterval = 1.0f / 30.0f;
}

void UInitialConsonantQuizComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UInitialConsonantQuizComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResultHoldHandle);
	}

	HidePanel();
	ReleaseVoiceRecognition();
	QuizState = EInitialConsonantQuizState::Idle;

	Super::EndPlay(EndPlayReason);
}

void UInitialConsonantQuizComponent::TickComponent(
	const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdatePanelFacing();
}

bool UInitialConsonantQuizComponent::FindQuizDefinition(
	const FName QuizID, FInitialConsonantQuizDefinition& OutQuiz) const
{
	if (QuizID.IsNone())
	{
		return false;
	}

	if (const FInitialConsonantQuizDefinition* Local = Quizzes.FindByPredicate(
		[QuizID](const FInitialConsonantQuizDefinition& Quiz) { return Quiz.QuizID == QuizID; }))
	{
		OutQuiz = *Local;
		return true;
	}

	return QuizSet && QuizSet->FindQuiz(QuizID, OutQuiz);
}

bool UInitialConsonantQuizComponent::StartQuiz(const FName QuizID)
{
	FInitialConsonantQuizDefinition Quiz;
	if (!FindQuizDefinition(QuizID, Quiz))
	{
		UE_LOG(LogTemp, Warning, TEXT("InitialConsonantQuiz: no quiz named %s."), *QuizID.ToString());
		return false;
	}

	return StartQuizDefinition(Quiz);
}

bool UInitialConsonantQuizComponent::StartQuizDefinition(const FInitialConsonantQuizDefinition& Quiz)
{
	if (IsQuizActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("InitialConsonantQuiz: %s is already running."),
			*ActiveQuiz.QuizID.ToString());
		return false;
	}

	if (!Quiz.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("InitialConsonantQuiz: quiz %s needs an ID and an answer."),
			*Quiz.QuizID.ToString());
		return false;
	}

	ActiveQuiz = Quiz;
	AttemptCount = 0;
	QuizState = EInitialConsonantQuizState::Listening;

	ShowPanel();
	OnQuizStarted.Broadcast(ActiveQuiz);
	BeginListeningAttempt();
	return true;
}

void UInitialConsonantQuizComponent::BeginListeningAttempt()
{
	ApplyFooterToWidget(BuildAttemptFooter());

	if (!bUseVoiceRecognition || !ResolveVoiceRecognition())
	{
		// Without a recognizer the quiz still runs; SubmitAnswer stays open for a button or console.
		ApplyStatusToWidget(VoiceUnavailableStatusText, RetryColor);
		return;
	}

	FVoiceRecognitionRequest Request;
	Request.RequestID = ActiveQuiz.QuizID;
	Request.Keywords = ActiveQuiz.GetAcceptedAnswerStrings();
	Request.ListenDuration = ActiveQuiz.ListenDuration;

	if (!VoiceRecognition->StartListening(Request))
	{
		ApplyStatusToWidget(VoiceUnavailableStatusText, RetryColor);
		return;
	}

	ApplyStatusToWidget(ListeningStatusText, ListeningColor);
}

void UInitialConsonantQuizComponent::HandleVoiceResult(FVoiceRecognitionResult Result)
{
	// Another system may share the recognizer, so only this quiz's request is consumed here.
	if (QuizState != EInitialConsonantQuizState::Listening || Result.RequestID != ActiveQuiz.QuizID)
	{
		return;
	}

	const FString SpokenText = Result.IsMatch() ? Result.MatchedKeyword : Result.RecognizedText;
	SubmitAnswer(SpokenText);
}

bool UInitialConsonantQuizComponent::SubmitAnswer(const FString& Answer)
{
	if (QuizState != EInitialConsonantQuizState::Listening)
	{
		return false;
	}

	++AttemptCount;

	if (VoiceRecognition && VoiceRecognition->IsListening())
	{
		VoiceRecognition->StopListening();
	}

	const bool bCorrect = UHangulTextLibrary::DoesAnswerMatch(Answer, ActiveQuiz.GetAcceptedAnswerStrings());
	OnQuizAttempt.Broadcast(Answer, bCorrect, AttemptCount);

	if (bCorrect)
	{
		QuizState = EInitialConsonantQuizState::Feedback;
		ApplyStatusToWidget(CorrectStatusText, CorrectColor);
		ApplyFooterToWidget(FText::Format(LOCTEXT("AnswerIs", "정답: {0}"), ActiveQuiz.Answer));
		bPendingSuccess = true;

		if (ActiveQuiz.ResultDisplayDuration > 0.0f && GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(ResultHoldHandle, this,
				&UInitialConsonantQuizComponent::HandleResultHoldElapsed, ActiveQuiz.ResultDisplayDuration, false);
		}
		else
		{
			HandleResultHoldElapsed();
		}
		return true;
	}

	const bool bAttemptsExhausted = ActiveQuiz.MaxAttempts > 0 && AttemptCount >= ActiveQuiz.MaxAttempts;
	if (!bAttemptsExhausted)
	{
		ApplyStatusToWidget(RetryStatusText, RetryColor);
		BeginListeningAttempt();
		return false;
	}

	QuizState = EInitialConsonantQuizState::Feedback;
	ApplyStatusToWidget(RetryStatusText, RetryColor);
	ApplyFooterToWidget(ActiveQuiz.bRevealAnswerOnFail
		? FText::Format(LOCTEXT("AnswerWas", "정답은 {0} 입니다"), ActiveQuiz.Answer)
		: FText::GetEmpty());
	bPendingSuccess = false;

	if (ActiveQuiz.ResultDisplayDuration > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(ResultHoldHandle, this,
			&UInitialConsonantQuizComponent::HandleResultHoldElapsed, ActiveQuiz.ResultDisplayDuration, false);
	}
	else
	{
		HandleResultHoldElapsed();
	}
	return false;
}

void UInitialConsonantQuizComponent::HandleResultHoldElapsed()
{
	FinishQuiz(bPendingSuccess, bPendingSuccess
		? EInitialConsonantQuizOutcome::Correct
		: EInitialConsonantQuizOutcome::Exhausted);
}

void UInitialConsonantQuizComponent::GiveUp()
{
	if (!IsQuizActive())
	{
		return;
	}

	if (ActiveQuiz.bRevealAnswerOnFail)
	{
		ApplyFooterToWidget(FText::Format(LOCTEXT("AnswerWas", "정답은 {0} 입니다"), ActiveQuiz.Answer));
	}
	FinishQuiz(false, EInitialConsonantQuizOutcome::Exhausted);
}

void UInitialConsonantQuizComponent::CancelQuiz()
{
	if (!IsQuizActive())
	{
		return;
	}

	FinishQuiz(false, EInitialConsonantQuizOutcome::Canceled);
}

void UInitialConsonantQuizComponent::FinishQuiz(const bool bCorrect, const EInitialConsonantQuizOutcome Outcome)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ResultHoldHandle);
	}

	if (VoiceRecognition && VoiceRecognition->IsListening())
	{
		VoiceRecognition->StopListening();
	}

	HidePanel();

	const FName FinishedQuizID = ActiveQuiz.QuizID;
	ActiveQuiz = FInitialConsonantQuizDefinition();
	AttemptCount = 0;
	bPendingSuccess = false;
	// Idle before broadcasting so a listener can start the next quiz from inside the callback.
	QuizState = EInitialConsonantQuizState::Idle;

	OnQuizFinished.Broadcast(FinishedQuizID, bCorrect, Outcome);
}

bool UInitialConsonantQuizComponent::ResolveVoiceRecognition()
{
	if (IsValid(VoiceRecognition))
	{
		return true;
	}

	VoiceRecognition = IsValid(VoiceRecognitionOverride)
		? VoiceRecognitionOverride.Get()
		: UVoiceRecognitionComponent::FindVoiceRecognition(this);

	if (!VoiceRecognition && bSpawnMockVoiceRecognitionIfMissing)
	{
		if (AActor* Owner = GetOwner())
		{
			UMockVoiceRecognitionComponent* Mock = NewObject<UMockVoiceRecognitionComponent>(
				Owner, TEXT("RuntimeMockVoiceRecognition"));
			Mock->RegisterComponent();
			Owner->AddInstanceComponent(Mock);
			VoiceRecognition = Mock;
			bOwnsVoiceRecognition = true;
			UE_LOG(LogTemp, Log,
				TEXT("InitialConsonantQuiz: no voice recognizer in the level, using the mock backend."));
		}
	}

	if (!VoiceRecognition)
	{
		return false;
	}

	VoiceRecognition->OnRecognitionResult.AddUniqueDynamic(
		this, &UInitialConsonantQuizComponent::HandleVoiceResult);
	return true;
}

void UInitialConsonantQuizComponent::ReleaseVoiceRecognition()
{
	if (!IsValid(VoiceRecognition))
	{
		VoiceRecognition = nullptr;
		return;
	}

	VoiceRecognition->OnRecognitionResult.RemoveDynamic(
		this, &UInitialConsonantQuizComponent::HandleVoiceResult);
	VoiceRecognition->StopListening();

	if (bOwnsVoiceRecognition)
	{
		VoiceRecognition->DestroyComponent();
		bOwnsVoiceRecognition = false;
	}
	VoiceRecognition = nullptr;
}

void UInitialConsonantQuizComponent::EnsureWidgetComponent()
{
	if (QuizWidgetComponent || !GetOwner())
	{
		return;
	}

	QuizWidgetComponent = NewObject<UWidgetComponent>(GetOwner(), TEXT("RuntimeInitialConsonantQuizPanel"));
	QuizWidgetComponent->SetupAttachment(GetOwner()->GetRootComponent());
	QuizWidgetComponent->RegisterComponent();
	QuizWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	QuizWidgetComponent->SetDrawSize(DrawSize);
	QuizWidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	QuizWidgetComponent->SetBlendMode(EWidgetBlendMode::Transparent);
	QuizWidgetComponent->SetTwoSided(true);
	QuizWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	QuizWidgetComponent->SetTranslucentSortPriority(10000);
	UClass* PanelClass = QuizWidgetClass
		? QuizWidgetClass.Get()
		: UInitialConsonantQuizWidget::StaticClass();
	QuizWidgetComponent->SetWidgetClass(PanelClass);
	QuizWidgetComponent->SetHiddenInGame(true, true);
	QuizWidgetComponent->SetVisibility(false);
}

void UInitialConsonantQuizComponent::ShowPanel()
{
	// No game instance means no local player to show a panel to, e.g. an automation world.
	if (!bShowQuizPanel || !GetWorld() || !GetWorld()->GetGameInstance())
	{
		return;
	}

	EnsureWidgetComponent();
	if (!QuizWidgetComponent)
	{
		return;
	}

	QuizWidgetComponent->SetDrawSize(DrawSize);
	QuizWidgetComponent->InitWidget();
	PlaceWidgetInFrontOfPlayer();
	ApplyPromptToWidget();
	QuizWidgetComponent->SetHiddenInGame(false, true);
	QuizWidgetComponent->SetVisibility(true);
	SetComponentTickEnabled(bFaceViewer);
}

void UInitialConsonantQuizComponent::HidePanel()
{
	SetComponentTickEnabled(false);

	if (!QuizWidgetComponent)
	{
		return;
	}

	QuizWidgetComponent->SetHiddenInGame(true, true);
	QuizWidgetComponent->SetVisibility(false);
}

void UInitialConsonantQuizComponent::PlaceWidgetInFrontOfPlayer()
{
	FVector ViewLocation;
	FRotator ViewRotation;
	if (!ResolveViewPoint(ViewLocation, ViewRotation))
	{
		// No view yet (automation, or a Pawn that has not spawned): keep the panel on the owner.
		PanelLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		QuizWidgetComponent->SetWorldLocation(PanelLocation);
		QuizWidgetComponent->SetWorldScale3D(FVector(WorldScale));
		return;
	}

	// Placed once, on a level plane in front of the player. The panel does not follow the head.
	FVector Forward = ViewRotation.Vector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		Forward = FVector::ForwardVector;
	}

	PanelLocation = ViewLocation + Forward * ViewDistance + FVector(0.0f, 0.0f, ViewHeightOffset);
	QuizWidgetComponent->SetWorldLocation(PanelLocation);
	QuizWidgetComponent->SetWorldScale3D(FVector(WorldScale));
	// Shared facing rule: WidgetComponent faces along local +X, and roll stays locked.
	QuizWidgetComponent->SetWorldRotation(
		UScenarioInteractionGuideComponent::CalculateGuideFacingRotation(PanelLocation, ViewLocation));
}

void UInitialConsonantQuizComponent::UpdatePanelFacing()
{
	if (!bFaceViewer || !QuizWidgetComponent || QuizState == EInitialConsonantQuizState::Idle)
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	if (!ResolveViewPoint(ViewLocation, ViewRotation))
	{
		return;
	}

	QuizWidgetComponent->SetWorldRotation(
		UScenarioInteractionGuideComponent::CalculateGuideFacingRotation(PanelLocation, ViewLocation));
}

bool UInitialConsonantQuizComponent::ResolveViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		TInlineComponentArray<UCameraComponent*> Cameras(PlayerPawn);
		for (const UCameraComponent* Camera : Cameras)
		{
			if (IsValid(Camera) && Camera->IsActive())
			{
				OutLocation = Camera->GetComponentLocation();
				OutRotation = Camera->GetComponentRotation();
				return true;
			}
		}
	}

	if (const APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0))
	{
		Controller->GetPlayerViewPoint(OutLocation, OutRotation);
		return true;
	}

	return false;
}

void UInitialConsonantQuizComponent::ApplyPromptToWidget()
{
	if (UInitialConsonantQuizWidget* Widget = QuizWidgetComponent
		? Cast<UInitialConsonantQuizWidget>(QuizWidgetComponent->GetUserWidgetObject())
		: nullptr)
	{
		Widget->SetQuizPrompt(ActiveQuiz.PromptTitle, ActiveQuiz.QuestionText, ActiveQuiz.GetDisplayConsonants());
	}
}

void UInitialConsonantQuizComponent::ApplyStatusToWidget(const FText& Status, const FLinearColor& Color)
{
	if (UInitialConsonantQuizWidget* Widget = QuizWidgetComponent
		? Cast<UInitialConsonantQuizWidget>(QuizWidgetComponent->GetUserWidgetObject())
		: nullptr)
	{
		Widget->SetStatus(Status, Color);
	}
}

void UInitialConsonantQuizComponent::ApplyFooterToWidget(const FText& Footer)
{
	if (UInitialConsonantQuizWidget* Widget = QuizWidgetComponent
		? Cast<UInitialConsonantQuizWidget>(QuizWidgetComponent->GetUserWidgetObject())
		: nullptr)
	{
		Widget->SetFooter(Footer);
	}
}

FText UInitialConsonantQuizComponent::BuildAttemptFooter() const
{
	if (ActiveQuiz.MaxAttempts > 0 && AttemptCount > 0)
	{
		return FText::Format(LOCTEXT("AttemptsLeft", "남은 기회 {0}회"),
			FText::AsNumber(FMath::Max(ActiveQuiz.MaxAttempts - AttemptCount, 0)));
	}

	return ActiveQuiz.HintText;
}

#undef LOCTEXT_NAMESPACE
