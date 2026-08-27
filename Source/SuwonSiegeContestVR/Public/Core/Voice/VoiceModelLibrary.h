#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoiceModelLibrary.generated.h"

/**
 * Locates the on-device speech model files at runtime.
 *
 * Models are large, so they are not committed: Scripts/DownloadKoreanVoiceModel.py fetches them
 * into <Project>/VoiceModels/, which is staged as non-UFS content for packaged builds.
 *
 * On Android the staged files live inside the OBB, where a native library cannot open them by
 * path, so they are copied once to the persistent download directory and used from there.
 */
UCLASS()
class SUWONSIEGECONTESTVR_API UVoiceModelLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Project-relative root that holds every voice model directory. */
	static const TCHAR* GetVoiceModelsFolderName();

	/** Directory the game reads models from, without checking that it exists. */
	UFUNCTION(BlueprintPure, Category = "Voice|Model")
	static FString GetStagedModelDirectory(const FString& ModelName);

	/** True when every listed file is present in the staged directory. */
	UFUNCTION(BlueprintPure, Category = "Voice|Model")
	static bool IsModelStaged(const FString& ModelName, const TArray<FString>& RequiredFiles);

	/**
	 * Returns a directory whose files can be opened by a native library, copying them out of the
	 * package first where the platform requires it. Blocking: call this off the game thread.
	 * Returns an empty string when the model is missing.
	 *
	 * Optional files are extracted when they happen to be staged but never gate the result, which
	 * is how bpe.vocab can be absent on a checkout that has not re-run the download script.
	 */
	static FString ResolveNativeModelDirectory(const FString& ModelName, const TArray<FString>& RequiredFiles,
		const TArray<FString>& OptionalFiles = TArray<FString>());

	/** Human-readable hint pointing at the download script. Used in logs and on the quiz panel. */
	UFUNCTION(BlueprintPure, Category = "Voice|Model")
	static FText GetMissingModelHint(const FString& ModelName);
};
