// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineMapSubsystem.h"

#include "Data/Map/EasyOnlineMapAsset.h"
#include "Engine/AssetManager.h"
#include "Game/EasyOnlineTypes.h"
#include "UObject/CoreRedirects.h"

void UEasyOnlineMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		TArray<FSoftObjectPath> AssetPaths;
		AssetManager->GetPrimaryAssetPathList(UEasyOnlineMapAsset::AssetType, AssetPaths);
		
		for (const FSoftObjectPath& AssetPath : AssetPaths)
		{
			//.pak files containing the map may not be mounted. If so, ignore them.
			const FCoreRedirectObjectName RedirectedName =
				FCoreRedirects::GetRedirectedName(
					ECoreRedirectFlags::Type_Package,
					FCoreRedirectObjectName(AssetPath.GetLongPackageName()));
			
			FString LocalizedName;
			LocalizedName = FPackageName::GetDelegateResolvedPackagePath(RedirectedName.PackageName.ToString());
			LocalizedName = FPackageName::GetLocalizedPackagePath(LocalizedName);
			
			if (FPackageName::DoesPackageExist(LocalizedName))
			{
				MapAssetPaths.Emplace(AssetPath);
			}
		}

		AssetManager->LoadAssetList(MapAssetPaths, FStreamableDelegate::CreateUObject(this, &ThisClass::OnMapAssetsLoaded));
	}
}

void UEasyOnlineMapSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UEasyOnlineMapSubsystem::OnMapAssetsLoaded()
{
	MapAssetMap.Reserve(MapAssetPaths.Num());
	
	for (const FSoftObjectPath& MapAssetPath : MapAssetPaths)
	{
		const UEasyOnlineMapAsset* MapAsset = Cast<UEasyOnlineMapAsset>(MapAssetPath.ResolveObject());
		if (!ensureAlwaysMsgf(IsValid(MapAsset),
			TEXT("%hs Failed to load Map asset: %s"), __FUNCTION__, *MapAssetPath.ToString()))
		{
			continue;
		}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		if (MapAsset->MapID.IsNone())
		{
			UE_LOG(LogEasyOnline, Warning, TEXT("%hs GameModeID is not set: %s"),
				__FUNCTION__, *MapAssetPath.ToString())
		}
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		
		MapAssetMap.Add(MapAsset->MapID, MapAsset);
	}

	TriggerEasyOnlineOnMapAssetsLoadedDelegates();
}

UEasyOnlineMapSubsystem& UEasyOnlineMapSubsystem::GetRef(const UWorld* World)
{
	check(World);
	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);
	UEasyOnlineMapSubsystem* MapSubsystem = GameInstance->GetSubsystem<UEasyOnlineMapSubsystem>();
	check(MapSubsystem);
	return *MapSubsystem;
}

TObjectPtr<const UEasyOnlineMapAsset> UEasyOnlineMapSubsystem::GetMapAsset(const FName& MapID) const
{
	if (const TObjectPtr<const UEasyOnlineMapAsset>* MapAssetPtr = MapAssetMap.Find(MapID))
	{
		return *MapAssetPtr;
	}
	return nullptr;
}

TArray<TObjectPtr<const UEasyOnlineMapAsset>> UEasyOnlineMapSubsystem::GetAllMapAssets() const
{
	TArray<TObjectPtr<const UEasyOnlineMapAsset>> Results;
	MapAssetMap.GenerateValueArray(Results);	
	return Results;
}

TArray<TObjectPtr<const UEasyOnlineMapAsset>> UEasyOnlineMapSubsystem::GetMapAssetsWithTag(FGameplayTag Tag) const
{
	TArray<TObjectPtr<const UEasyOnlineMapAsset>> Results;
	for (const TPair<FName, TObjectPtr<const UEasyOnlineMapAsset>>& MapAssetPair : MapAssetMap)
	{
		if (const UEasyOnlineMapAsset* MapAsset = MapAssetPair.Value)
		{
			if (MapAsset->MapTags.HasTag(Tag))
			{
				Results.Add(MapAssetPair.Value);
			}
		}
	}
	return Results;
}

bool UEasyOnlineMapSubsystem::HasLoadedMapAssets() const
{
	return MapAssetPaths.Num() == MapAssetMap.Num();
}
