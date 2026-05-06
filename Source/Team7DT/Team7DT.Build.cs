// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Team7DT : ModuleRules
{
	public Team7DT(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ChaosVehicles", "PhysicsCore", "Landscape" });
	}
}
