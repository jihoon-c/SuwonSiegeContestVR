#include "Core/Voice/SherpaVoiceRecognitionComponent.h"

#include "Async/Async.h"
#include "Core/Text/HangulTextLibrary.h"
#include "Core/Voice/VoiceModelLibrary.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/ThreadSafeBool.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "TimerManager.h"

#define SUWON_WITH_SHERPA (PLATFORM_WINDOWS || PLATFORM_ANDROID)

#if PLATFORM_ANDROID
#include "AndroidPermissionFunctionLibrary.h"
#endif

#if SUWON_WITH_SHERPA
#include "AudioCaptureCore.h"
THIRD_PARTY_INCLUDES_START
#include "sherpa-onnx/c-api/c-api.h"
THIRD_PARTY_INCLUDES_END
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSherpaVoice, Log, All);

/** Native state kept out of the header so no gameplay code has to see the C API. */
struct FSherpaRecognizerHandles
{
#if SUWON_WITH_SHERPA
	const SherpaOnnxOnlineRecognizer* Recognizer = nullptr;
	const SherpaOnnxOnlineStream* Stream = nullptr;
	Audio::FAudioCapture AudioCapture;
#endif

	/** Written by the audio callback, drained by the decode worker. */
	FCriticalSection AudioLock;
	TArray<float> PendingSamples;
	int32 CaptureSampleRate = 16000;

	/**
	 * The device can outlive a request by CaptureIdleTimeout so consecutive requests do not reopen
	 * it. This gates the callback so nothing is recorded in that window.
	 */
	FThreadSafeBool bAcceptSamples = false;

	TArray<FString> Keywords;
	TFuture<void> DecodeWorker;
	TFuture<void> InitWorker;
	FThreadSafeBool bStopRequested = false;
};

namespace
{
#if PLATFORM_WINDOWS
	void* GOnnxRuntimeHandle = nullptr;
	void* GSherpaHandle = nullptr;
	bool GNativeLoadAttempted = false;
	bool GNativeLoaded = false;
#endif

	/** 20ms of work per pass keeps the decoder responsive without spinning a core. */
	constexpr float WorkerSleepSeconds = 0.02f;
}

bool USherpaVoiceRecognitionComponent::IsNativeLibraryAvailable()
{
#if PLATFORM_WINDOWS
	if (GNativeLoadAttempted)
	{
		return GNativeLoaded;
	}
	GNativeLoadAttempted = true;

	// The DLLs are delay loaded, so they have to be resolved before the first API call.
	const FString BinariesDir = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64")));
	FPlatformProcess::PushDllDirectory(*BinariesDir);
	GOnnxRuntimeHandle = FPlatformProcess::GetDllHandle(TEXT("onnxruntime.dll"));
	GSherpaHandle = FPlatformProcess::GetDllHandle(TEXT("sherpa-onnx-c-api.dll"));
	FPlatformProcess::PopDllDirectory(*BinariesDir);

	GNativeLoaded = GOnnxRuntimeHandle != nullptr && GSherpaHandle != nullptr;
	if (!GNativeLoaded)
	{
		UE_LOG(LogSherpaVoice, Warning,
			TEXT("sherpa-onnx libraries were not found in %s. Speech recognition stays unavailable."),
			*BinariesDir);
	}
	return GNativeLoaded;
#elif PLATFORM_ANDROID
	// Loaded by the APL soLoadLibrary entries before the game module starts.
	return true;
#else
	return false;
#endif
}

FString USherpaVoiceRecognitionComponent::GetBackendDescription() const
{
	static const UEnum* StatusEnum = StaticEnum<ESherpaRecognizerStatus>();
	FString Description = FString::Printf(TEXT("%s, model %s: %s"),
		*Super::GetBackendDescription(), *ModelName,
		*StatusEnum->GetNameStringByValue(static_cast<int64>(RecognizerStatus)));

	if (!ResolvedDecodingMethod.IsEmpty())
	{
		Description += FString::Printf(TEXT(" | %s, hotwords %s"),
			*ResolvedDecodingMethod, bHotwordsActive ? TEXT("on") : TEXT("off"));
	}
	if (!ResolvedModelDirectory.IsEmpty())
	{
		Description += FString::Printf(TEXT(" | files: %s"), *ResolvedModelDirectory);
	}
	if (!InitializationError.IsEmpty())
	{
		Description += FString::Printf(TEXT(" | error: %s"), *InitializationError);
	}
	return Description;
}

