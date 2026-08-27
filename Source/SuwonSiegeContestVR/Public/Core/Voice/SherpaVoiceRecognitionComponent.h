#pragma once

#include "CoreMinimal.h"
#include "Core/Voice/VoiceRecognitionComponent.h"
#include "SherpaVoiceRecognitionComponent.generated.h"

struct FSherpaRecognizerHandles;

UENUM(BlueprintType)
enum class ESherpaRecognizerStatus : uint8
{
	NotInitialized,
	Initializing,
	Ready,
	/** The model files are missing. Run Scripts/DownloadKoreanVoiceModel.py. */
	ModelMissing,
	/** The native library or the recognizer refused to load. See GetInitializationError. */
	Failed
};

/**
 * On-device Korean speech recognition backed by sherpa-onnx (Apache-2.0).
 *
 * Runs the same way on Windows (editor, PC build, headset over Link) and on the Android
 * standalone build: UE's audio capture feeds a streaming Zipformer transducer, and whatever it
 * hears is funnelled into the shared UVoiceRecognitionComponent::ReportRecognizedText ingress.
 *
 * Model files are provisioned by Scripts/DownloadKoreanVoiceModel.py; see
 * docs/Core/specs/SHERPA_ONNX_INTEGRATION.md.
 */
UCLASS(ClassGroup = (Voice), meta = (BlueprintSpawnableComponent, DisplayName = "Sherpa Voice Recognition"))
class SUWONSIEGECONTESTVR_API USherpaVoiceRecognitionComponent : public UVoiceRecognitionComponent
{
	GENERATED_BODY()

public:
	USherpaVoiceRecognitionComponent();
	/** Defined in the .cpp so the native handle type can stay out of this header. */
	virtual ~USherpaVoiceRecognitionComponent() override;

	/** Loads the model on a background thread. Safe to call more than once. */
	UFUNCTION(BlueprintCallable, Category = "Voice|Sherpa")
	bool BeginInitialization();

	UFUNCTION(BlueprintPure, Category = "Voice|Sherpa")
	ESherpaRecognizerStatus GetRecognizerStatus() const { return RecognizerStatus; }

	UFUNCTION(BlueprintPure, Category = "Voice|Sherpa")
	bool IsRecognizerReady() const { return RecognizerStatus == ESherpaRecognizerStatus::Ready; }

	/** Empty unless initialization failed. Shown in logs and by ssv.voice.status. */
	UFUNCTION(BlueprintPure, Category = "Voice|Sherpa")
	FString GetInitializationError() const { return InitializationError; }

	/** Model files this component cannot start without, relative to the model directory. */
	UFUNCTION(BlueprintPure, Category = "Voice|Sherpa")
	TArray<FString> GetRequiredModelFiles() const;

	/**
	 * Model files that improve recognition but are not fatal when missing, relative to the model
	 * directory. Today that is the BPE vocabulary hotwords need.
	 */
	UFUNCTION(BlueprintPure, Category = "Voice|Sherpa")
	TArray<FString> GetOptionalModelFiles() const;

	/**
	 * Decodes a 16-bit PCM wave file through the same recognizer. Used by the automation test and
	 * by ssv.voice.decodewav so the backend can be verified without a microphone.
	 * Blocking; not for gameplay use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice|Sherpa|Debug")
	bool DecodeWaveFile(const FString& WaveFilePath, FString& OutRecognizedText);

	/** True when the sherpa-onnx native library could be loaded on this platform. */
	static bool IsNativeLibraryAvailable();

	virtual FString GetBackendDescription() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Model")
	FString ModelName = TEXT("sherpa-onnx-streaming-zipformer-korean-2024-06-16");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Model")
	FString EncoderFile = TEXT("encoder-epoch-99-avg-1.int8.onnx");

