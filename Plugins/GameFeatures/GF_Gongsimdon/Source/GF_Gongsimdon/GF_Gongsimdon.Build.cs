using UnrealBuildTool;

public class GF_Gongsimdon : ModuleRules
{
    public GF_Gongsimdon(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "HeadMountedDisplay",
            "SuwonSiegeContestVR"
        });
    }
}
