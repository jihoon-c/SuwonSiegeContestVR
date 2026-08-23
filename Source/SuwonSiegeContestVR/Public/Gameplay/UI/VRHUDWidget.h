#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/UI/VRHUDTypes.h"
#include "VRHUDWidget.generated.h"

class UBorder;
class UProgressBar;
class UTextBlock;

/** Native fallback VR HUD. A Blueprint subclass can replace visuals and consume OnHUDStateApplied. */
UCLASS(BlueprintType)
class SUWONSIEGECONTESTVR_API UVRHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="VR|UI")
	void ApplyHUDState(FVRHUDState State);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintImplementableEvent, Category="VR|UI", meta=(DisplayName="On HUD State Applied"))
	void BP_OnHUDStateApplied(const FVRHUDState& State);

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ObjectiveText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DetailText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ProgressText;
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> ProgressBar;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PromptText;
	UPROPERTY(Transient)
	TObjectPtr<UBorder> NotificationBorder;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NotificationText;

private:
	FVRHUDState CachedState;
};
