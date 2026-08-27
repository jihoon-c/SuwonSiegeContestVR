#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VoiceKeywordTestWidget.generated.h"

class UTextBlock;

/**
 * Native HUD panel for the voice keyword detection test level. Shows recognizer status, the last
 * recognized sentence, whether a target keyword was found in it, and a rolling history of
 * utterances. Built entirely in RebuildWidget() so no WBP designer work is needed, matching
 * UInitialConsonantQuizWidget / USubtitleWidget.
 */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UVoiceKeywordTestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** One-line recognizer/listening status, e.g. "듣는 중" or "모델 로딩 중". */
	UFUNCTION(BlueprintCallable, Category = "Voice|Test UI")
	void SetStatus(FText Status, FLinearColor StatusColor);

	/** Full text the backend produced for the last utterance. */
	UFUNCTION(BlueprintCallable, Category = "Voice|Test UI")
	void SetLastRecognized(FText Sentence);

	/** Large keyword readout. Empty keyword shows a neutral placeholder instead of blanking. */
	UFUNCTION(BlueprintCallable, Category = "Voice|Test UI")
	void SetDetectedKeyword(FText Keyword, bool bHasKeyword);

	/** Prepends one line to the rolling history log (most recent first). */
	UFUNCTION(BlueprintCallable, Category = "Voice|Test UI")
	void AppendHistoryLine(FText Line);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LastSentenceText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> KeywordText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HistoryText;

private:
	FText PendingStatus;
	FLinearColor PendingStatusColor = FLinearColor::White;
	FText PendingLastSentence;
	FText PendingKeyword;
	bool bPendingHasKeyword = false;

	TArray<FText> HistoryLines;
	static constexpr int32 MaxHistoryLines = 12;

	void RefreshHistoryDisplay();
};
