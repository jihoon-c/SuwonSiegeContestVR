#include "Core/Voice/VoiceModelLibrary.h"

#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "VoiceModel"

const TCHAR* UVoiceModelLibrary::GetVoiceModelsFolderName()
{
	return TEXT("VoiceModels");
}

FString UVoiceModelLibrary::GetStagedModelDirectory(const FString& ModelName)
{
	return FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectDir(), GetVoiceModelsFolderName(), ModelName));
}

bool UVoiceModelLibrary::IsModelStaged(const FString& ModelName, const TArray<FString>& RequiredFiles)
{
	const FString Directory = GetStagedModelDirectory(ModelName);
	IFileManager& FileManager = IFileManager::Get();

	for (const FString& RelativeFile : RequiredFiles)
	{
		if (!FileManager.FileExists(*FPaths::Combine(Directory, RelativeFile)))
		{
			return false;
		}
	}

	return RequiredFiles.Num() > 0;
}

FString UVoiceModelLibrary::ResolveNativeModelDirectory(
	const FString& ModelName, const TArray<FString>& RequiredFiles, const TArray<FString>& OptionalFiles)
{
	const FString StagedDirectory = GetStagedModelDirectory(ModelName);
	if (!IsModelStaged(ModelName, RequiredFiles))
	{
		UE_LOG(LogTemp, Warning, TEXT("Voice model %s is not staged under %s."),
			*ModelName, *StagedDirectory);
		return FString();
	}

#if PLATFORM_ANDROID
	// The staged files sit inside the OBB, which onnxruntime cannot open. Copy them once into a
	// real filesystem location and hand that path to the native library instead.
	const FString NativeDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectPersistentDownloadDir(), GetVoiceModelsFolderName(), ModelName));

	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> FilesToExtract = RequiredFiles;
	for (const FString& RelativeFile : OptionalFiles)
	{
		// An optional file that was never provisioned is not an error: the caller degrades instead.
		if (FileManager.FileExists(*FPaths::Combine(StagedDirectory, RelativeFile)))
		{
			FilesToExtract.AddUnique(RelativeFile);
		}
	}

	for (const FString& RelativeFile : FilesToExtract)
	{
		const FString SourceFile = FPaths::Combine(StagedDirectory, RelativeFile);
		const FString TargetFile = FPaths::Combine(NativeDirectory, RelativeFile);

		// Size is enough to detect a partial or outdated extraction without hashing 130MB.
		if (FileManager.FileExists(*TargetFile)
			&& FileManager.FileSize(*TargetFile) == FileManager.FileSize(*SourceFile))
		{
			continue;
		}

		UE_LOG(LogTemp, Display, TEXT("Extracting voice model file %s to the device."), *RelativeFile);
		if (FileManager.Copy(*TargetFile, *SourceFile, true, true) != COPY_OK)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to extract voice model file %s."), *RelativeFile);
			return FString();
		}
	}

	return NativeDirectory;
#else
	return StagedDirectory;
#endif
}

FText UVoiceModelLibrary::GetMissingModelHint(const FString& ModelName)
{
	return FText::Format(
		LOCTEXT("MissingModel", "음성 인식 모델이 없습니다. Scripts/DownloadKoreanVoiceModel.py 를 실행해 {0} 을(를) 내려받으세요."),
		FText::FromString(ModelName));
}

#undef LOCTEXT_NAMESPACE
