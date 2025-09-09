// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/DataTable.h"

#include "EasyOnlineMapPreloadRow.generated.h"

USTRUCT(BlueprintType)
struct EASYONLINE_API FEasyOnlineMapPreloadRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * Asset that should be preloaded before entering the map so the game can load faster.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UObject> Asset;
};
