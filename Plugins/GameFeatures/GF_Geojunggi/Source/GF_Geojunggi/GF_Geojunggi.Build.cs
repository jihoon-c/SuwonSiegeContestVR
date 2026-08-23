using UnrealBuildTool;

public class GF_Geojunggi : ModuleRules
{
	public GF_Geojunggi(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "HeadMountedDisplay", "InputCore",
			"SuwonSiegeContestVR"
		});
	}
}
