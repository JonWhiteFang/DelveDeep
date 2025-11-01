// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DelveDeep : ModuleRules
{
	public DelveDeep(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Paper2D",
			"UMG",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",  // For hot reload support
			"Json",           // For test data loading
			"JsonUtilities"  // For JSON serialization
		});

		// Add Private/Tests to include paths for test utilities
		PrivateIncludePaths.AddRange(new string[]
		{
			"DelveDeep/Private/Tests"
		});
	}
}
