#pragma once

#include "Core/Scenario/ScenarioManagerActor.h"
#include "Main/Education/MainEducationTypes.h"
#include "MainEducationScenarioManagerActor.generated.h"

class UMainEducationScenarioDefinition;
class UVRHUDComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMainEducationContentRequested, FMainEducationContent, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMainEducationQuizFeedback, bool, bCorrect, FText, Feedback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMainEducationExperienceUnavailable, FName, RouteID, FText, Reason);

/**
 * Main-level adapter for presentation, quiz input, and Core Experience travel.
 * Speech capture/STT deliberately remains an empty Blueprint integration port.
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

	/** Implement in the separate voice-recognition Blueprint/module. Base implementation is intentionally empty. */
	UFUNCTION(BlueprintNativeEvent, Category = "Main Education|Voice")
	void RequestVoiceRecognition(FName QuizID);
	virtual void RequestVoiceRecognition_Implementation(FName QuizID);

	/** Implement in the separate voice-recognition Blueprint/module. Base implementation is intentionally empty. */
	UFUNCTION(BlueprintNativeEvent, Category = "Main Education|Voice")
	void CancelVoiceRecognition();
	virtual void CancelVoiceRecognition_Implementation();

	UPROPERTY(BlueprintAssignable, Category = "Main Education|Events")
	FOnMainEducationContentRequested OnEducationContentRequested;

	UPROPERTY(BlueprintAssignable, Category = "Main Education|Events")
	FOnMainEducationQuizFeedback OnQuizFeedback;

	UPROPERTY(BlueprintAssignable, Category = "Main Education|Events")
	FOnMainEducationExperienceUnavailable OnExperienceUnavailable;

	/** Mirrors text to the common VR HUD when the Pawn owns one. Rich image/quiz UI still uses events above. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Presentation")
	bool bMirrorTextToVRHUD = true;

	/** Enable only when a MainLevelIntroActor in this level is responsible for calling StartEducationAfterIntro. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Main Education|Intro")
	bool bWaitForIntroSequence = false;

protected:
	virtual void BeginPlay() override;
	virtual bool ShouldAutoStartScenario() const override;

private:
	UFUNCTION()
	void HandleInteractionRequested(FScenarioInteraction Interaction);

	bool BeginExperienceTravel(const FScenarioInteraction& Interaction);
	UMainEducationScenarioDefinition* GetEducationDefinition() const;
	UVRHUDComponent* ResolveVRHUD() const;

	UPROPERTY(Transient)
	FMainEducationContent CurrentContent;

	FName UnavailableRouteID;
	int32 QuizAttemptCount = 0;
	bool bEducationStartedByIntro = false;
};
