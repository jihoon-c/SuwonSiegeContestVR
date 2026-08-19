#include "Core/Narration/NarrationSequenceComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UNarrationSequenceComponent::UNarrationSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNarrationSequenceComponent::SetAudioComponent(UAudioComponent* InAudioComponent)
{
	if (AudioComponent)
	{
		AudioComponent->OnAudioFinished.RemoveDynamic(this, &UNarrationSequenceComponent::HandleAudioFinished);
	}

	AudioComponent = InAudioComponent;
	if (AudioComponent)
	{
		AudioComponent->bAutoActivate = false;
		AudioComponent->OnAudioFinished.AddUniqueDynamic(this, &UNarrationSequenceComponent::HandleAudioFinished);
	}
}

bool UNarrationSequenceComponent::PlaySequence(UDataTable* InNarrationTable, const FName StartRow)
{
	StopSequence();
	NarrationTable = InNarrationTable;
	return PlayRow(StartRow);
}

bool UNarrationSequenceComponent::PlayRow(const FName RowName)
{
	if (!NarrationTable || RowName.IsNone())
	{
		return false;
	}

	const FNarrationSequenceRow* Row = NarrationTable->FindRow<FNarrationSequenceRow>(RowName, TEXT("NarrationSequence"));
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("Narration row '%s' was not found in '%s'."), *RowName.ToString(), *GetNameSafe(NarrationTable));
		return false;
	}

	GetWorld()->GetTimerManager().ClearTimer(PreviewTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(AdvanceTimerHandle);
	if (ActiveSoundLoad.IsValid())
	{
		ActiveSoundLoad->CancelHandle();
		ActiveSoundLoad.Reset();
	}

	bSuppressAudioFinished = true;
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
	bSuppressAudioFinished = false;

	OnSubtitleChanged.Broadcast(FText::GetEmpty(), FText::GetEmpty(), false);
	CurrentRowName = RowName;
	CurrentRow = *Row;
	PendingNextRow = NAME_None;
	bNarrationPlaying = false;
	bWaitingForContinue = false;

	if (!CurrentRow.NarrationSound.IsNull())
	{
		const FSoftObjectPath SoundPath = CurrentRow.NarrationSound.ToSoftObjectPath();
		ActiveSoundLoad = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			SoundPath,
			FStreamableDelegate::CreateUObject(this, &UNarrationSequenceComponent::HandleNarrationSoundLoaded, RowName));
		return true;
	}

	BeginCurrentPlayback();
	return true;
}

void UNarrationSequenceComponent::HandleNarrationSoundLoaded(const FName RequestedRow)
{
	ActiveSoundLoad.Reset();
	if (RequestedRow != CurrentRowName)
	{
		return;
	}

	BeginCurrentPlayback();
}

void UNarrationSequenceComponent::BeginCurrentPlayback()
{
	bNarrationPlaying = true;
	OnNarrationStarted.Broadcast(CurrentRowName);
	OnSubtitleChanged.Broadcast(CurrentRow.SpeakerName, CurrentRow.Subtitle, true);

	USoundBase* Sound = CurrentRow.NarrationSound.Get();
	if (AudioComponent && Sound)
	{
		AudioComponent->SetSound(Sound);
		AudioComponent->Play();
		return;
	}

	if (CurrentRow.PreviewDuration <= 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UNarrationSequenceComponent::FinishCurrentNarration);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		PreviewTimerHandle,
		this,
		&UNarrationSequenceComponent::FinishCurrentNarration,
		CurrentRow.PreviewDuration,
		false);
}

void UNarrationSequenceComponent::HandleAudioFinished()
{
	if (!bSuppressAudioFinished && bNarrationPlaying)
	{
		FinishCurrentNarration();
	}
}

void UNarrationSequenceComponent::FinishCurrentNarration()
{
	if (!bNarrationPlaying)
	{
		return;
	}

	bNarrationPlaying = false;
	GetWorld()->GetTimerManager().ClearTimer(PreviewTimerHandle);
	OnSubtitleChanged.Broadcast(FText::GetEmpty(), FText::GetEmpty(), false);
	OnNarrationFinished.Broadcast(CurrentRowName);

	for (const FName EventName : CurrentRow.CompletionEvents)
	{
		if (!EventName.IsNone())
		{
			OnSequenceEvent.Broadcast(EventName, CurrentRowName);
		}
	}

	if (!CurrentRow.PostNarrationWidgetClass.IsNull())
	{
		if (UClass* WidgetClass = CurrentRow.PostNarrationWidgetClass.LoadSynchronous())
		{
			OnWidgetRequested.Broadcast(WidgetClass, CurrentRowName);
		}
	}

	PendingNextRow = CurrentRow.NextRow;
	ScheduleOrAdvance();
}

void UNarrationSequenceComponent::ScheduleOrAdvance()
{
	switch (CurrentRow.AdvanceMode)
	{
	case ENarrationAdvanceMode::Auto:
		if (PendingNextRow.IsNone())
		{
			CompleteSequence();
		}
		else if (CurrentRow.AdvanceDelay <= 0.0f)
		{
			PlayRow(PendingNextRow);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(
				AdvanceTimerHandle,
				this,
				&UNarrationSequenceComponent::ContinueSequence,
				CurrentRow.AdvanceDelay,
				false);
		}
		break;

	case ENarrationAdvanceMode::WaitForContinue:
		bWaitingForContinue = true;
		break;

	case ENarrationAdvanceMode::Stop:
	default:
		CompleteSequence();
		break;
	}
}

void UNarrationSequenceComponent::ContinueSequence()
{
	GetWorld()->GetTimerManager().ClearTimer(AdvanceTimerHandle);
	bWaitingForContinue = false;
	if (PendingNextRow.IsNone())
	{
		CompleteSequence();
		return;
	}

	const FName NextRow = PendingNextRow;
	PendingNextRow = NAME_None;
	PlayRow(NextRow);
}

void UNarrationSequenceComponent::SkipCurrentNarration()
{
	if (!bNarrationPlaying)
	{
		return;
	}

	bSuppressAudioFinished = true;
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
	bSuppressAudioFinished = false;
	FinishCurrentNarration();
}

void UNarrationSequenceComponent::StopSequence()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PreviewTimerHandle);
		World->GetTimerManager().ClearTimer(AdvanceTimerHandle);
	}

	if (ActiveSoundLoad.IsValid())
	{
		ActiveSoundLoad->CancelHandle();
		ActiveSoundLoad.Reset();
	}

	bSuppressAudioFinished = true;
	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
	bSuppressAudioFinished = false;

	bNarrationPlaying = false;
	bWaitingForContinue = false;
	CurrentRowName = NAME_None;
	PendingNextRow = NAME_None;
	OnSubtitleChanged.Broadcast(FText::GetEmpty(), FText::GetEmpty(), false);
}

void UNarrationSequenceComponent::CompleteSequence()
{
	bWaitingForContinue = false;
	PendingNextRow = NAME_None;
	OnSequenceFinished.Broadcast();
}

void UNarrationSequenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSequence();
	SetAudioComponent(nullptr);
	Super::EndPlay(EndPlayReason);
}
