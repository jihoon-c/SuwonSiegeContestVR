// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SuwonSiegeContestVR : ModuleRules
{
	public SuwonSiegeContestVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"InputCore",
			"HeadMountedDisplay",
			"NavigationSystem",
			"AIModule",
			"GameplayTasks",
			"Niagara",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// On-device speech recognition for the initial-consonant quiz.
		// AudioCapture brings in the per-platform microphone backends (WASAPI/RtAudio, Android).
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AudioCapture",
			"AudioCaptureCore",
			"SherpaOnnx"
		});

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Runtime RECORD_AUDIO grant.
			PrivateDependencyModuleNames.Add("AndroidPermission");
		}

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
