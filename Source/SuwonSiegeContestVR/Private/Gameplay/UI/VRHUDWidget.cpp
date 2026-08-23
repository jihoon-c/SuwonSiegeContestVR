#include "Gameplay/UI/VRHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
	void ConfigureText(UTextBlock* Text, const int32 Size, const FLinearColor Color)
	{
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetJustification(ETextJustify::Center);
		Text->SetAutoWrapText(true);
		Text->SetShadowOffset(FVector2D(1.5f, 1.5f));
		Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = Size;
		Text->SetFont(Font);
	}

	FLinearColor NotificationColor(const EVRHUDNotificationType Type)
	{
		switch (Type)
		{
		case EVRHUDNotificationType::Success: return FLinearColor(0.04f, 0.42f, 0.16f, 0.92f);
		case EVRHUDNotificationType::Warning: return FLinearColor(0.62f, 0.28f, 0.02f, 0.92f);
		case EVRHUDNotificationType::Error: return FLinearColor(0.58f, 0.04f, 0.04f, 0.92f);
		default: return FLinearColor(0.03f, 0.16f, 0.32f, 0.92f);
		}
	}
}

TSharedRef<SWidget> UVRHUDWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HUDBackground"));
		Background->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.025f, 0.78f));
		Background->SetPadding(FMargin(24.0f, 16.0f));
		Background->SetVisibility(ESlateVisibility::HitTestInvisible);
		WidgetTree->RootWidget = Background;

		UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HUDLayout"));
		Background->SetContent(Layout);

		ObjectiveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ObjectiveText"));
		ConfigureText(ObjectiveText, 30, FLinearColor(1.0f, 0.75f, 0.2f));
		Layout->AddChildToVerticalBox(ObjectiveText);

		DetailText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DetailText"));
		ConfigureText(DetailText, 22, FLinearColor(0.9f, 0.92f, 0.95f));
		Layout->AddChildToVerticalBox(DetailText);

		ProgressText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressText"));
		ConfigureText(ProgressText, 22, FLinearColor::White);
		if (UVerticalBoxSlot* ProgressSlot = Layout->AddChildToVerticalBox(ProgressText))
		{
			ProgressSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 3.0f));
		}

		ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar"));
		ProgressBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.7f, 1.0f));
		Layout->AddChildToVerticalBox(ProgressBar);

		PromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PromptText"));
		ConfigureText(PromptText, 25, FLinearColor(0.55f, 0.9f, 1.0f));
		if (UVerticalBoxSlot* PromptSlot = Layout->AddChildToVerticalBox(PromptText))
		{
			PromptSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));
		}

		NotificationBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NotificationBorder"));
		NotificationBorder->SetPadding(FMargin(12.0f, 7.0f));
		if (UVerticalBoxSlot* NotificationSlot = Layout->AddChildToVerticalBox(NotificationBorder))
		{
			NotificationSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));
		}
		NotificationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NotificationText"));
		ConfigureText(NotificationText, 25, FLinearColor::White);
		NotificationBorder->SetContent(NotificationText);
	}

	const TSharedRef<SWidget> Widget = Super::RebuildWidget();
	ApplyHUDState(CachedState);
	return Widget;
}

void UVRHUDWidget::ApplyHUDState(const FVRHUDState State)
{
	CachedState = State;
	if (ObjectiveText)
	{
		ObjectiveText->SetText(State.Objective);
		ObjectiveText->SetVisibility(State.Objective.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (DetailText)
	{
		DetailText->SetText(State.ObjectiveDetail);
		DetailText->SetVisibility(State.ObjectiveDetail.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (ProgressText)
	{
		const FText Value = FText::Format(NSLOCTEXT("VRHUD", "ProgressFormat", "{0}  {1}/{2}"),
			State.ProgressLabel, FText::AsNumber(State.ProgressCurrent), FText::AsNumber(State.ProgressTotal));
		ProgressText->SetText(Value);
		ProgressText->SetVisibility(State.bProgressVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (ProgressBar)
	{
		ProgressBar->SetPercent(State.GetProgressFraction());
		ProgressBar->SetVisibility(State.bProgressVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (PromptText)
	{
		PromptText->SetText(State.Prompt);
		PromptText->SetVisibility(State.bPromptVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (NotificationBorder && NotificationText)
	{
		NotificationBorder->SetBrushColor(NotificationColor(State.NotificationType));
		NotificationText->SetText(State.Notification);
		NotificationBorder->SetVisibility(State.bNotificationVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	BP_OnHUDStateApplied(State);
}