USherpaVoiceRecognitionComponent::USherpaVoiceRecognitionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Handles = new FSherpaRecognizerHandles();
}

USherpaVoiceRecognitionComponent::~USherpaVoiceRecognitionComponent()
{
	DestroyRecognizer();
	delete Handles;
	Handles = nullptr;
}

TArray<FString> USherpaVoiceRecognitionComponent::GetRequiredModelFiles() const
{
	return { EncoderFile, DecoderFile, JoinerFile, TokensFile };
}

TArray<FString> USherpaVoiceRecognitionComponent::GetOptionalModelFiles() const
{
	// Deliberately not required: a checkout that has not re-run the download script still has a
	// working recognizer, just without hotword biasing.
	if (bUseHotwords && !BpeVocabFile.IsEmpty())
	{
		return { BpeVocabFile };
	}
	return {};
}

void USherpaVoiceRecognitionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bRequestMicrophonePermissionOnBeginPlay)
	{
		RequestMicrophonePermission();
	}
	if (bInitializeOnBeginPlay)
	{
		BeginInitialization();
	}
}

void USherpaVoiceRecognitionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// StopListening in the base class stops capture; the recognizer itself is released here.
	Super::EndPlay(EndPlayReason);
	DestroyRecognizer();
}

void USherpaVoiceRecognitionComponent::BeginDestroy()
{
	DestroyRecognizer();
	Super::BeginDestroy();
}

bool USherpaVoiceRecognitionComponent::BeginInitialization()
{
	if (RecognizerStatus == ESherpaRecognizerStatus::Ready
		|| RecognizerStatus == ESherpaRecognizerStatus::Initializing)
	{
		return true;
	}

	if (!IsNativeLibraryAvailable())
	{
		InitializationError = TEXT("sherpa-onnx native library is unavailable on this platform.");
		RecognizerStatus = ESherpaRecognizerStatus::Failed;
		return false;
	}

	if (!UVoiceModelLibrary::IsModelStaged(ModelName, GetRequiredModelFiles()))
	{
		InitializationError = UVoiceModelLibrary::GetMissingModelHint(ModelName).ToString();
		RecognizerStatus = ESherpaRecognizerStatus::ModelMissing;
		UE_LOG(LogSherpaVoice, Warning, TEXT("%s"), *InitializationError);
		return false;
	}

	RecognizerStatus = ESherpaRecognizerStatus::Initializing;
	InitializationError.Reset();

	// Loading a 130MB model takes seconds; keep it off the game thread.
	TWeakObjectPtr<USherpaVoiceRecognitionComponent> WeakThis(this);
	Handles->InitWorker = Async(EAsyncExecution::Thread, [WeakThis]()
	{
		if (USherpaVoiceRecognitionComponent* Self = WeakThis.Get())
		{
			Self->InitializeRecognizerOnWorkerThread();
		}
	});
	return true;
}

