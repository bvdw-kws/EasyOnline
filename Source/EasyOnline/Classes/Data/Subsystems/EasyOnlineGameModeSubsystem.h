// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineDelegateMacros.h"

#include "EasyOnlineGameModeSubsystem.generated.h"

class UEasyOnlineGameModeAsset;

DECLARE_MULTICAST_DELEGATE(FEasyOnlineOnGameModeAssetsLoaded);
typedef FEasyOnlineOnGameModeAssetsLoaded::FDelegate FEasyOnlineOnGameModeAssetsLoadedDelegate;

UCLASS()
class EASYONLINE_API UEasyOnlineGameModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UEasyOnlineGameModeSubsystem& GetRef(const UWorld* World);

public:
	TObjectPtr<const UEasyOnlineGameModeAsset> GetGameModeAsset(const FName& GameModeID) const;
	TArray<TObjectPtr<const UEasyOnlineGameModeAsset>> GetAllGameModeAssets() const;

	bool HasLoadedGameModeAssets() const;
	
	DEFINE_ONLINE_DELEGATE(EasyOnlineOnGameModeAssetsLoaded);

	// USubsystem implementation Begin
protected:
	void Initialize(FSubsystemCollectionBase& Collection) override final;
	void Deinitialize() override final;
	// USubsystem implementation End
    
private:
	UPROPERTY(Transient)
	TArray<FSoftObjectPath> GameModeAssetPaths;
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<const UEasyOnlineGameModeAsset>> GameModeAssetGameMode;

	void OnGameModeAssetsLoaded();
};
