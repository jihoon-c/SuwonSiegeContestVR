#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Narration/NarrationTypes.h"
#include "NarrationSequenceComponent.generated.h"

struct FStreamableHandle;
class UAudioComponent;
class UDataTable;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNarrationSubtitleChanged, FText, SpeakerName, FText, Subtitle, bool, bVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrationRowEvent, FName, RowName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrationNamedEvent, FName, EventName, FName, SourceRow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrationWidgetRequested, TSubclassOf<UUserWidget>, WidgetClass, FName, SourceRow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNarrationSequenceFinished);

/**
 * Plays linked Data Table narration rows and exposes Blueprint-friendly flow hooks.
 * It owns flow only; subtitle and post-widget presentation remain the Pawn's job.
 */
UCLASS(ClassGroup = (Narration), meta = (BlueprintSpawnableComponent))
class SUWONSIEGECONTESTVR_API UNarrationSequenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNarrationSequenceComponent();

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void SetAudioComponent(UAudioComponent* InAudioComponent);

	UFUNCTION(BlueprintCallable, Category = "Narration")
	bool PlaySequence(UDataTable* InNarrationTable, FName StartRow);

	UFUNCTION(BlueprintCallable, Category = "Narration")
	bool PlayRow(FName RowName);

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void ContinueSequence();

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void SkipCurrentNarration();

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void StopSequence();

	UFUNCTION(BlueprintPure, Category = "Narration")
	bool IsNarrationPlaying() const { return bNarrationPlaying; }

	UFUNCTION(BlueprintPure, Category = "Narration")
	bool IsWaitingForContinue() const { return bWaitingForContinue; }

	UFUNCTION(BlueprintPure, Category = "Narration")
	FName GetCurrentRowName() const { return CurrentRowName; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narration")
	TObjectPtr<UDataTable> NarrationTable;

	UPROPERTY(BlueprintAssignable, Category = "Narration|Events")
	FOnNarrationSubtitleChanged OnSubtitleChanged;

	UPROPERTY(BlueprintAssignable, Category = "Narration|Events")
	FOnNarrationRowEvent OnNarrationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Narration|Events")
	FOnNarrationRowEvent OnNarrationFinished;

	UPROPERTY(BlueprintAssignable, Category = "Narration|Events")
	FOnNarrationNamedEvent OnSequenceEvent;

	UPROPERTY(BlueprintAssignable, Category = "Narration|Events")
	FOnNarrationWidgetRequested OnWidgetRequested;

	UPROPERTY(BlueprintAssignable, Category = "Narration|Events")
	FOnNarrationSequenceFinished OnSequenceFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BeginCurrentPlayback();
	void HandleNarrationSoundLoaded(FName RequestedRow);
	void FinishCurrentNarration();
	void ScheduleOrAdvance();
	void CompleteSequence();

	UFUNCTION()
	void HandleAudioFinished();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AudioComponent;

	FName CurrentRowName;
	FName PendingNextRow;
	FNarrationSequenceRow CurrentRow;
	FTimerHandle PreviewTimerHandle;
	FTimerHandle AdvanceTimerHandle;
	TSharedPtr<FStreamableHandle> ActiveSoundLoad;
	bool bNarrationPlaying = false;
	bool bWaitingForContinue = false;
	bool bSuppressAudioFinished = false;
};
