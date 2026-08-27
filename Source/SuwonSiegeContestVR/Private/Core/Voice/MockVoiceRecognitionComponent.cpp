#include "Core/Voice/MockVoiceRecognitionComponent.h"

#include "Engine/World.h"
#include "TimerManager.h"


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
