#pragma once

#include "Blueprint/UserWidget.h"
#include "Main/Education/MainEducationTypes.h"
#include "MainEducationPresentationWidget.generated.h"

class AMainEducationScenarioManagerActor;
class UButton;
class UImage;
class UTextBlock;

/** Runtime VR panel: the presentation image itself is the continue button. */
UCLASS()
class SUWONSIEGECONTESTVR_API UMainEducationPresentationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	void Configure(AMainEducationScenarioManagerActor* InManager, const FMainEducationContent& InContent);

private:
	UFUNCTION()
	void HandleContinueClicked();

	void ApplyContent();

	UPROPERTY(Transient)
	TObjectPtr<AMainEducationScenarioManagerActor> Manager;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(Transient)
	TObjectPtr<UImage> ContentImage;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContinueText;

	FMainEducationContent PendingContent;
};
