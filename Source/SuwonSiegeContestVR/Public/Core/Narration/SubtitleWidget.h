#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SubtitleWidget.generated.h"

class UTextBlock;

/** Native fallback subtitle panel used by the VR Pawn. It can be replaced by a Blueprint widget. */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API USubtitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Narration|Subtitle")
	void SetSubtitle(FText SpeakerName, FText Subtitle);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SubtitleText;
};
