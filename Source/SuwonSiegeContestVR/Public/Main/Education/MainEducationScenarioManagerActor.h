#pragma once

#include "Core/Quiz/InitialConsonantQuizTypes.h"
#include "Core/Scenario/ScenarioManagerActor.h"
#include "Main/Education/MainEducationTypes.h"
#include "MainEducationScenarioManagerActor.generated.h"

class UInitialConsonantQuizComponent;
class UMainEducationScenarioDefinition;
class UVRHUDComponent;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMainEducationContentRequested, FMainEducationContent, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMainEducationQuizFeedback, bool, bCorrect, FText, Feedback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMainEducationExperienceUnavailable, FName, RouteID, FText, Reason);

/**
 * Main-level adapter for presentation, quiz input, and Core Experience travel.
 *
 * Quiz steps are answered by voice: the actor turns the step's FMainEducationContent into a Core
 * FInitialConsonantQuizDefinition and hands it to UInitialConsonantQuizComponent, which owns the
 * panel, the attempts and the microphone. SubmitQuizAnswer stays open for a button or the console,
 * so the flow still works with no speech backend at all.
 */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AMainEducationScenarioManagerActor : public AScenarioManagerActor
{
	GENERATED_BODY()

public:
	AMainEducationScenarioManagerActor();

	/** Completes the currently displayed instructor/image/comparison/summary step. */
	UFUNCTION(BlueprintCallable, Category = "Main Education|Presentation")
	bool ContinuePresentation();

	/** Shared answer ingress for buttons, keyboard debug input, or the future STT implementation. */
	UFUNCTION(BlueprintCallable, Category = "Main Education|Quiz")
	bool SubmitQuizAnswer(const FString& Answer);

	/** Development fallback for routes whose Experience asset/level has not been created yet. */
	UFUNCTION(BlueprintCallable, Category = "Main Education|Experience")
	bool SkipUnavailableExperience();

	/** Starts the configured education scenario after the level intro has completed. Safe to call only once. */
	UFUNCTION(BlueprintCallable, Category = "Main Education|Intro")
	bool StartEducationAfterIntro();

	UFUNCTION(BlueprintPure, Category = "Main Education")
	FMainEducationContent GetCurrentEducationContent() const { return CurrentContent; }

	UFUNCTION(BlueprintPure, Category = "Main Education|Quiz")
	int32 GetQuizAttemptCount() const { return QuizAttemptCount; }

	/** Opens the Core initial-consonant quiz for the current step. Override to use another backend. */
	UFUNCTION(BlueprintNativeEvent, Category = "Main Education|Voice")
	void RequestVoiceRecognition(FName QuizID);
	virtual void RequestVoiceRecognition_Implementation(FName QuizID);

	/** Closes the quiz and releases the microphone. Override alongside RequestVoiceRecognition. */
	UFUNCTION(BlueprintNativeEvent, Category = "Main Education|Voice")
	void CancelVoiceRecognition();
	virtual void CancelVoiceRecognition_Implementation();

	/** The quiz runtime that owns the panel and the microphone during Quiz steps. */
	UFUNCTION(BlueprintPure, Category = "Main Education|Quiz")
	UInitialConsonantQuizComponent* GetEducationQuiz() const { return EducationQuiz; }

	UPROPERTY(BlueprintAssignable, Category = "Main Education|Events")
	FOnMainEducationContentRequested OnEducationContentRequested;

	UPROPERTY(BlueprintAssignable, Category = "Main Education|Events")
	FOnMainEducationQuizFeedback OnQuizFeedback;

	UPROPERTY(BlueprintAssignable, Category = "Main Education|Events")
	FOnMainEducationExperienceUnavailable OnExperienceUnavailable;

	/** Mirrors text to the common VR HUD when the Pawn owns one. Rich image/quiz UI still uses events above. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Presentation")
	bool bMirrorTextToVRHUD = true;

	/** Camera-relative position of the clickable image panel. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Presentation")
	FVector PresentationPanelOffset = FVector(165.0f, 0.0f, -5.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Presentation", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float PresentationPanelWorldScale = 0.075f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Presentation")
	FVector2D PresentationPanelDrawSize = FVector2D(1100.0f, 850.0f);

	/** Enable only when a MainLevelIntroActor in this level is responsible for calling StartEducationAfterIntro. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Intro")
	bool bWaitForIntroSequence = false;

	/** Turn off to answer quizzes with buttons or the console only; no microphone is opened then. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Quiz")
	bool bUseVoiceQuiz = true;

	/** Seconds of listening per attempt. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Quiz", meta = (ClampMin = "1.0", Units = "s"))
	float QuizListenDuration = 8.0f;

	/**
	 * Wrong answers allowed before the answer is revealed and the education continues.
	 * 0 keeps asking; leave it above zero so a broken microphone cannot strand the course.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Quiz", meta = (ClampMin = "0"))
	int32 QuizMaxAttempts = 3;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ShouldAutoStartScenario() const override;

private:
	UFUNCTION()
	void HandleInteractionRequested(FScenarioInteraction Interaction);

	UFUNCTION()
	void HandleEducationQuizFinished(FName QuizID, bool bCorrect, EInitialConsonantQuizOutcome Outcome);

	/** Builds the Core quiz from the Main content so the question lives in one place only. */
	FInitialConsonantQuizDefinition BuildQuizFromContent(const FMainEducationContent& Content) const;

	/** Clears the quiz step and advances the Scenario, whatever the answer was. */
	bool CompleteQuizInteraction();

	bool BeginExperienceTravel(const FScenarioInteraction& Interaction);
	UMainEducationScenarioDefinition* GetEducationDefinition() const;
	UVRHUDComponent* ResolveVRHUD() const;
	void ShowPresentationPanel(const FMainEducationContent& Content);
	void HidePresentationPanel();

	UPROPERTY(VisibleAnywhere, Category = "Main Education|Presentation")
	TObjectPtr<UWidgetComponent> PresentationWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Main Education|Quiz")
	TObjectPtr<UInitialConsonantQuizComponent> EducationQuiz;

	UPROPERTY(Transient)
	FMainEducationContent CurrentContent;

	FName UnavailableRouteID;
	int32 QuizAttemptCount = 0;
	bool bEducationStartedByIntro = false;
};
