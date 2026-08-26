#include "Core/Voice/MockVoiceRecognitionComponent.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"

#if !UE_BUILD_SHIPPING
namespace
{
	/** Debug entry point for PC testing: `ssv.voice.submit 옹성`. */
	void SubmitMockSpeechCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (!World)
		{
			return;
		}

		const FString SpokenText = FString::Join(Args, TEXT(" ")).TrimStartAndEnd();
		if (SpokenText.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("ssv.voice.submit needs the spoken text, e.g. ssv.voice.submit 옹성"));
			return;
		}

		int32 Delivered = 0;
		for (TObjectIterator<UMockVoiceRecognitionComponent> It; It; ++It)
		{
			UMockVoiceRecognitionComponent* Mock = *It;
			if (!IsValid(Mock) || Mock->GetWorld() != World || !Mock->bEnableConsoleCommand || !Mock->IsListening())
			{
				continue;
			}
			Mock->SubmitMockSpeech(SpokenText);
			++Delivered;
		}

		if (Delivered == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("ssv.voice.submit found no recognizer that is currently listening."));
		}
	}

	FAutoConsoleCommandWithWorldAndArgs GSubmitMockSpeechCommand(
		TEXT("ssv.voice.submit"),
		TEXT("Feeds text to the listening mock voice recognizer, e.g. ssv.voice.submit 옹성"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SubmitMockSpeechCommand));
}
#endif

UMockVoiceRecognitionComponent::UMockVoiceRecognitionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UMockVoiceRecognitionComponent::SubmitMockSpeech(const FString& SpokenText)
{
	return ReportRecognizedText(SpokenText, 1.0f);
}

bool UMockVoiceRecognitionComponent::BeginBackendListening_Implementation(const FVoiceRecognitionRequest& Request)
{
	if (MockMode == EMockVoiceRecognitionMode::ManualOnly || AutoResponseDelay < 0.0f)
	{
		return true;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoResponseHandle, this,
			&UMockVoiceRecognitionComponent::HandleAutoResponse, FMath::Max(AutoResponseDelay, 0.01f), false);
	}

	return true;
}

void UMockVoiceRecognitionComponent::EndBackendListening_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoResponseHandle);
	}
}

void UMockVoiceRecognitionComponent::HandleAutoResponse()
{
	const FVoiceRecognitionRequest Request = GetActiveRequest();
	const FString SpokenText = MockMode == EMockVoiceRecognitionMode::AutoCorrect && Request.Keywords.Num() > 0
		? Request.Keywords[0]
		: MisrecognizedText;

	SubmitMockSpeech(SpokenText);
}
