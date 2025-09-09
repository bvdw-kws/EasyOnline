// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineDelegateMacros.h"

#include "EasyOnlineMapSubsystem.generated.h"

class UEasyOnlineMapAsset;

DECLARE_MULTICAST_DELEGATE(FEasyOnlineOnMapAssetsLoaded);
typedef FEasyOnlineOnMapAssetsLoaded::FDelegate FEasyOnlineOnMapAssetsLoadedDelegate;

UCLASS()
class EASYONLINE_API UEasyOnlineMapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UEasyOnlineMapSubsystem& GetRef(const UWorld* World);

public:
	TObjectPtr<const UEasyOnlineMapAsset> GetMapAsset(const FName& MapID) const;
	TArray<TObjectPtr<const UEasyOnlineMapAsset>> GetAllMapAssets() const;

	bool HasLoadedMapAssets() const;
	
	DEFINE_ONLINE_DELEGATE(EasyOnlineOnMapAssetsLoaded);

	// USubsystem implementation Begin
protected:
	void Initialize(FSubsystemCollectionBase& Collection) override final;
	void Deinitialize() override final;
	// USubsystem implementation End
    
private:
	UPROPERTY(Transient)
	TArray<FSoftObjectPath> MapAssetPaths;
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<const UEasyOnlineMapAsset>> MapAssetMap;

	void OnMapAssetsLoaded();
};
