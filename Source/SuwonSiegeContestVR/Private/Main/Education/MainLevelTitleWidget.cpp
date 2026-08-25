#include "Main/Education/MainLevelTitleWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Engine/Font.h"

TSharedRef<SWidget> UMainLevelTitleWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("IntroTitleText"));
		TextBlock->SetJustification(ETextJustify::Center);
		TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		TextBlock->SetAutoWrapText(true);
		TextBlock->SetWrapTextAt(1500.0f);
		TextBlock->SetShadowOffset(FVector2D(3.0f, 3.0f));
		TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
		TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetTree->RootWidget = TextBlock;
	}

	ApplyPendingStyle();
	return Super::RebuildWidget();
}

void UMainLevelTitleWidget::Configure(
	const FText& InText, UFont* InFont, const int32 InFontSize, const FLinearColor& InColor)
{
	PendingText = InText;
	PendingFont = InFont;
	PendingFontSize = FMath::Max(1, InFontSize);
	PendingColor = InColor;
	ApplyPendingStyle();
}

void UMainLevelTitleWidget::ApplyPendingStyle()
{
	if (!TextBlock)
	{
		return;
	}

	TextBlock->SetText(PendingText);
	TextBlock->SetColorAndOpacity(FSlateColor(PendingColor));
	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = PendingFontSize;
	if (PendingFont)
	{
		FontInfo.FontObject = PendingFont;
		FontInfo.TypefaceFontName = TEXT("Default");
	}
	TextBlock->SetFont(FontInfo);
}
