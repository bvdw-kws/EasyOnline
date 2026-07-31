// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

using UnrealBuildTool;

public class EasyGauntletTest : ModuleRules
{
	public EasyGauntletTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Default;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Gauntlet",
			"EasyOnline",
			"EasyDataTransferModule"
		});

		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PublicDependencyModuleNames.Add("AutomationController");
		}

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Json",
			"JsonUtilities"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"EditorStyle",
				"EditorWidgets"
			});
		}
	}
}