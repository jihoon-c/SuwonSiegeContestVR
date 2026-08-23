#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScenarioInteractionGuideWidget.generated.h"

class UTextBlock;

/** Native, Feature-agnostic prompt rendered above the active Scenario target. */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UScenarioInteractionGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Scenario|Guide")
	void SetGuide(FText ActionLabel, FText Instruction, FLinearColor AccentColor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActionText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> InstructionText;

private:
	FText PendingActionLabel;
	FText PendingInstruction;
	FLinearColor PendingAccentColor = FLinearColor(0.15f, 0.75f, 1.0f, 1.0f);
};