void USherpaVoiceRecognitionComponent::InitializeRecognizerOnWorkerThread()
{
#if SUWON_WITH_SHERPA
	const FString ModelDirectory = UVoiceModelLibrary::ResolveNativeModelDirectory(
		ModelName, GetRequiredModelFiles(), GetOptionalModelFiles());
	if (ModelDirectory.IsEmpty())
	{
		FailInitialization(ESherpaRecognizerStatus::ModelMissing,
			UVoiceModelLibrary::GetMissingModelHint(ModelName).ToString());
		return;
	}

	const FString EncoderPath = FPaths::Combine(ModelDirectory, EncoderFile);
	const FString DecoderPath = FPaths::Combine(ModelDirectory, DecoderFile);
	const FString JoinerPath = FPaths::Combine(ModelDirectory, JoinerFile);
	const FString TokensPath = FPaths::Combine(ModelDirectory, TokensFile);
	const FString BpeVocabPath = bUseHotwords && !BpeVocabFile.IsEmpty()
		? FPaths::Combine(ModelDirectory, BpeVocabFile)
		: FString();

	// Hotwords need both the vocabulary and modified_beam_search; without either one the recognizer
	// still has to come up, so degrade to the plain greedy configuration instead of failing.
	bool bHotwordsResolved = !BpeVocabPath.IsEmpty();
	if (bUseHotwords && !bHotwordsResolved)
	{
		UE_LOG(LogSherpaVoice, Warning,
			TEXT("Hotwords are enabled but no BPE vocabulary is configured; decoding without them."));
	}
	else if (bHotwordsResolved && !IFileManager::Get().FileExists(*BpeVocabPath))
	{
		UE_LOG(LogSherpaVoice, Warning,
			TEXT("Hotwords are enabled but %s is missing. Re-run Scripts/DownloadKoreanVoiceModel.py "
				 "to derive it from bpe.model. Decoding without hotwords for now."),
			*BpeVocabPath);
		bHotwordsResolved = false;
	}

	static const FString ModifiedBeamSearch(TEXT("modified_beam_search"));
	FString EffectiveDecodingMethod = DecodingMethod;
	if (bHotwordsResolved && EffectiveDecodingMethod != ModifiedBeamSearch)
	{
		UE_LOG(LogSherpaVoice, Warning,
			TEXT("Hotwords need modified_beam_search; overriding the configured %s."), *DecodingMethod);
		EffectiveDecodingMethod = ModifiedBeamSearch;
	}
	else if (!bHotwordsResolved && bUseHotwords && EffectiveDecodingMethod == ModifiedBeamSearch)
	{
		// modified_beam_search only earns its cost through hotwords, so pay for neither.
		EffectiveDecodingMethod = TEXT("greedy_search");
	}

	const FTCHARToUTF8 EncoderUtf8(*EncoderPath);
	const FTCHARToUTF8 DecoderUtf8(*DecoderPath);
	const FTCHARToUTF8 JoinerUtf8(*JoinerPath);
	const FTCHARToUTF8 TokensUtf8(*TokensPath);
	const FTCHARToUTF8 BpeVocabUtf8(*BpeVocabPath);
	const FTCHARToUTF8 DecodingUtf8(*EffectiveDecodingMethod);

	SherpaOnnxOnlineRecognizerConfig Config;
	FMemory::Memzero(&Config, sizeof(Config));

	Config.feat_config.sample_rate = 16000;
	Config.feat_config.feature_dim = 80;
	Config.model_config.transducer.encoder = EncoderUtf8.Get();
	Config.model_config.transducer.decoder = DecoderUtf8.Get();
	Config.model_config.transducer.joiner = JoinerUtf8.Get();
	Config.model_config.tokens = TokensUtf8.Get();
	Config.model_config.num_threads = FMath::Clamp(NumThreads, 1, 4);
	Config.model_config.provider = "cpu";
	Config.model_config.debug = bDebugLogging ? 1 : 0;
	if (bHotwordsResolved)
	{
		// sherpa-onnx maps raw hotword text onto BPE units with this vocabulary; without it the
		// hotwords would have to be handed over already tokenised.
		Config.model_config.modeling_unit = "bpe";
		Config.model_config.bpe_vocab = BpeVocabUtf8.Get();
		Config.hotwords_score = HotwordsScore;
	}
	Config.decoding_method = DecodingUtf8.Get();
	Config.max_active_paths = 4;
	Config.enable_endpoint = 1;
	Config.rule1_min_trailing_silence = Rule1MinTrailingSilence;
	Config.rule2_min_trailing_silence = Rule2MinTrailingSilence;
	Config.rule3_min_utterance_length = Rule3MinUtteranceLength;

	IFileManager& FileManager = IFileManager::Get();
	for (const FString& Path : { EncoderPath, DecoderPath, JoinerPath, TokensPath })
	{
		if (!FileManager.FileExists(*Path))
		{
			FailInitialization(ESherpaRecognizerStatus::ModelMissing,
				FString::Printf(TEXT("Voice model file is missing: %s"), *Path));
			return;
		}
		if (bDebugLogging)
		{
			UE_LOG(LogSherpaVoice, Display, TEXT("  model file %s (%lld bytes)"),
				*Path, FileManager.FileSize(*Path));
		}
	}

	const double StartTime = FPlatformTime::Seconds();
	const SherpaOnnxOnlineRecognizer* Recognizer = SherpaOnnxCreateOnlineRecognizer(&Config);
	if (!Recognizer)
	{
		FailInitialization(ESherpaRecognizerStatus::Failed,
			FString::Printf(TEXT("sherpa-onnx could not create a recognizer from %s."), *ModelDirectory));
		return;
	}

	UE_LOG(LogSherpaVoice, Display,
		TEXT("Korean speech recognizer loaded in %.2fs (%s, %s, hotwords %s, %d threads)."),
		FPlatformTime::Seconds() - StartTime, *ModelName, *EffectiveDecodingMethod,
		bHotwordsResolved ? TEXT("on") : TEXT("off"), Config.model_config.num_threads);

	// Publish on the game thread: status, error text and the handle are read from there.
	TWeakObjectPtr<USherpaVoiceRecognitionComponent> WeakThis(this);
	AsyncTask(ENamedThreads::GameThread,
		[WeakThis, Recognizer, ModelDirectory, EffectiveDecodingMethod, bHotwordsResolved]()
	{
		USherpaVoiceRecognitionComponent* Self = WeakThis.Get();
		if (!Self || Self->Handles->bStopRequested)
		{
			// The component went away while the model was loading; do not leak the recognizer.
			SherpaOnnxDestroyOnlineRecognizer(Recognizer);
			return;
		}
		Self->Handles->Recognizer = Recognizer;
		Self->ResolvedModelDirectory = ModelDirectory;
		Self->ResolvedDecodingMethod = EffectiveDecodingMethod;
		Self->bHotwordsActive = bHotwordsResolved;
		Self->RecognizerStatus = ESherpaRecognizerStatus::Ready;
	});
#else
	FailInitialization(ESherpaRecognizerStatus::Failed, TEXT("This platform has no sherpa-onnx build."));
#endif
}

