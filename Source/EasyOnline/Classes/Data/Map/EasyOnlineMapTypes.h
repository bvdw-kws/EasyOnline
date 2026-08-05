// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "NativeGameplayTags.h"

// Sample tags for UEasyOnlineMapAsset::MapTags, so the plugin is usable/testable standalone.
// Games built on EasyOnline should declare their own taxonomy under "Map" (e.g. Map.Mode.*).
EASYONLINE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Map_Sample_Small)
EASYONLINE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Map_Sample_Large)
