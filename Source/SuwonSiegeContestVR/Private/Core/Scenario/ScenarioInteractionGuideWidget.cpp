#include "Core/Scenario/ScenarioInteractionGuideWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

TSharedRef<SWidget> UScenarioInteractionGuideWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("GuideBackground"));
		Background->SetBrushColor(FLinearColor(0.015f, 0.025f, 0.045f, 0.88f));
		Background->SetPadding(FMargin(24.0f, 14.0f));
		Background->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetTree->RootWidget = Background;

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GuideLayout"));
		Background->SetContent(Layout);

		ActionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GuideAction"));
		ActionText->SetJustification(ETextJustify::Center);
		ActionText->SetShadowOffset(FVector2D(1.0f, 1.0f));
		FSlateFontInfo ActionFont = ActionText->GetFont();
		ActionFont.Size = 22;
		ActionFont.TypefaceFontName = TEXT("Bold");
		ActionText->SetFont(ActionFont);
		Layout->AddChildToVerticalBox(ActionText);

		InstructionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GuideInstruction"));
		InstructionText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		InstructionText->SetJustification(ETextJustify::Center);
		InstructionText->SetAutoWrapText(true);
		InstructionText->SetShadowOffset(FVector2D(2.0f, 2.0f));
		InstructionText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
		FSlateFontInfo InstructionFont = InstructionText->GetFont();
		InstructionFont.Size = 30;
		InstructionText->SetFont(InstructionFont);
		if (UVerticalBoxSlot* InstructionSlot = Layout->AddChildToVerticalBox(InstructionText))
		{
			InstructionSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
		}
	}

	SetGuide(PendingActionLabel, PendingInstruction, PendingAccentColor);
	return Super::RebuildWidget();
}

void UScenarioInteractionGuideWidget::SetGuide(
	const FText ActionLabel, const FText Instruction, const FLinearColor AccentColor)
{
	PendingActionLabel = ActionLabel;
	PendingInstruction = Instruction;
	PendingAccentColor = AccentColor;
	if (ActionText)
	{
		ActionText->SetText(ActionLabel);
		ActionText->SetColorAndOpacity(FSlateColor(AccentColor));
	}
	if (InstructionText)
	{
		InstructionText->SetText(Instruction);
	}
}