void USherpaVoiceRecognitionComponent::FailInitialization(
	const ESherpaRecognizerStatus Status, const FString& Error)
{
	UE_LOG(LogSherpaVoice, Warning, TEXT("%s"), *Error);

	TWeakObjectPtr<USherpaVoiceRecognitionComponent> WeakThis(this);
	AsyncTask(ENamedThreads::GameThread, [WeakThis, Status, Error]()
	{
		if (USherpaVoiceRecognitionComponent* Self = WeakThis.Get())
		{
			Self->RecognizerStatus = Status;
			Self->InitializationError = Error;
		}
	});
}

void USherpaVoiceRecognitionComponent::DestroyRecognizer()
{
	if (!Handles)
	{
		return;
	}

	CancelCaptureIdleTimeout();

	Handles->bStopRequested = true;
	Handles->bAcceptSamples = false;
	if (Handles->InitWorker.IsValid())
	{
		Handles->InitWorker.Wait();
		Handles->InitWorker.Reset();
	}
	if (Handles->DecodeWorker.IsValid())
	{
		Handles->DecodeWorker.Wait();
		Handles->DecodeWorker.Reset();
	}

#if SUWON_WITH_SHERPA
	StopCapture();
	if (Handles->Stream)
	{
		SherpaOnnxDestroyOnlineStream(Handles->Stream);
		Handles->Stream = nullptr;
	}
	if (Handles->Recognizer)
	{
		SherpaOnnxDestroyOnlineRecognizer(Handles->Recognizer);
		Handles->Recognizer = nullptr;
	}
#endif

	RecognizerStatus = ESherpaRecognizerStatus::NotInitialized;
}

