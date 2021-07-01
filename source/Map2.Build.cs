// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Map2 : ModuleRules
{
	public Map2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysXVehicles", "HeadMountedDisplay", "PhysicsCore" });

		PublicDefinitions.Add("HMD_MODULE_INCLUDED=1");
	}
}
