using UnrealBuildTool;

using System.IO;

public class FacePipe : ModuleRules
{
	public FacePipe(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;
        bLegacyPublicIncludePaths = false;

        PublicIncludePaths.AddRange(new string[] { });
		PrivateIncludePaths.AddRange(new string[] { });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "AnimGraphRuntime" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine", "Slate", "SlateCore", "Sockets", "Networking" });
		DynamicallyLoadedModuleNames.AddRange(new string[] { });

        string sPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "ThirdParty"));
        PublicIncludePaths.Add(sPath);
    }
}