void USherpaVoiceRecognitionComponent::RequestMicrophonePermission()
{
#if PLATFORM_ANDROID
	// Without the runtime grant the capture stream still opens but only ever delivers silence,
	// so ask at level load rather than in the middle of a quiz.
	const FString RecordAudio(TEXT("android.permission.RECORD_AUDIO"));
	if (UAndroidPermissionFunctionLibrary::CheckPermission(RecordAudio))
	{
		return;
	}

	UE_LOG(LogSherpaVoice, Display, TEXT("Requesting the RECORD_AUDIO permission."));
	UAndroidPermissionFunctionLibrary::AcquirePermissions({ RecordAudio });
#endif
}

bool USherpaVoiceRecognitionComponent::BeginBackendListening_Implementation(
	const FVoiceRecognitionRequest& Request)
{
#if SUWON_WITH_SHERPA
	if (RecognizerStatus == ESherpaRecognizerStatus::NotInitialized)
	{
		BeginInitialization();
	}
	if (!Handles->Recognizer)
	{
		UE_LOG(LogSherpaVoice, Warning,
			TEXT("Speech capture was requested before the recognizer was ready (%s)."),
			*InitializationError);
		return false;
	}

	CancelCaptureIdleTimeout();

	Handles->Keywords = Request.Keywords;
	Handles->bStopRequested = false;
	{
		// Anything captured before this request belongs to the previous utterance. Feeding it in
		// would re-report the word that just ended the last request.
		FScopeLock Lock(&Handles->AudioLock);
		Handles->PendingSamples.Reset();
	}

	if (bHotwordsActive && Request.Keywords.Num() > 0)
	{
		// One hotword per line, as raw text: sherpa-onnx maps it onto BPE units with bpe_vocab.
		const FString Hotwords = FString::Join(Request.Keywords, TEXT("\n"));
		const FTCHARToUTF8 HotwordsUtf8(*Hotwords);
		Handles->Stream = SherpaOnnxCreateOnlineStreamWithHotwords(Handles->Recognizer, HotwordsUtf8.Get());

		if (!Handles->Stream)
		{
			// A keyword sherpa cannot encode must not cost the player the whole recognizer; a plain
			// stream still recognizes the word, it just is not boosted.
			UE_LOG(LogSherpaVoice, Warning,
				TEXT("sherpa-onnx rejected the hotwords \"%s\"; listening without them."),
				*Hotwords.Replace(TEXT("\n"), TEXT(", ")));
		}
	}

	if (!Handles->Stream)
	{
		Handles->Stream = SherpaOnnxCreateOnlineStream(Handles->Recognizer);
	}

	if (!Handles->Stream)
	{
		UE_LOG(LogSherpaVoice, Error, TEXT("sherpa-onnx could not create a decoding stream."));
		return false;
	}

	if (!StartCapture())
	{
		SherpaOnnxDestroyOnlineStream(Handles->Stream);
		Handles->Stream = nullptr;
		return false;
	}

	TWeakObjectPtr<USherpaVoiceRecognitionComponent> WeakThis(this);
	Handles->DecodeWorker = Async(EAsyncExecution::Thread, [WeakThis]()
	{
		if (USherpaVoiceRecognitionComponent* Self = WeakThis.Get())
		{
			Self->ProcessCapturedAudio();
		}
	});

	UE_LOG(LogSherpaVoice, Display, TEXT("Listening for %d keyword(s) at %d Hz."),
		Request.Keywords.Num(), Handles->CaptureSampleRate);
	return true;
#else
	return false;
#endif
}

