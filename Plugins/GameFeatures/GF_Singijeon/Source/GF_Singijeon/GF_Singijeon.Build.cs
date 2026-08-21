using UnrealBuildTool;

public class GF_Singijeon : ModuleRules
{
    public GF_Singijeon(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "NavigationSystem",
            "SuwonSiegeContestVR"
        });
    }
}
