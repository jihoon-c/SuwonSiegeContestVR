#pragma once

#include "CoreMinimal.h"
#include "VRHUDTypes.generated.h"

UENUM(BlueprintType)
enum class EVRHUDNotificationType : uint8
{
	Info,
	Success,
	Warning,
	Error
};

/** Feature-neutral presentation state consumed by the VR HUD. */
USTRUCT(BlueprintType)
struct SUWONSIEGECONTESTVR_API FVRHUDState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	FText Objective;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	FText ObjectiveDetail;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	FText ProgressLabel;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	int32 ProgressCurrent = 0;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	int32 ProgressTotal = 0;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	FText Prompt;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	FText Notification;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	EVRHUDNotificationType NotificationType = EVRHUDNotificationType::Info;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	bool bProgressVisible = false;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	bool bPromptVisible = false;

	UPROPERTY(BlueprintReadOnly, Category="VR|UI")
	bool bNotificationVisible = false;

	float GetProgressFraction() const
	{
		return ProgressTotal > 0
			? FMath::Clamp(static_cast<float>(ProgressCurrent) / static_cast<float>(ProgressTotal), 0.0f, 1.0f)
			: 0.0f;
	}

	bool HasVisibleContent() const
	{
		return !Objective.IsEmpty() || bProgressVisible || bPromptVisible || bNotificationVisible;
	}
};