void USherpaVoiceRecognitionComponent::EndBackendListening_Implementation()
{
#if SUWON_WITH_SHERPA
	Handles->bStopRequested = true;
	Handles->bAcceptSamples = false;
	if (Handles->DecodeWorker.IsValid())
	{
		Handles->DecodeWorker.Wait();
		Handles->DecodeWorker.Reset();
	}

	if (Handles->Stream)
	{
		SherpaOnnxDestroyOnlineStream(Handles->Stream);
		Handles->Stream = nullptr;
	}

	// The decoder state is gone, so nothing is being recorded either way. Holding the device open
	// for a moment is purely so a caller that listens continuously does not reopen it every
	// utterance and lose the start of the next one.
	UWorld* World = GetWorld();
	if (CaptureIdleTimeout > 0.0f && World && !World->bIsTearingDown)
	{
		World->GetTimerManager().SetTimer(CaptureIdleHandle, this,
			&USherpaVoiceRecognitionComponent::HandleCaptureIdleTimeout, CaptureIdleTimeout, false);
	}
	else
	{
		StopCapture();
	}
#endif
}

void USherpaVoiceRecognitionComponent::HandleCaptureIdleTimeout()
{
	StopCapture();
}

void USherpaVoiceRecognitionComponent::CancelCaptureIdleTimeout()
{
	if (!CaptureIdleHandle.IsValid())
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CaptureIdleHandle);
	}
	CaptureIdleHandle.Invalidate();
}

bool USherpaVoiceRecognitionComponent::StartCapture()
{
#if SUWON_WITH_SHERPA
	// Still open from the previous request: reopening is exactly what CaptureIdleTimeout avoids.
	if (Handles->AudioCapture.IsStreamOpen())
	{
		Handles->bAcceptSamples = true;
		return true;
	}

	Audio::FAudioCaptureDeviceParams Params;
	Params.NumInputChannels = 1;

	// Which microphone is used matters in PC/Link mode, where the headset mic is just one of the
	// Windows input devices, so name it in the log rather than leaving it to guesswork.
	Audio::FCaptureDeviceInfo DeviceInfo;
	if (Handles->AudioCapture.GetCaptureDeviceInfo(DeviceInfo))
	{
		UE_LOG(LogSherpaVoice, Display, TEXT("Capture device: %s (%d ch, %d Hz)."),
			*DeviceInfo.DeviceName, DeviceInfo.InputChannels, DeviceInfo.PreferredSampleRate);
	}
	else
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("No capture device was reported by the audio system."));
	}

	FSherpaRecognizerHandles* LocalHandles = Handles;
	Audio::FOnAudioCaptureFunction OnCapture =
		[LocalHandles](const void* InAudio, int32 NumFrames, int32 NumChannels, int32 SampleRate, double, bool)
	{
		// The device can be open between requests; discard rather than record during that window.
		if (!InAudio || NumFrames <= 0 || NumChannels <= 0 || !LocalHandles->bAcceptSamples)
		{
			return;
		}

		const float* Samples = static_cast<const float*>(InAudio);
		FScopeLock Lock(&LocalHandles->AudioLock);
		LocalHandles->CaptureSampleRate = SampleRate;
		LocalHandles->PendingSamples.Reserve(LocalHandles->PendingSamples.Num() + NumFrames);
		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			// sherpa-onnx wants mono in [-1, 1] and resamples to 16kHz internally.
			float Mixed = 0.0f;
			for (int32 Channel = 0; Channel < NumChannels; ++Channel)
			{
				Mixed += Samples[Frame * NumChannels + Channel];
			}
			LocalHandles->PendingSamples.Add(Mixed / static_cast<float>(NumChannels));
		}
	};

	if (!Handles->AudioCapture.OpenAudioCaptureStream(Params, MoveTemp(OnCapture), 1024))
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("No microphone could be opened for speech recognition."));
		return false;
	}
	if (!Handles->AudioCapture.StartStream())
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("The microphone stream refused to start."));
		Handles->AudioCapture.CloseStream();
		return false;
	}

	Handles->bAcceptSamples = true;
	return true;
#else
	return false;
#endif
}

void USherpaVoiceRecognitionComponent::StopCapture()
{
#if SUWON_WITH_SHERPA
	Handles->bAcceptSamples = false;
	if (Handles->AudioCapture.IsStreamOpen())
	{
		Handles->AudioCapture.StopStream();
		Handles->AudioCapture.CloseStream();
	}
#endif
}

