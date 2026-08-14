#include "Core/Narration/SubtitleWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

TSharedRef<SWidget> USubtitleWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SubtitleBackground"));
		Background->SetBrushColor(FLinearColor(0.01f, 0.015f, 0.025f, 0.82f));
		Background->SetPadding(FMargin(28.0f, 18.0f));
		WidgetTree->RootWidget = Background;

		UVerticalBox* TextLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SubtitleTextLayout"));
		Background->SetContent(TextLayout);

		SpeakerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SpeakerText"));
		SpeakerText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.72f, 0.18f, 1.0f)));
		SpeakerText->SetJustification(ETextJustify::Center);
		FSlateFontInfo SpeakerFont = SpeakerText->GetFont();
		SpeakerFont.Size = 24;
		SpeakerText->SetFont(SpeakerFont);
		TextLayout->AddChildToVerticalBox(SpeakerText);

		SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubtitleText"));
		SubtitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		SubtitleText->SetJustification(ETextJustify::Center);
		SubtitleText->SetAutoWrapText(true);
		FSlateFontInfo SubtitleFont = SubtitleText->GetFont();
		SubtitleFont.Size = 32;
		SubtitleText->SetFont(SubtitleFont);
		if (UVerticalBoxSlot* SubtitleSlot = TextLayout->AddChildToVerticalBox(SubtitleText))
		{
			SubtitleSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
		}
	}

	return Super::RebuildWidget();
}

void USubtitleWidget::SetSubtitle(const FText SpeakerName, const FText Subtitle)
{
	if (SpeakerText)
	{
		SpeakerText->SetText(SpeakerName);
		SpeakerText->SetVisibility(SpeakerName.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (SubtitleText)
	{
		SubtitleText->SetText(Subtitle);
	}
}
