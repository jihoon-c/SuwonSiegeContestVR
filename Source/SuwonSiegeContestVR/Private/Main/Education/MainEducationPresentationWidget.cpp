#include "Main/Education/MainEducationPresentationWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Main/Education/MainEducationScenarioManagerActor.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UMainEducationPresentationWidget::RebuildWidget()
{
	ContinueButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("PresentationImageButton"));
	ContinueButton->SetBackgroundColor(FLinearColor(0.025f, 0.035f, 0.05f, 0.96f));
	ContinueButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleContinueClicked);
	WidgetTree->RootWidget = ContinueButton;

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PresentationLayout"));
	ContinueButton->AddChild(Layout);

	ContentImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("EducationImage"));
	ContentImage->SetDesiredSizeOverride(FVector2D(1000.0f, 520.0f));
	if (UVerticalBoxSlot* ImageSlot = Layout->AddChildToVerticalBox(ContentImage))
	{
		ImageSlot->SetPadding(FMargin(24.0f, 24.0f, 24.0f, 12.0f));
		ImageSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EducationTitle"));
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 42));
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.86f, 0.42f)));
	TitleText->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* TitleSlot = Layout->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetPadding(FMargin(30.0f, 4.0f, 30.0f, 8.0f));
	}

	BodyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EducationBody"));
	BodyText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 27));
	BodyText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	BodyText->SetAutoWrapText(true);
	BodyText->SetWrapTextAt(980.0f);
	BodyText->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* BodySlot = Layout->AddChildToVerticalBox(BodyText))
	{
		BodySlot->SetPadding(FMargin(36.0f, 4.0f, 36.0f, 16.0f));
	}

	ContinueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ContinueHint"));
	ContinueText->SetText(FText::FromString(TEXT("이미지를 클릭하면 다음 설명으로 이동합니다")));
	ContinueText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 22));
	ContinueText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.84f, 1.0f)));
	ContinueText->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* HintSlot = Layout->AddChildToVerticalBox(ContinueText))
	{
		HintSlot->SetPadding(FMargin(20.0f, 2.0f, 20.0f, 24.0f));
	}

	ApplyContent();
	return Super::RebuildWidget();
}

void UMainEducationPresentationWidget::Configure(
	AMainEducationScenarioManagerActor* InManager,
	const FMainEducationContent& InContent)
{
	Manager = InManager;
	PendingContent = InContent;
	ApplyContent();
}

void UMainEducationPresentationWidget::ApplyContent()
{
	if (TitleText)
	{
		TitleText->SetText(PendingContent.Title);
	}
	if (BodyText)
	{
		BodyText->SetText(PendingContent.Body);
	}
	if (ContentImage)
	{
		if (UTexture2D* Texture = PendingContent.Image.LoadSynchronous())
		{
			ContentImage->SetBrushFromTexture(Texture, true);
			ContentImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ContentImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMainEducationPresentationWidget::HandleContinueClicked()
{
	if (Manager)
	{
		Manager->ContinuePresentation();
	}
}
