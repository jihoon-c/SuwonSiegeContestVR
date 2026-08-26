#include "Core/Quiz/InitialConsonantQuizWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
	UTextBlock* MakeQuizText(UWidgetTree* Tree, const FName Name, const int32 FontSize,
		const FLinearColor& Color, const bool bBold = false)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Text->SetJustification(ETextJustify::Center);
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

TSharedRef<SWidget> UInitialConsonantQuizWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("QuizBackground"));
		Background->SetBrushColor(FLinearColor(0.015f, 0.025f, 0.045f, 0.92f));
		Background->SetPadding(FMargin(56.0f, 40.0f));
		Background->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetTree->RootWidget = Background;

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("QuizLayout"));
		Background->SetContent(Layout);

		TitleText = MakeQuizText(WidgetTree, TEXT("QuizTitle"), 44, FLinearColor(0.15f, 0.75f, 1.0f, 1.0f), true);
		Layout->AddChildToVerticalBox(TitleText);

		QuestionText = MakeQuizText(WidgetTree, TEXT("QuizQuestion"), 34, FLinearColor::White);
		if (UVerticalBoxSlot* QuestionSlot = Layout->AddChildToVerticalBox(QuestionText))
		{
			QuestionSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 0.0f));
		}

		// The consonants are the point of the panel, so they dominate the layout.
		ConsonantText = MakeQuizText(WidgetTree, TEXT("QuizConsonants"), 150, FLinearColor(1.0f, 0.93f, 0.62f, 1.0f), true);
		if (UVerticalBoxSlot* ConsonantSlot = Layout->AddChildToVerticalBox(ConsonantText))
		{
			ConsonantSlot->SetPadding(FMargin(0.0f, 26.0f, 0.0f, 26.0f));
		}

		StatusText = MakeQuizText(WidgetTree, TEXT("QuizStatus"), 36, PendingStatusColor, true);
		Layout->AddChildToVerticalBox(StatusText);

		FooterText = MakeQuizText(WidgetTree, TEXT("QuizFooter"), 26, FLinearColor(0.75f, 0.78f, 0.85f, 1.0f));
		if (UVerticalBoxSlot* FooterSlot = Layout->AddChildToVerticalBox(FooterText))
		{
			FooterSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 0.0f));
		}
	}

	SetQuizPrompt(PendingTitle, PendingQuestion, PendingConsonants);
	SetStatus(PendingStatus, PendingStatusColor);
	SetFooter(PendingFooter);
	return Super::RebuildWidget();
}

void UInitialConsonantQuizWidget::SetQuizPrompt(const FText Title, const FText Question, const FText Consonants)
{
	PendingTitle = Title;
	PendingQuestion = Question;
	PendingConsonants = Consonants;

	if (TitleText)
	{
		TitleText->SetText(Title);
	}
	if (QuestionText)
	{
		QuestionText->SetText(Question);
		QuestionText->SetVisibility(Question.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (ConsonantText)
	{
		ConsonantText->SetText(Consonants);
	}

	BP_OnQuizPromptSet(Title, Question, Consonants);
}

void UInitialConsonantQuizWidget::SetStatus(const FText Status, const FLinearColor StatusColor)
{
	PendingStatus = Status;
	PendingStatusColor = StatusColor;

	if (StatusText)
	{
		StatusText->SetText(Status);
		StatusText->SetColorAndOpacity(FSlateColor(StatusColor));
		StatusText->SetVisibility(Status.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	BP_OnQuizStatusSet(Status, StatusColor);
}

void UInitialConsonantQuizWidget::SetFooter(const FText Footer)
{
	PendingFooter = Footer;

	if (FooterText)
	{
		FooterText->SetText(Footer);
		FooterText->SetVisibility(Footer.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}
