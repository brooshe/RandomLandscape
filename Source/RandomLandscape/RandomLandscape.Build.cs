// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RandomLandscape : ModuleRules
{
	public RandomLandscape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	}
}
