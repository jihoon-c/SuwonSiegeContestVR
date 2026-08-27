#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Voice/VoiceRecognitionTypes.h"
#include "VoiceRecognitionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceRecognitionResult, FVoiceRecognitionResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceRecognitionStateChanged,
	EVoiceRecognitionState, OldState, EVoiceRecognitionState, NewState);

/**
 * Backend-neutral speech capture contract. This base class owns request bookkeeping, the listen
 * timeout and keyword matching; it never touches a microphone.
 *
 * A backend subclass implements BeginBackendListening/EndBackendListening and funnels whatever it
 * recognizes into ReportRecognizedText. Mock input, console debug input and a future on-device
 * recognizer therefore share one path. See docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md.
 *
 * Audio is only ever recorded between StartListening and StopListening. Owners must not leave a
 * recognizer running past the interaction that asked for it. A backend may hold the capture device
 * open a little longer to make back-to-back requests seamless, but nothing it hears in that window
 * reaches a buffer or a decoder: see USherpaVoiceRecognitionComponent::CaptureIdleTimeout.
 */
UCLASS(Abstract, ClassGroup = (Voice))
class SUWONSIEGECONTESTVR_API UVoiceRecognitionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVoiceRecognitionComponent();

	/** Starts capture. Returns false when a backend refuses, which leaves the state Unavailable. */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	bool StartListening(const FVoiceRecognitionRequest& Request);

	/** Stops capture without producing a result. Safe to call when already idle. */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StopListening();

	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsListening() const { return State == EVoiceRecognitionState::Listening; }

	UFUNCTION(BlueprintPure, Category = "Voice")
	EVoiceRecognitionState GetVoiceState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Voice")
	FVoiceRecognitionRequest GetActiveRequest() const { return ActiveRequest; }

	/**
	 * Single ingress for every backend, including debug and manual input.
	 * Matches the text against the active request's keywords and broadcasts the result.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	bool ReportRecognizedText(const FString& RecognizedText, float Confidence = 1.0f);

	/** Ends the active request with a non-matching outcome, e.g. when a backend fails mid-capture. */
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void ReportRecognitionFailed(EVoiceRecognitionOutcome Outcome = EVoiceRecognitionOutcome::Failed);

	/** Normalized comparison shared with the quiz system. Partial matches are rejected. */
	UFUNCTION(BlueprintPure, Category = "Voice")
	static bool MatchKeyword(const FString& RecognizedText, const TArray<FString>& Keywords, FString& OutMatchedKeyword);

	/**
	 * Finds a recognizer to use: the player Pawn first, then the PlayerController, then any Actor.
	 * Returns null when the project has no backend placed, which callers treat as "no voice input".
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice", meta = (WorldContext = "WorldContextObject"))
	static UVoiceRecognitionComponent* FindVoiceRecognition(const UObject* WorldContextObject);

	/** One line describing the backend and its readiness. Printed by ssv.voice.status. */
	UFUNCTION(BlueprintPure, Category = "Voice")
	virtual FString GetBackendDescription() const;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Events")
	FOnVoiceRecognitionResult OnRecognitionResult;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Events")
	FOnVoiceRecognitionStateChanged OnVoiceStateChanged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Backend hook: open the microphone. Return false to report the recognizer as unavailable. */
	UFUNCTION(BlueprintNativeEvent, Category = "Voice|Backend")
	bool BeginBackendListening(const FVoiceRecognitionRequest& Request);
	virtual bool BeginBackendListening_Implementation(const FVoiceRecognitionRequest& Request);

	/** Backend hook: close the microphone and release backend resources. */
	UFUNCTION(BlueprintNativeEvent, Category = "Voice|Backend")
	void EndBackendListening();
	virtual void EndBackendListening_Implementation();

	void SetVoiceState(EVoiceRecognitionState NewState);
	void FinishRequest(const FVoiceRecognitionResult& Result);

	UFUNCTION()
	void HandleListenTimeout();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Voice")
	EVoiceRecognitionState State = EVoiceRecognitionState::Idle;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Voice")
	FVoiceRecognitionRequest ActiveRequest;

	FTimerHandle ListenTimeoutHandle;
};
