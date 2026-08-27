#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Quiz/InitialConsonantQuizTypes.h"
#include "Core/Voice/VoiceRecognitionTypes.h"
#include "Templates/SubclassOf.h"
#include "InitialConsonantQuizComponent.generated.h"

class UInitialConsonantQuizSet;
class UUserWidget;
class UVoiceRecognitionComponent;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInitialConsonantQuizStarted, FInitialConsonantQuizDefinition, Quiz);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInitialConsonantQuizAttempt,
	FString, SpokenAnswer, bool, bCorrect, int32, AttemptCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInitialConsonantQuizFinished,
	FName, QuizID, bool, bCorrect, EInitialConsonantQuizOutcome, Outcome);

/**
 * Runs one voice-answered initial-consonant quiz at a time.
 *
 * Add this component next to whatever drives a sequence, author the quizzes inline or in a shared
 * UInitialConsonantQuizSet, then call StartQuiz and listen to OnQuizFinished. The component owns
 * the panel, the attempt bookkeeping and the microphone lifetime: speech capture starts with the
 * quiz and always stops when it ends.
 *
 * Scenario-driven levels can skip the manual calls entirely and use UScenarioQuizBridgeComponent.
 */
UCLASS(ClassGroup = (Quiz), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UInitialConsonantQuizComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInitialConsonantQuizComponent();

	/** Starts an authored quiz by ID. Fails when the ID is unknown or another quiz is running. */
	UFUNCTION(BlueprintCallable, Category = "Quiz")
	bool StartQuiz(FName QuizID);

	/** Starts a quiz built at runtime, e.g. from a level's own data. */
	UFUNCTION(BlueprintCallable, Category = "Quiz")
	bool StartQuizDefinition(const FInitialConsonantQuizDefinition& Quiz);

	/** Ends the quiz with no result and releases the microphone and panel. */
	UFUNCTION(BlueprintCallable, Category = "Quiz")
	void CancelQuiz();

	/**
	 * Shared answer ingress for speech, a debug button, or a UI selection.
	 * Returns true when the answer was accepted as correct.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quiz")
	bool SubmitAnswer(const FString& Answer);

	/** Finishes the quiz as unanswered, revealing the answer when the quiz allows it. */
	UFUNCTION(BlueprintCallable, Category = "Quiz")
	void GiveUp();

	UFUNCTION(BlueprintPure, Category = "Quiz")
	bool IsQuizActive() const { return QuizState != EInitialConsonantQuizState::Idle; }

	UFUNCTION(BlueprintPure, Category = "Quiz")
	EInitialConsonantQuizState GetQuizState() const { return QuizState; }

	UFUNCTION(BlueprintPure, Category = "Quiz")
	FInitialConsonantQuizDefinition GetActiveQuiz() const { return ActiveQuiz; }

	UFUNCTION(BlueprintPure, Category = "Quiz")
	int32 GetAttemptCount() const { return AttemptCount; }

	UFUNCTION(BlueprintPure, Category = "Quiz")
	bool FindQuizDefinition(FName QuizID, FInitialConsonantQuizDefinition& OutQuiz) const;

	/** The recognizer in use, once a quiz has resolved one. */
	UFUNCTION(BlueprintPure, Category = "Quiz|Voice")
	UVoiceRecognitionComponent* GetVoiceRecognition() const { return VoiceRecognition; }

	UPROPERTY(BlueprintAssignable, Category = "Quiz|Events")
	FOnInitialConsonantQuizStarted OnQuizStarted;

	UPROPERTY(BlueprintAssignable, Category = "Quiz|Events")
	FOnInitialConsonantQuizAttempt OnQuizAttempt;

	UPROPERTY(BlueprintAssignable, Category = "Quiz|Events")
	FOnInitialConsonantQuizFinished OnQuizFinished;

	/** Quizzes authored on this component. Searched before QuizSet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz", meta = (TitleProperty = "QuizID"))
	TArray<FInitialConsonantQuizDefinition> Quizzes;

	/** Optional shared library so several experiences can reuse the same quizzes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	TObjectPtr<UInitialConsonantQuizSet> QuizSet;

	/**
	 * Turn this off when an experience presents the quiz with its own UI and only wants the flow,
	 * the answer judging and the events from this component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI")
	bool bShowQuizPanel = true;

	/** Leave empty to use the native panel. A Blueprint subclass replaces the visuals. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI")
	TSubclassOf<UUserWidget> QuizWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI")
	FVector2D DrawSize = FVector2D(1400.0f, 900.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI", meta = (ClampMin = "0.001"))
	float WorldScale = 0.12f;

	/** Distance in front of the player where the panel appears. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI", meta = (ClampMin = "20.0", Units = "cm"))
	float ViewDistance = 200.0f;

	/** Vertical offset from eye height. Slightly below eye level reads comfortably in VR. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI", meta = (Units = "cm"))
	float ViewHeightOffset = -15.0f;

	/**
	 * Keeps the panel turned towards the player. The panel never follows the head position:
	 * head-locked UI is uncomfortable in VR.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI")
	bool bFaceViewer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Voice")
	bool bUseVoiceRecognition = true;

	/** Optional explicit recognizer. Left empty, the component searches the Pawn and the level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Voice")
	TObjectPtr<UVoiceRecognitionComponent> VoiceRecognitionOverride;

	/**
	 * Recognizer added to the owner when the level has none. Defaults to the on-device sherpa-onnx
	 * backend; swap in UMockVoiceRecognitionComponent to run a level without speech input.
	 * Clear it to leave the quiz on manual answers only.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Voice")
	TSubclassOf<UVoiceRecognitionComponent> FallbackVoiceRecognitionClass;

	/**
	 * Resolves the recognizer at level load instead of at the first quiz. The speech model takes
	 * seconds to load, and this hides that behind the rest of the level start.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Voice")
	bool bPreloadVoiceRecognitionOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI|Text")
	FText ListeningStatusText = NSLOCTEXT("Quiz", "Listening", "정답을 말해보세요");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI|Text")
	FText CorrectStatusText = NSLOCTEXT("Quiz", "Correct", "정답입니다!");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI|Text")
	FText RetryStatusText = NSLOCTEXT("Quiz", "Retry", "다시 말해보세요");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|UI|Text")
	FText VoiceUnavailableStatusText = NSLOCTEXT("Quiz", "VoiceUnavailable", "음성 인식을 사용할 수 없습니다");

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleVoiceResult(FVoiceRecognitionResult Result);

	void BeginListeningAttempt();
	/** Keeps a quiz moving when no recognizer can listen for this attempt. */
	void HandleAttemptTimeout();
	void FinishQuiz(bool bCorrect, EInitialConsonantQuizOutcome Outcome);
	void HandleResultHoldElapsed();

	bool ResolveVoiceRecognition();
	void ReleaseVoiceRecognition();

	void ShowPanel();
	void HidePanel();
	void EnsureWidgetComponent();
	void PlaceWidgetInFrontOfPlayer();
	void UpdatePanelFacing();
	bool ResolveViewPoint(FVector& OutLocation, FRotator& OutRotation) const;
	void ApplyPromptToWidget();
	void ApplyStatusToWidget(const FText& Status, const FLinearColor& Color);
	void ApplyFooterToWidget(const FText& Footer);
	FText BuildAttemptFooter() const;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> QuizWidgetComponent;

	UPROPERTY(Transient)
	TObjectPtr<UVoiceRecognitionComponent> VoiceRecognition;

	/** True when this component created the recognizer and therefore owns its lifetime. */
	bool bOwnsVoiceRecognition = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Quiz", meta = (AllowPrivateAccess = "true"))
	EInitialConsonantQuizState QuizState = EInitialConsonantQuizState::Idle;

	UPROPERTY(Transient)
	FInitialConsonantQuizDefinition ActiveQuiz;

	int32 AttemptCount = 0;
	bool bPendingSuccess = false;
	FTimerHandle ResultHoldHandle;
	FTimerHandle AttemptTimeoutHandle;
	FVector PanelLocation = FVector::ZeroVector;
};
