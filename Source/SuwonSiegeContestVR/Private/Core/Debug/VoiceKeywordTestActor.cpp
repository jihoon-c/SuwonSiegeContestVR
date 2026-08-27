#include "Core/Debug/VoiceKeywordTestActor.h"

#include "Blueprint/UserWidget.h"
#include "Core/Debug/VoiceKeywordTestWidget.h"
#include "Core/Voice/SherpaVoiceRecognitionComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVoiceKeywordTestActor::AVoiceKeywordTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	VoiceRecognition = CreateDefaultSubobject<USherpaVoiceRecognitionComponent>(TEXT("VoiceRecognition"));
	WidgetClass = UVoiceKeywordTestWidget::StaticClass();
}

void AVoiceKeywordTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (WidgetClass)
	{
		TestWidget = CreateWidget<UVoiceKeywordTestWidget>(GetWorld(), WidgetClass);
		if (TestWidget)
		{
			TestWidget->AddToViewport();
		}
	}

	if (VoiceRecognition)
	{
		VoiceRecognition->OnRecognitionResult.AddDynamic(this, &AVoiceKeywordTestActor::HandleVoiceResult);
		VoiceRecognition->OnVoiceStateChanged.AddDynamic(this, &AVoiceKeywordTestActor::HandleVoiceStateChanged);
	}

	RefreshStatusDisplay();

	// The model loads in the background; poll until StartListening actually succeeds instead of
	// assuming one BeginPlay-timed attempt is enough.
	TryStartListening();
	GetWorldTimerManager().SetTimer(ListenRetryHandle, this,
		&AVoiceKeywordTestActor::TryStartListening, ListenRetryInterval, true);
}

void AVoiceKeywordTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ListenRetryHandle);

	if (VoiceRecognition)
	{
		VoiceRecognition->StopListening();
	}

	if (TestWidget)
	{
		TestWidget->RemoveFromParent();
		TestWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AVoiceKeywordTestActor::TryStartListening()
{
	if (!VoiceRecognition || VoiceRecognition->IsListening())
	{
		return;
	}

	FVoiceRecognitionRequest Request;
	Request.RequestID = TEXT("VoiceKeywordTest");
	Request.Keywords = TestKeywords;
	Request.ListenDuration = ListenDuration;

	VoiceRecognition->StartListening(Request);
	RefreshStatusDisplay();
}

void AVoiceKeywordTestActor::HandleVoiceResult(const FVoiceRecognitionResult Result)
{
	++UtteranceCount;

	if (TestWidget)
	{
		const FString Sentence = Result.RecognizedText.IsEmpty() ? TEXT("(무음)") : Result.RecognizedText;
		TestWidget->SetLastRecognized(FText::FromString(Sentence));
		TestWidget->SetDetectedKeyword(FText::FromString(Result.MatchedKeyword), Result.IsMatch());

		const FString HistoryLine = Result.IsMatch()
			? FString::Printf(TEXT("#%d \"%s\" -> 키워드: %s"), UtteranceCount, *Sentence, *Result.MatchedKeyword)
			: FString::Printf(TEXT("#%d \"%s\""), UtteranceCount, *Sentence);
		TestWidget->AppendHistoryLine(FText::FromString(HistoryLine));
	}

	// Every report ends the request (see UVoiceRecognitionComponent::FinishRequest), so listening
	// has to be re-armed here to keep the test tool continuously live.
	TryStartListening();
}

void AVoiceKeywordTestActor::HandleVoiceStateChanged(const EVoiceRecognitionState OldState,
	const EVoiceRecognitionState NewState)
{
	RefreshStatusDisplay();
}

void AVoiceKeywordTestActor::RefreshStatusDisplay()
{
	if (!TestWidget || !VoiceRecognition)
	{
		return;
	}

	FText Status;
	FLinearColor Color = FLinearColor::White;

	switch (VoiceRecognition->GetVoiceState())
	{
	case EVoiceRecognitionState::Listening:
		Status = NSLOCTEXT("VoiceKeywordTest", "Listening", "듣는 중");
		Color = FLinearColor(0.35f, 0.9f, 0.45f, 1.0f);
		break;
	case EVoiceRecognitionState::Processing:
		Status = NSLOCTEXT("VoiceKeywordTest", "Processing", "인식 처리 중");
		Color = FLinearColor(0.95f, 0.85f, 0.35f, 1.0f);
		break;
	case EVoiceRecognitionState::Unavailable:
		Status = FText::FromString(FString::Printf(TEXT("사용 불가 — %s"), *VoiceRecognition->GetBackendDescription()));
		Color = FLinearColor(0.95f, 0.4f, 0.35f, 1.0f);
		break;
	case EVoiceRecognitionState::Idle:
	default:
		Status = FText::FromString(FString::Printf(TEXT("대기 중 — %s"), *VoiceRecognition->GetBackendDescription()));
		Color = FLinearColor(0.75f, 0.78f, 0.85f, 1.0f);
		break;
	}

	TestWidget->SetStatus(Status, Color);
}
