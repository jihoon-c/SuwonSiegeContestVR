#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Async/TaskGraphInterfaces.h"
#include "Core/Voice/SherpaVoiceRecognitionComponent.h"
#include "Core/Voice/VoiceModelLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"

namespace
{
	/** Model loading finishes on a worker and publishes on the game thread, so pump both. */
	bool WaitForRecognizer(USherpaVoiceRecognitionComponent& Component, const double TimeoutSeconds)
	{
		const double Deadline = FPlatformTime::Seconds() + TimeoutSeconds;
		while (Component.GetRecognizerStatus() == ESherpaRecognizerStatus::Initializing
			&& FPlatformTime::Seconds() < Deadline)
		{
			FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
			FPlatformProcess::Sleep(0.05f);
		}
		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		return Component.GetRecognizerStatus() == ESherpaRecognizerStatus::Ready;
	}
}

/**
 * Runs the real on-device recognizer over the wave files shipped with the Korean model, so the
 * backend can be verified on a build machine that has no microphone.
 * Skips with a warning when the model has not been downloaded.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSherpaKoreanDecodeTest,
	"Suwon.Core.Voice.SherpaKoreanDecode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSherpaKoreanDecodeTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world is created"), World)) return false;
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	FURL URL;
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AActor* Host = World->SpawnActor<AActor>();
	USherpaVoiceRecognitionComponent* Voice =
		NewObject<USherpaVoiceRecognitionComponent>(Host, TEXT("TestSherpaVoice"));
	// The test drives initialization explicitly and never opens a microphone.
	Voice->bInitializeOnBeginPlay = false;
	Voice->bRequestMicrophonePermissionOnBeginPlay = false;
	Voice->RegisterComponent();

	const FString ModelDirectory = UVoiceModelLibrary::GetStagedModelDirectory(Voice->ModelName);
	if (!UVoiceModelLibrary::IsModelStaged(Voice->ModelName, Voice->GetRequiredModelFiles()))
	{
		AddWarning(FString::Printf(
			TEXT("Skipped: the speech model is not in %s. Run Scripts/DownloadKoreanVoiceModel.py."),
			*ModelDirectory));
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return true;
	}

	TestTrue(TEXT("The sherpa-onnx native library loads"),
		USherpaVoiceRecognitionComponent::IsNativeLibraryAvailable());

	const double LoadStart = FPlatformTime::Seconds();
	TestTrue(TEXT("Initialization starts"), Voice->BeginInitialization());
	const bool bReady = WaitForRecognizer(*Voice, 120.0);
	TestTrue(TEXT("The Korean recognizer becomes ready"), bReady);

	if (bReady)
	{
		AddInfo(FString::Printf(TEXT("Recognizer ready in %.1fs."), FPlatformTime::Seconds() - LoadStart));

		// The transcripts come from test_wavs/trans.txt in the model package.
		const TArray<TPair<FString, FString>> Cases = {
			{ TEXT("test_wavs/0.wav"), TEXT("괜찮") },
			{ TEXT("test_wavs/1.wav"), TEXT("지하철") },
		};

		for (const TPair<FString, FString>& Case : Cases)
		{
			const FString WavePath = FPaths::Combine(ModelDirectory, Case.Key);
			if (!FPaths::FileExists(WavePath))
			{
				AddWarning(FString::Printf(TEXT("Skipped %s: file not downloaded."), *Case.Key));
				continue;
			}

			FString Recognized;
			const double DecodeStart = FPlatformTime::Seconds();
			const bool bDecoded = Voice->DecodeWaveFile(WavePath, Recognized);
			AddInfo(FString::Printf(TEXT("%s -> \"%s\" (%.2fs)"),
				*Case.Key, *Recognized, FPlatformTime::Seconds() - DecodeStart));

			TestTrue(FString::Printf(TEXT("%s produces Korean text"), *Case.Key), bDecoded);
			TestTrue(FString::Printf(TEXT("%s contains \"%s\""), *Case.Key, *Case.Value),
				Recognized.Contains(Case.Value));
		}
	}
	else
	{
		AddError(FString::Printf(TEXT("Recognizer status: %s"), *Voice->GetInitializationError()));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
