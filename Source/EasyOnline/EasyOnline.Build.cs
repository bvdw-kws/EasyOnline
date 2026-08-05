// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

using System;
using System.IO;
using UnrealBuildTool;

public class EasyOnline : ModuleRules
{
	public EasyOnline(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Engine",
			"CommonGame",
			"ModularGameplay",
			"EasyDataTransferModule", // Add EasyDataTransferModule dependency
			"GameplayTags",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"CommonUI",
			"ExtendedCommonUI",
			"EnhancedInput",
			"DeveloperSettings",
			"AIModule",
		});
		
		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
	}
}