#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InitialConsonantQuizWidget.generated.h"

class UTextBlock;

/**
 * Native, Feature-agnostic quiz panel. A Blueprint subclass can replace the visuals entirely;
 * the quiz component only calls the three functions below, so any WBP that overrides them works.
 */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UInitialConsonantQuizWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Title, question and the large consonants. Called once when the quiz opens. */
	UFUNCTION(BlueprintCallable, Category = "Quiz|UI")
	void SetQuizPrompt(FText Title, FText Question, FText Consonants);

	/** Player-facing line under the consonants, e.g. "정답을 말해보세요". */
	UFUNCTION(BlueprintCallable, Category = "Quiz|UI")
	void SetStatus(FText Status, FLinearColor StatusColor);

	/** Secondary line used for hints, attempts left, and the revealed answer. */
	UFUNCTION(BlueprintCallable, Category = "Quiz|UI")
	void SetFooter(FText Footer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	/** Lets a Blueprint subclass react without rebuilding the native tree. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Quiz|UI", meta = (DisplayName = "On Quiz Prompt Set"))
	void BP_OnQuizPromptSet(const FText& Title, const FText& Question, const FText& Consonants);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quiz|UI", meta = (DisplayName = "On Quiz Status Set"))
	void BP_OnQuizStatusSet(const FText& Status, const FLinearColor& StatusColor);

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> QuestionText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ConsonantText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FooterText;

private:
	FText PendingTitle;
	FText PendingQuestion;
	FText PendingConsonants;
	FText PendingStatus;
	FText PendingFooter;
	FLinearColor PendingStatusColor = FLinearColor(0.95f, 0.85f, 0.35f, 1.0f);
};
