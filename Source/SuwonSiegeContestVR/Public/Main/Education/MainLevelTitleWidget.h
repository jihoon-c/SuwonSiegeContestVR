#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainLevelTitleWidget.generated.h"

class UFont;
class UTextBlock;

/**
 * Native UMG text used by the Main intro title.
 * UMG supports runtime composite fonts such as GmarketSans, unlike TextRenderComponent's
 * offline font-atlas rendering path.
 */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UMainLevelTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Configure(const FText& InText, UFont* InFont, int32 InFontSize, const FLinearColor& InColor);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void ApplyPendingStyle();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TextBlock;

	UPROPERTY(Transient)
	TObjectPtr<UFont> PendingFont;

	FText PendingText;
	FLinearColor PendingColor = FLinearColor::White;
	int32 PendingFontSize = 72;
};
