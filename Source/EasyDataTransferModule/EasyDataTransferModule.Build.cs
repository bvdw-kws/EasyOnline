// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

using System;
using System.IO;
using UnrealBuildTool;

public class EasyDataTransferModule : ModuleRules
{
	public EasyDataTransferModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependencies - visible to other modules
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"ModularGameplay",  // Required for UPlayerStateComponent
			"DeveloperSettings" // Required for settings system
		});
		
		// Private dependencies - internal to this module
		PrivateDependencyModuleNames.AddRange(new string[] {
			"NetCore"           // Required for networking
			// Note: FCompression is part of Core module, no separate dependency needed
		});
	}
}