void USherpaVoiceRecognitionComponent::ProcessCapturedAudio()
{
#if SUWON_WITH_SHERPA
	TArray<float> Batch;
	while (!Handles->bStopRequested)
	{
		{
			FScopeLock Lock(&Handles->AudioLock);
			Batch = MoveTemp(Handles->PendingSamples);
			Handles->PendingSamples.Reset();
		}

		if (Batch.Num() > 0 && Handles->Stream && Handles->Recognizer)
		{
			SherpaOnnxOnlineStreamAcceptWaveform(
				Handles->Stream, Handles->CaptureSampleRate, Batch.GetData(), Batch.Num());

			while (SherpaOnnxIsOnlineStreamReady(Handles->Recognizer, Handles->Stream))
			{
				SherpaOnnxDecodeOnlineStream(Handles->Recognizer, Handles->Stream);
			}

			FString Text;
			if (const SherpaOnnxOnlineRecognizerResult* Result =
				SherpaOnnxGetOnlineStreamResult(Handles->Recognizer, Handles->Stream))
			{
				Text = UTF8_TO_TCHAR(Result->text);
				SherpaOnnxDestroyOnlineRecognizerResult(Result);
			}

			// Answering early keeps the quiz snappy: as soon as the expected word appears there is
			// no reason to wait out the trailing-silence rule.
			const FString Keyword = ResolveSpokenKeyword(Text);
			if (!Keyword.IsEmpty())
			{
				ReportFromWorker(Keyword);
				return;
			}

			if (SherpaOnnxOnlineStreamIsEndpoint(Handles->Recognizer, Handles->Stream))
			{
				SherpaOnnxOnlineStreamReset(Handles->Recognizer, Handles->Stream);
				if (!Text.IsEmpty())
				{
					// Nothing matched, but the player did say something: let the quiz judge it.
					ReportFromWorker(Text);
					return;
				}
			}
		}

		Batch.Reset();
		FPlatformProcess::Sleep(WorkerSleepSeconds);
	}
#endif
}

FString USherpaVoiceRecognitionComponent::ResolveSpokenKeyword(const FString& RecognizedText) const
{
	if (RecognizedText.IsEmpty() || Handles->Keywords.Num() == 0)
	{
		return FString();
	}

	const FString Normalized = UHangulTextLibrary::NormalizeAnswer(RecognizedText);
	if (Normalized.IsEmpty())
	{
		return FString();
	}

	for (const FString& Keyword : Handles->Keywords)
	{
		const FString NormalizedKeyword = UHangulTextLibrary::NormalizeAnswer(Keyword);
		if (NormalizedKeyword.IsEmpty())
		{
			continue;
		}
		const bool bFound = bMatchKeywordInsideUtterance
			? Normalized.Contains(NormalizedKeyword)
			: Normalized == NormalizedKeyword;
		if (bFound)
		{
			return Keyword;
		}
	}

	return FString();
}

void USherpaVoiceRecognitionComponent::ReportFromWorker(const FString& RecognizedText)
{
	TWeakObjectPtr<USherpaVoiceRecognitionComponent> WeakThis(this);
	AsyncTask(ENamedThreads::GameThread, [WeakThis, RecognizedText]()
	{
		if (USherpaVoiceRecognitionComponent* Self = WeakThis.Get())
		{
			UE_LOG(LogSherpaVoice, Display, TEXT("Recognized \"%s\"."), *RecognizedText);
			Self->ReportRecognizedText(RecognizedText, 1.0f);
		}
	});
}

