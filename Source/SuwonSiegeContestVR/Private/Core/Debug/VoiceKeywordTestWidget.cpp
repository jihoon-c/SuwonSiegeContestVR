#include "Core/Debug/VoiceKeywordTestWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
	const FLinearColor NeutralKeywordColor(0.55f, 0.58f, 0.65f, 1.0f);
	const FLinearColor DetectedKeywordColor(1.0f, 0.93f, 0.62f, 1.0f);

	UTextBlock* MakeTestText(UWidgetTree* Tree, const FName Name, const int32 FontSize,
		const FLinearColor& Color, const bool bBold = false)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Text->SetJustification(ETextJustify::Left);
		Text->SetAutoWrapText(true);
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetShadowOffset(FVector2D(2.0f, 2.0f));
		Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));

		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		if (bBold)
		{
			Font.TypefaceFontName = TEXT("Bold");
		}
		Text->SetFont(Font);
		return Text;
	}
}

TSharedRef<SWidget> UVoiceKeywordTestWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TestBackground"));
		Background->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.85f));
		Background->SetPadding(FMargin(36.0f, 28.0f));
		Background->SetHorizontalAlignment(HAlign_Left);
		Background->SetVerticalAlignment(VAlign_Top);
		Background->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetTree->RootWidget = Background;

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TestLayout"));
		Background->SetContent(Layout);

		UTextBlock* TitleText = MakeTestText(WidgetTree, TEXT("TestTitle"), 30,
			FLinearColor(0.15f, 0.75f, 1.0f, 1.0f), true);
		TitleText->SetText(NSLOCTEXT("VoiceKeywordTest", "Title", "키워드 감지 테스트 (옹성 / 신기전)"));
		Layout->AddChildToVerticalBox(TitleText);

		StatusText = MakeTestText(WidgetTree, TEXT("TestStatus"), 22, FLinearColor::White, true);
		if (UVerticalBoxSlot* StatusSlot = Layout->AddChildToVerticalBox(StatusText))
		{
			StatusSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));
		}

		UTextBlock* KeywordLabel = MakeTestText(WidgetTree, TEXT("KeywordLabel"), 18,
			FLinearColor(0.75f, 0.78f, 0.85f, 1.0f));
		KeywordLabel->SetText(NSLOCTEXT("VoiceKeywordTest", "KeywordLabel", "감지된 키워드"));
		if (UVerticalBoxSlot* KeywordLabelSlot = Layout->AddChildToVerticalBox(KeywordLabel))
		{
			KeywordLabelSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 0.0f));
		}

		KeywordText = MakeTestText(WidgetTree, TEXT("KeywordValue"), 72, NeutralKeywordColor, true);
		if (UVerticalBoxSlot* KeywordSlot = Layout->AddChildToVerticalBox(KeywordText))
		{
			KeywordSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 12.0f));
		}

		UTextBlock* SentenceLabel = MakeTestText(WidgetTree, TEXT("SentenceLabel"), 18,
			FLinearColor(0.75f, 0.78f, 0.85f, 1.0f));
		SentenceLabel->SetText(NSLOCTEXT("VoiceKeywordTest", "SentenceLabel", "마지막 인식 문장"));
		Layout->AddChildToVerticalBox(SentenceLabel);

		LastSentenceText = MakeTestText(WidgetTree, TEXT("SentenceValue"), 28, FLinearColor::White);
		if (UVerticalBoxSlot* SentenceSlot = Layout->AddChildToVerticalBox(LastSentenceText))
		{
			SentenceSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 20.0f));
		}

		UTextBlock* HistoryLabel = MakeTestText(WidgetTree, TEXT("HistoryLabel"), 18,
			FLinearColor(0.75f, 0.78f, 0.85f, 1.0f));
		HistoryLabel->SetText(NSLOCTEXT("VoiceKeywordTest", "HistoryLabel", "최근 기록"));
		Layout->AddChildToVerticalBox(HistoryLabel);

		HistoryText = MakeTestText(WidgetTree, TEXT("HistoryValue"), 18, FLinearColor(0.8f, 0.82f, 0.88f, 1.0f));
		if (UVerticalBoxSlot* HistorySlot = Layout->AddChildToVerticalBox(HistoryText))
		{
			HistorySlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		}
	}

	SetStatus(PendingStatus, PendingStatusColor);
	SetLastRecognized(PendingLastSentence);
	SetDetectedKeyword(PendingKeyword, bPendingHasKeyword);
	RefreshHistoryDisplay();
	return Super::RebuildWidget();
}

void UVoiceKeywordTestWidget::SetStatus(const FText Status, const FLinearColor StatusColor)
{
	PendingStatus = Status;
	PendingStatusColor = StatusColor;

	if (StatusText)
	{
		StatusText->SetText(Status);
		StatusText->SetColorAndOpacity(FSlateColor(StatusColor));
	}
}

void UVoiceKeywordTestWidget::SetLastRecognized(const FText Sentence)
{
	PendingLastSentence = Sentence;

	if (LastSentenceText)
	{
		LastSentenceText->SetText(Sentence.IsEmpty()
			? NSLOCTEXT("VoiceKeywordTest", "NoSentenceYet", "(아직 없음)")
			: Sentence);
	}
}

void UVoiceKeywordTestWidget::SetDetectedKeyword(const FText Keyword, const bool bHasKeyword)
{
	PendingKeyword = Keyword;
	bPendingHasKeyword = bHasKeyword;

	if (KeywordText)
	{
		KeywordText->SetText(bHasKeyword ? Keyword : NSLOCTEXT("VoiceKeywordTest", "NoKeyword", "-"));
		KeywordText->SetColorAndOpacity(FSlateColor(bHasKeyword ? DetectedKeywordColor : NeutralKeywordColor));
	}
}

void UVoiceKeywordTestWidget::AppendHistoryLine(const FText Line)
{
	HistoryLines.Insert(Line, 0);
	if (HistoryLines.Num() > MaxHistoryLines)
	{
		HistoryLines.RemoveAt(MaxHistoryLines, HistoryLines.Num() - MaxHistoryLines);
	}
	RefreshHistoryDisplay();
}

void UVoiceKeywordTestWidget::RefreshHistoryDisplay()
{
	if (!HistoryText)
	{
		return;
	}

	TArray<FString> Lines;
	Lines.Reserve(HistoryLines.Num());
	for (const FText& Line : HistoryLines)
	{
		Lines.Add(Line.ToString());
	}
	HistoryText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
}
