using UnrealBuildTool;

public class GF_OngseongCrossbow : ModuleRules
{
	public GF_OngseongCrossbow(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"SuwonSiegeContestVR"
		});
	}
}
