// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/DataAsset.h"

#include "EasyOnlineMapAsset.generated.h"

USTRUCT(BlueprintType)
struct EASYONLINE_API FEasyOnlineMapData
{
	GENERATED_BODY()

	/**
	 * Reference the asset of the map to load.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Map;

	/**
	 * Localized name of the map.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
	
	/**
	 * Localized description of the map.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayDescription;
	
	/**
	 * Smaller size texture to preview this map.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> PreviewTexture;

	/**
	 * Main texture to display the map in the menu.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> MainTexture;

	/*
	 * List of assets to preload before traveling to the map.
	 * Doing so can greatly decrease the load time.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(RowType="/Script/EasyOnline.EasyOnlineMapPreloadRow"))
	TSoftObjectPtr<UDataTable> PreloadAsset;
	
	/**
	 * Options to be passed when opening this map, appended to the travel URL as "?Key=Value"
	 * (e.g. Key="GameEditorMap", Value="Puzzle_001" becomes "?GameEditorMap=Puzzle_001").
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FString, FString> Options;
};

UCLASS()
class EASYONLINE_API UEasyOnlineMapAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Unique identifier that can be used to reference this map.
	 * This identifier could be used to unlock maps or during matchmaking.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MapID = NAME_None;
	
	/**
	 * Priority while sorting maps in the menu.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MenuSortOrder = 0;

	/**
	 * Data describing this map.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FEasyOnlineMapData MapData;
	
public:
	static const FPrimaryAssetType AssetType;

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
};
