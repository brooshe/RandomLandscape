// Copyright 2021 IOLACORP STUDIO. All Rights Reserved


using System.IO;				
using UnrealBuildTool;

public class WorldScapeVolume : ModuleRules
{
	public WorldScapeVolume(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

#if UE_4_24_OR_LATER
        bUseUnity = false;
#else
        bFasterWithoutUnity = true;
#endif
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        // For raytracing
        PrivateIncludePaths.Add(EngineDirectory + "/Shaders/Shared");
        // For HLSL translator
        PrivateIncludePaths.Add(EngineDirectory + "/Source/Runtime/Engine/Private");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine"
    }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
             {
                "Core",
                "WorldScapeNoise",
                "WorldScapeCommon",
            });


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
