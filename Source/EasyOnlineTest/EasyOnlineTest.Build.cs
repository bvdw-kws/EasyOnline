// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

using UnrealBuildTool;

public class EasyOnlineTest : ModuleRules
{
	public EasyOnlineTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"AutomationController", // Required for automation tests
			"UnrealEd",             // Required for editor functionality in tests
			"ToolMenus",			// Required for automation tests
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"EasyDataTransferModule", // Module we're testing
			"EasyOnline",            // EasyOnline module for integration tests
			"ModularGameplay",       // Required for PlayerStateComponent tests
			"NetCore",              // Required for network testing
			"Slate",                // Required for UI in tests
			"SlateCore",            // Required for UI in tests
			"EditorStyle",          // Editor styling for test UI
			"EditorWidgets",        // Editor widgets for test UI
			"GameplayTags",			// Required for gameplay tag tests
			"AITestSuite",
		});

		// Automation tests are already handled by Unreal Engine's build system
		// No need to manually define WITH_AUTOMATION_TESTS
	}
}