	/** The decoder stays float: quantizing it costs accuracy and saves only a few megabytes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Model")
	FString DecoderFile = TEXT("decoder-epoch-99-avg-1.onnx");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Model")
	FString JoinerFile = TEXT("joiner-epoch-99-avg-1.int8.onnx");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Model")
	FString TokensFile = TEXT("tokens.txt");

	/** Two threads keeps headroom on a headset CPU that is also running the experience. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Decoding", meta = (ClampMin = "1", ClampMax = "4"))
	int32 NumThreads = 2;

	/**
	 * "greedy_search" or "modified_beam_search". Hotwords require modified_beam_search, so this is
	 * forced to it while they are active and falls back to greedy_search when they cannot be used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Decoding")
	FString DecodingMethod = TEXT("modified_beam_search");

	/**
	 * Biases decoding towards the expected answers. Greedy decoding drops multi-syllable answers
	 * such as "신기전" far more often than two-syllable ones, and keyword matching compares exact
	 * strings, so a single wrong syllable loses the answer.
	 *
	 * Needs BpeVocabFile beside the model. Scripts/DownloadKoreanVoiceModel.py derives it from
	 * bpe.model; when it is absent the component logs a warning and decodes without hotwords.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Decoding")
	bool bUseHotwords = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Decoding",
		meta = (EditCondition = "bUseHotwords"))
	FString BpeVocabFile = TEXT("bpe.vocab");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Decoding",
		meta = (EditCondition = "bUseHotwords", ClampMin = "0.0"))
	float HotwordsScore = 2.0f;

	/**
	 * Players answer with one word inside a sentence ("옹성이요"), so a keyword found anywhere in
	 * the utterance is reported as that keyword. The quiz itself still compares exactly.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Decoding")
	bool bMatchKeywordInsideUtterance = true;

	/** Trailing silence that ends an utterance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Endpoint", meta = (ClampMin = "0.1", Units = "s"))
	float Rule1MinTrailingSilence = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Endpoint", meta = (ClampMin = "0.1", Units = "s"))
	float Rule2MinTrailingSilence = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Endpoint", meta = (ClampMin = "1.0", Units = "s"))
	float Rule3MinUtteranceLength = 15.0f;

	/**
	 * How long the microphone device stays open after a request ends, so back-to-back requests do
	 * not pay for closing and reopening it. Callers that listen continuously restart within the
	 * same frame, and reopening the device there costs enough audio to swallow the first syllable
	 * of whatever is being said.
	 *
	 * No audio is buffered or decoded while idle: the capture callback discards it until the next
	 * StartListening. Set to 0 to close the device the moment a request ends.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa|Capture", meta = (ClampMin = "0.0", Units = "s"))
	float CaptureIdleTimeout = 2.0f;

	/** Prints the resolved model paths and lets sherpa-onnx log its own configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa")
	bool bDebugLogging = false;

	/** Loading the model takes seconds, so it starts at level load rather than at the first quiz. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa")
	bool bInitializeOnBeginPlay = true;

	/** Android only. Asks for RECORD_AUDIO on BeginPlay so the first quiz is not blocked by a dialog. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Voice|Sherpa")
	bool bRequestMicrophonePermissionOnBeginPlay = true;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy() override;

	virtual bool BeginBackendListening_Implementation(const FVoiceRecognitionRequest& Request) override;
	virtual void EndBackendListening_Implementation() override;

private:
	friend class FSherpaDecodeWorker;

	void InitializeRecognizerOnWorkerThread();
	/** Publishes an initialization failure back onto the game thread. */
	void FailInitialization(ESherpaRecognizerStatus Status, const FString& Error);
	void DestroyRecognizer();
	void RequestMicrophonePermission();

	bool StartCapture();
	void StopCapture();

	/** Closes the microphone once CaptureIdleTimeout has passed with no new request. */
	UFUNCTION()
	void HandleCaptureIdleTimeout();

	/** Cancels a pending idle close, either because a new request arrived or the component is going away. */
	void CancelCaptureIdleTimeout();

	/** Worker-thread entry point: consumes captured audio, decodes, reports on the game thread. */
	void ProcessCapturedAudio();
	void ReportFromWorker(const FString& RecognizedText);
	FString ResolveSpokenKeyword(const FString& RecognizedText) const;

	/** Raw on purpose: a smart pointer would need this type to be complete in generated code. */
	FSherpaRecognizerHandles* Handles = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Voice|Sherpa", meta = (AllowPrivateAccess = "true"))
	ESherpaRecognizerStatus RecognizerStatus = ESherpaRecognizerStatus::NotInitialized;

	FString InitializationError;
	FString ResolvedModelDirectory;

	/** What the recognizer was actually built with, which is not always what was asked for. */
	FString ResolvedDecodingMethod;
	bool bHotwordsActive = false;

	FTimerHandle CaptureIdleHandle;
};