bool USherpaVoiceRecognitionComponent::DecodeWaveFile(const FString& WaveFilePath, FString& OutRecognizedText)
{
	OutRecognizedText.Reset();

#if SUWON_WITH_SHERPA
	if (!Handles->Recognizer)
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("DecodeWaveFile needs a ready recognizer."));
		return false;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *WaveFilePath))
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("Could not read %s."), *WaveFilePath);
		return false;
	}

	// Minimal RIFF walk: the bundled test wavs are 16-bit PCM, which is all this debug path needs.
	if (FileData.Num() < 44 || FMemory::Memcmp(FileData.GetData(), "RIFF", 4) != 0
		|| FMemory::Memcmp(FileData.GetData() + 8, "WAVE", 4) != 0)
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("%s is not a RIFF/WAVE file."), *WaveFilePath);
		return false;
	}

	int32 SampleRate = 16000;
	int32 NumChannels = 1;
	int32 BitsPerSample = 16;
	const uint8* DataChunk = nullptr;
	int32 DataSize = 0;

	for (int32 Offset = 12; Offset + 8 <= FileData.Num();)
	{
		const uint8* Chunk = FileData.GetData() + Offset;
		uint32 ChunkSize = 0;
		FMemory::Memcpy(&ChunkSize, Chunk + 4, sizeof(uint32));

		if (FMemory::Memcmp(Chunk, "fmt ", 4) == 0 && ChunkSize >= 16)
		{
			uint16 Channels16 = 0, Bits16 = 0;
			uint32 Rate32 = 0;
			FMemory::Memcpy(&Channels16, Chunk + 8 + 2, sizeof(uint16));
			FMemory::Memcpy(&Rate32, Chunk + 8 + 4, sizeof(uint32));
			FMemory::Memcpy(&Bits16, Chunk + 8 + 14, sizeof(uint16));
			NumChannels = FMath::Max<int32>(1, Channels16);
			SampleRate = FMath::Max<int32>(1, static_cast<int32>(Rate32));
			BitsPerSample = Bits16;
		}
		else if (FMemory::Memcmp(Chunk, "data", 4) == 0)
		{
			DataChunk = Chunk + 8;
			DataSize = FMath::Min<int32>(static_cast<int32>(ChunkSize), FileData.Num() - Offset - 8);
			break;
		}

		Offset += 8 + static_cast<int32>(ChunkSize) + (ChunkSize & 1);
	}

	if (!DataChunk || DataSize <= 0 || BitsPerSample != 16)
	{
		UE_LOG(LogSherpaVoice, Warning, TEXT("%s is not 16-bit PCM."), *WaveFilePath);
		return false;
	}

	const int32 NumFrames = DataSize / (2 * NumChannels);
	TArray<float> Samples;
	Samples.Reserve(NumFrames);
	const int16* Pcm = reinterpret_cast<const int16*>(DataChunk);
	for (int32 Frame = 0; Frame < NumFrames; ++Frame)
	{
		int32 Mixed = 0;
		for (int32 Channel = 0; Channel < NumChannels; ++Channel)
		{
			Mixed += Pcm[Frame * NumChannels + Channel];
		}
		Samples.Add(static_cast<float>(Mixed) / (NumChannels * 32768.0f));
	}

	const SherpaOnnxOnlineStream* Stream = SherpaOnnxCreateOnlineStream(Handles->Recognizer);
	if (!Stream)
	{
		return false;
	}

	SherpaOnnxOnlineStreamAcceptWaveform(Stream, SampleRate, Samples.GetData(), Samples.Num());
	// Trailing silence flushes the last frames out of the feature extractor.
	TArray<float> Tail;
	Tail.AddZeroed(SampleRate / 2);
	SherpaOnnxOnlineStreamAcceptWaveform(Stream, SampleRate, Tail.GetData(), Tail.Num());
	SherpaOnnxOnlineStreamInputFinished(Stream);

	while (SherpaOnnxIsOnlineStreamReady(Handles->Recognizer, Stream))
	{
		SherpaOnnxDecodeOnlineStream(Handles->Recognizer, Stream);
	}

	if (const SherpaOnnxOnlineRecognizerResult* Result =
		SherpaOnnxGetOnlineStreamResult(Handles->Recognizer, Stream))
	{
		OutRecognizedText = UTF8_TO_TCHAR(Result->text);
		SherpaOnnxDestroyOnlineRecognizerResult(Result);
	}
	SherpaOnnxDestroyOnlineStream(Stream);

	return !OutRecognizedText.IsEmpty();
#else
	return false;
#endif
}
