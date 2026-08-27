using System.IO;
using UnrealBuildTool;

/**
 * sherpa-onnx v1.13.6 (Apache-2.0) prebuilt C API, used for on-device Korean speech recognition.
 *
 * Only the C API and onnxruntime are vendored: Win64 for the editor and PC/Link builds, and
 * arm64-v8a for the Android standalone headset build. See
 * docs/Core/specs/VOICE_RECOGNITION_BACKEND_SURVEY.md for why this backend was chosen and
 * docs/Core/specs/SHERPA_ONNX_INTEGRATION.md for how the models are provisioned.
 */
public class SherpaOnnx : ModuleRules
{
	public SherpaOnnx(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string Win64Dir = Path.Combine(ModuleDirectory, "Win64");

			PublicAdditionalLibraries.Add(Path.Combine(Win64Dir, "sherpa-onnx-c-api.lib"));

			// Loaded lazily so a build without the DLLs still links; the component reports the
			// recognizer as unavailable instead of failing to start the game.
			PublicDelayLoadDLLs.Add("sherpa-onnx-c-api.dll");
			PublicDelayLoadDLLs.Add("onnxruntime.dll");

			foreach (string Dll in new string[] {
				"sherpa-onnx-c-api.dll", "onnxruntime.dll", "onnxruntime_providers_shared.dll" })
			{
				RuntimeDependencies.Add("$(BinaryOutputDir)/" + Dll, Path.Combine(Win64Dir, Dll));
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Only arm64-v8a is shipped: the target headsets are all 64-bit.
			string ArmDir = Path.Combine(ModuleDirectory, "Android", "arm64-v8a");

			PublicAdditionalLibraries.Add(Path.Combine(ArmDir, "libsherpa-onnx-c-api.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ArmDir, "libonnxruntime.so"));

			// Packages the shared objects into the APK and declares RECORD_AUDIO.
			AdditionalPropertiesForReceipt.Add(
				"AndroidPlugin", Path.Combine(ModuleDirectory, "SherpaOnnx_APL.xml"));
		}
	}
}
