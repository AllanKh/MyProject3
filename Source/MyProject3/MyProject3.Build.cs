using UnrealBuildTool;

public class MyProject3 : ModuleRules
{
    public MyProject3(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "AIModule",
            "EnhancedInput",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore"
        });

        PublicIncludePaths.AddRange(new string[] { "MyProject3/Public" });
        PrivateIncludePaths.AddRange(new string[] { "MyProject3/Private" });
    }
}
