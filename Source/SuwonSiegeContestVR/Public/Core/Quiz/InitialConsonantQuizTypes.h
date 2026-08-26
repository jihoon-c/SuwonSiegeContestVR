#pragma once

#include "CoreMinimal.h"
#include "InitialConsonantQuizTypes.generated.h"

UENUM(BlueprintType)
enum class EInitialConsonantQuizState : uint8
{
	Idle,
	/** The panel is up and the recognizer is listening for this attempt. */
	Listening,
	/** An answer arrived and is being shown before the handoff. */
	Feedback
};

UENUM(BlueprintType)
enum class EInitialConsonantQuizOutcome : uint8
{
	Correct,
	/** Attempts ran out. The flow continues; the answer is revealed when configured. */
	Exhausted,
	/** Someone canceled the quiz, e.g. the experience restarted. */
	Canceled
};

/**
 * One reusable quiz. Only Answer is mandatory: the displayed consonants are derived from it,
 * so authoring a quiz in another experience is a single array entry.
 */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FInitialConsonantQuizDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	FName QuizID;

	/** Header that tells the player this is a quiz. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	FText PromptTitle = NSLOCTEXT("Quiz", "DefaultPromptTitle", "초성 퀴즈");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz", meta = (MultiLine = true))
	FText QuestionText;

	/** Leave empty to derive the consonants from Answer, e.g. "옹성" shows "ㅇ ㅅ". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	FText InitialConsonants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	FText Answer;

	/** Extra spellings accepted as correct. Useful for absorbing recognizer quirks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	TArray<FText> AcceptedAnswers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz", meta = (MultiLine = true))
	FText HintText;

	/** Seconds of listening per attempt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Flow", meta = (ClampMin = "1.0", Units = "s"))
	float ListenDuration = 8.0f;

	/** 0 keeps listening until the player answers correctly or the quiz is canceled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Flow", meta = (ClampMin = "0"))
	int32 MaxAttempts = 3;

	/** Educational fallback: show the answer instead of blocking the experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Flow")
	bool bRevealAnswerOnFail = true;

	/** How long the result stays on screen before the quiz hands control back. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz|Flow", meta = (ClampMin = "0.0", Units = "s"))
	float ResultDisplayDuration = 3.0f;

	bool IsValid() const { return !QuizID.IsNone() && !Answer.IsEmpty(); }

	/** Answer plus aliases, in the order they were authored. */
	TArray<FString> GetAcceptedAnswerStrings() const;

	/** Authored consonants when present, otherwise the ones derived from Answer. */
	FText GetDisplayConsonants() const;
};
