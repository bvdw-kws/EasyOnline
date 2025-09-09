// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineGameModeSubsystem.h"

#include "Data/GameMode/EasyOnlineGameModeAsset.h"
#include "Engine/AssetManager.h"
#include "Game/EasyOnlineTypes.h"
#include "UObject/CoreRedirects.h"

void UEasyOnlineGameModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
	{
		TArray<FSoftObjectPath> AssetPaths;
		AssetManager->GetPrimaryAssetPathList(UEasyOnlineGameModeAsset::AssetType, AssetPaths);
		
		for (const FSoftObjectPath& AssetPath : AssetPaths)
		{
			//.pak files containing the GameMode may not be mounted. If so, ignore them.
			const FCoreRedirectObjectName RedirectedName =
				FCoreRedirects::GetRedirectedName(
					ECoreRedirectFlags::Type_Package,
					FCoreRedirectObjectName(AssetPath.GetLongPackageName()));
			
			FString LocalizedName;
			LocalizedName = FPackageName::GetDelegateResolvedPackagePath(RedirectedName.PackageName.ToString());
			LocalizedName = FPackageName::GetLocalizedPackagePath(LocalizedName);
			
			if (FPackageName::DoesPackageExist(LocalizedName))
			{
				GameModeAssetPaths.Emplace(AssetPath);
			}
		}

		AssetManager->LoadAssetList(GameModeAssetPaths, FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameModeAssetsLoaded));
	}
}

void UEasyOnlineGameModeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UEasyOnlineGameModeSubsystem::OnGameModeAssetsLoaded()
{
	GameModeAssetGameMode.Reserve(GameModeAssetPaths.Num());
	
	for (const FSoftObjectPath& GameModeAssetPath : GameModeAssetPaths)
	{
		const UEasyOnlineGameModeAsset* GameModeAsset = Cast<UEasyOnlineGameModeAsset>(GameModeAssetPath.ResolveObject());
		if (!ensureAlwaysMsgf(IsValid(GameModeAsset),
			TEXT("%hs Failed to load GameMode asset: %s"), __FUNCTION__, *GameModeAssetPath.ToString()))
		{
			continue;
		}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		if (GameModeAsset->GameModeID.IsNone())
		{
			UE_LOG(LogEasyOnline, Warning, TEXT("%hs GameModeID is not set: %s"),
				__FUNCTION__, *GameModeAssetPath.ToString())
		}
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		
		GameModeAssetGameMode.Add(GameModeAsset->GameModeID, GameModeAsset);
	}

	TriggerEasyOnlineOnGameModeAssetsLoadedDelegates();
}

UEasyOnlineGameModeSubsystem& UEasyOnlineGameModeSubsystem::GetRef(const UWorld* World)
{
	check(World);
	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);
	UEasyOnlineGameModeSubsystem* GameModeSubsystem = GameInstance->GetSubsystem<UEasyOnlineGameModeSubsystem>();
	check(GameModeSubsystem);
	return *GameModeSubsystem;
}

TObjectPtr<const UEasyOnlineGameModeAsset> UEasyOnlineGameModeSubsystem::GetGameModeAsset(const FName& GameModeID) const
{
	if (const TObjectPtr<const UEasyOnlineGameModeAsset>* GameModeAssetPtr = GameModeAssetGameMode.Find(GameModeID))
	{
		return *GameModeAssetPtr;
	}
	return nullptr;
}

TArray<TObjectPtr<const UEasyOnlineGameModeAsset>> UEasyOnlineGameModeSubsystem::GetAllGameModeAssets() const
{
	TArray<TObjectPtr<const UEasyOnlineGameModeAsset>> Results;
	GameModeAssetGameMode.GenerateValueArray(Results);	
	return Results;
}

bool UEasyOnlineGameModeSubsystem::HasLoadedGameModeAssets() const
{
	return GameModeAssetPaths.Num() == GameModeAssetGameMode.Num();
}
