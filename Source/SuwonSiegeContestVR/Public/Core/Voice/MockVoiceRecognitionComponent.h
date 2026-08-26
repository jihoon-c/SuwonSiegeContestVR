#pragma once

#include "CoreMinimal.h"
#include "Core/Voice/VoiceRecognitionComponent.h"
#include "MockVoiceRecognitionComponent.generated.h"

UENUM(BlueprintType)
enum class EMockVoiceRecognitionMode : uint8
{
	/** Nothing is recognized until SubmitMockSpeech or the console command is used. */
	ManualOnly,
	/** Answers with the first request keyword after AutoResponseDelay. Keeps a flow test moving. */
	AutoCorrect,
	/** Answers with MisrecognizedText after AutoResponseDelay, exercising the retry path. */
	AutoNoMatch
};

/**
 * Stand-in recognizer used until an on-device backend is imported. It owns no audio at all;
 * it just produces text on the same ingress a real backend would use.
 *
 * Replace it by deriving another UVoiceRecognitionComponent and swapping the component class.
 * See docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md.
 */
UCLASS(ClassGroup = (Voice), meta = (BlueprintSpawnableComponent, DisplayName = "Mock Voice Recognition"))
class SUWONSIEGECONTESTVR_API UMockVoiceRecognitionComponent : public UVoiceRecognitionComponent
{
	GENERATED_BODY()

public:
	UMockVoiceRecognitionComponent();

	/** Manual ingress for a debug button, keyboard test input, or an editor utility. */
	UFUNCTION(BlueprintCallable, Category = "Voice|Mock")
	bool SubmitMockSpeech(const FString& SpokenText);

	/**
	 * ManualOnly is the honest default: with no microphone behind it, the mock recognizes nothing
	 * and the quiz falls through on its own timeouts. Use ssv.voice.submit to answer during a test,
	 * or switch to AutoCorrect for an unattended flow run.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Mock")
	EMockVoiceRecognitionMode MockMode = EMockVoiceRecognitionMode::ManualOnly;

	/** Rough stand-in for how long a player takes to answer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Mock", meta = (ClampMin = "0.0", Units = "s"))
	float AutoResponseDelay = 4.0f;

	/** Text reported in AutoNoMatch mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Mock")
	FString MisrecognizedText = TEXT("모르겠어요");

	/** Registers ssv.voice.submit while this component exists. Editor and development builds only. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Mock")
	bool bEnableConsoleCommand = true;

protected:
	virtual bool BeginBackendListening_Implementation(const FVoiceRecognitionRequest& Request) override;
	virtual void EndBackendListening_Implementation() override;

private:
	UFUNCTION()
	void HandleAutoResponse();

	FTimerHandle AutoResponseHandle;
};
