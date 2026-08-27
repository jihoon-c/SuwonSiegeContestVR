#pragma once

#include "CoreMinimal.h"
#include "VoiceRecognitionTypes.generated.h"

UENUM(BlueprintType)
enum class EVoiceRecognitionState : uint8
{
	/** No capture is running. This is the only valid state while no quiz or voice step is active. */
	Idle,
	Listening,
	/** Capture stopped, the backend is still resolving the utterance. */
	Processing,
	/** No backend could be started, e.g. no microphone permission. Callers fall back to manual input. */
	Unavailable
};

UENUM(BlueprintType)
enum class EVoiceRecognitionOutcome : uint8
{
	KeywordMatched,
	NoMatch,
	TimedOut,
	Canceled,
	Failed
};

/** Backend-neutral capture request. Keywords let a backend bias or restrict its vocabulary. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FVoiceRecognitionRequest
{
	GENERATED_BODY()

	/** Logical owner of the request, e.g. the Quiz ID. Echoed back on the result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	FName RequestID;

	/**
	 * Expected utterances. A keyword-spotting backend listens for exactly these;
	 * a general recognizer boosts them (hotwords / contextual biasing).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	TArray<FString> Keywords;

	/** Seconds of listening before the request times out. 0 listens until stopped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", Units = "s"))
	float ListenDuration = 8.0f;

	/** Results below this confidence are reported as NoMatch. Backends that lack scores report 1.0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConfidenceThreshold = 0.0f;
};

USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FVoiceRecognitionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	FName RequestID;

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	EVoiceRecognitionOutcome Outcome = EVoiceRecognitionOutcome::NoMatch;

	/** Raw text the backend produced. Empty on a timeout. */
	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	FString RecognizedText;

	/** The request keyword the text resolved to, in its original spelling. Empty when nothing matched. */
	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	FString MatchedKeyword;

	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	float Confidence = 0.0f;

	bool IsMatch() const { return Outcome == EVoiceRecognitionOutcome::KeywordMatched; }
};
