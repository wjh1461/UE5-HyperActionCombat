// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HyperActionCombatFrameworkEditor : ModuleRules
{
	public HyperActionCombatFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"HyperActionCombatFramework",
				"InputCore",
				"PropertyEditor",
				"Slate",
				"SlateCore"
			}
		);
	}
}
