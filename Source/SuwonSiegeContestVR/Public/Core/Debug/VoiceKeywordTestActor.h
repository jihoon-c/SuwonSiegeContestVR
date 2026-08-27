#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Voice/VoiceRecognitionTypes.h"
#include "Templates/SubclassOf.h"
#include "VoiceKeywordTestActor.generated.h"

class USherpaVoiceRecognitionComponent;
class UVoiceKeywordTestWidget;

/**
 * Standalone microphone + keyword detection test tool. Owns a real sherpa-onnx recognizer,
 * listens continuously (re-arming after every utterance, since UVoiceRecognitionComponent ends a
 * request after each report), and mirrors every recognized sentence and matched keyword onto a
 * screen-space debug widget.
 *
 * Not tied to any quiz or scenario: drop this into a bare test level to check the mic + model
 * independent of Game Feature gameplay. `ssv.voice.submit <text>` also reaches this recognizer.
 */
UCLASS(Blueprintable)
class SUWONSIEGECONTESTVR_API AVoiceKeywordTestActor : public AActor
{
	GENERATED_BODY()

public:
	AVoiceKeywordTestActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleVoiceResult(FVoiceRecognitionResult Result);

	UFUNCTION()
	void HandleVoiceStateChanged(EVoiceRecognitionState OldState, EVoiceRecognitionState NewState);

	/** Tries to (re)start listening. Safe to call repeatedly; a no-op while already listening. */
	UFUNCTION()
	void TryStartListening();

	void RefreshStatusDisplay();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voice|Test")
	TObjectPtr<USherpaVoiceRecognitionComponent> VoiceRecognition;

	/** Words the tester expects to say. Reported keyword text matches these, not fuzzy synonyms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Test")
	TArray<FString> TestKeywords = { TEXT("옹성"), TEXT("신기전") };

	/** 0 listens until an utterance ends; there is no separate silence timeout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Test", meta = (ClampMin = "0.0", Units = "s"))
	float ListenDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Test|UI")
	TSubclassOf<UVoiceKeywordTestWidget> WidgetClass;

	/** How often to retry StartListening while the model is still loading. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice|Test", meta = (ClampMin = "0.1", Units = "s"))
	float ListenRetryInterval = 0.5f;

private:
	UPROPERTY(Transient)
	TObjectPtr<UVoiceKeywordTestWidget> TestWidget;

	FTimerHandle ListenRetryHandle;
	int32 UtteranceCount = 0;
};
