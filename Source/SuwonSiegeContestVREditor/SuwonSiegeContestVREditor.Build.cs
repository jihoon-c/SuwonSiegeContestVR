using UnrealBuildTool;

public class SuwonSiegeContestVREditor : ModuleRules
{
	public SuwonSiegeContestVREditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"ContentBrowser",
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"SuwonSiegeContestVR",
			"ToolMenus",
			"UnrealEd"
		});
	}
}
