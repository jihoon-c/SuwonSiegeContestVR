#include "Ongseong/ChongtongChargeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UChongtongChargeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ChargeBar)
	{
		return;
	}

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
	Background->SetBrushColor(FLinearColor(0.015f, 0.015f, 0.015f, 0.82f));
	WidgetTree->RootWidget = Background;

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Layout"));
	Background->SetContent(Layout);
	ChargeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ChargeText"));
	ChargeText->SetJustification(ETextJustify::Center);
	ChargeText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Layout->AddChildToVerticalBox(ChargeText);
	ChargeBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ChargeBar"));
	ChargeBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.34f, 0.04f));
	Layout->AddChildToVerticalBox(ChargeBar);
	SetChargePercent(0.0f);
}

void UChongtongChargeWidget::SetChargePercent(const float Percent)
{
	const float Clamped = FMath::Clamp(Percent, 0.0f, 1.0f);
	if (ChargeBar)
	{
		ChargeBar->SetPercent(Clamped);
	}
	if (ChargeText)
	{
		ChargeText->SetText(FText::Format(NSLOCTEXT("Chongtong", "PowerPercent", "POWER {0}%"),
			FText::AsNumber(FMath::RoundToInt(Clamped * 100.0f))));
	}
}
