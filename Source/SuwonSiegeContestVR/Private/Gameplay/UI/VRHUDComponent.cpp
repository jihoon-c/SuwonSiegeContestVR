#include "Gameplay/UI/VRHUDComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"

UVRHUDComponent::UVRHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRHUDComponent::SetObjective(const FText Objective, const FText Detail)
{
	HUDState.Objective = Objective;
	HUDState.ObjectiveDetail = Detail;
	BroadcastState();
}

void UVRHUDComponent::ClearObjective()
{
	HUDState.Objective = FText::GetEmpty();
	HUDState.ObjectiveDetail = FText::GetEmpty();
	BroadcastState();
}

void UVRHUDComponent::SetProgress(const FText Label, const int32 Current, const int32 Total)
{
	HUDState.ProgressLabel = Label;
	HUDState.ProgressTotal = FMath::Max(0, Total);
	HUDState.ProgressCurrent = FMath::Clamp(Current, 0, HUDState.ProgressTotal);
	HUDState.bProgressVisible = HUDState.ProgressTotal > 0;
	BroadcastState();
}

void UVRHUDComponent::ClearProgress()
{
	HUDState.ProgressLabel = FText::GetEmpty();
	HUDState.ProgressCurrent = 0;
	HUDState.ProgressTotal = 0;
	HUDState.bProgressVisible = false;
	BroadcastState();
}

void UVRHUDComponent::ShowPrompt(const FText Prompt)
{
	HUDState.Prompt = Prompt;
	HUDState.bPromptVisible = !Prompt.IsEmpty();
	BroadcastState();
}

void UVRHUDComponent::ClearPrompt()
{
	HUDState.Prompt = FText::GetEmpty();
	HUDState.bPromptVisible = false;
	BroadcastState();
}

void UVRHUDComponent::ShowNotification(const FText Message, const EVRHUDNotificationType Type, const float Duration)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimer);
	}
	HUDState.Notification = Message;
	HUDState.NotificationType = Type;
	HUDState.bNotificationVisible = !Message.IsEmpty();
	BroadcastState();

	if (HUDState.bNotificationVisible && Duration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(NotificationTimer, this, &ThisClass::ClearNotification, Duration, false);
		}
	}
}

void UVRHUDComponent::ClearNotification()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimer);
	}
	HUDState.Notification = FText::GetEmpty();
	HUDState.bNotificationVisible = false;
	BroadcastState();
}

void UVRHUDComponent::ClearAll()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimer);
	}
	HUDState = FVRHUDState();
	BroadcastState();
}

void UVRHUDComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void UVRHUDComponent::BroadcastState()
{
	OnHUDStateChanged.Broadcast(HUDState);
}
