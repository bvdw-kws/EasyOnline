// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineLobbyModeComponent.h"

#include "Data/Subsystems/EasyOnlineGameModeSubsystem.h"
#include "Data/Subsystems/EasyOnlineMapSubsystem.h"
#include "Engine/AssetManager.h"
#include "Game/InGame/EasyOnlineGameMode_InGame.h"
#include "Net/UnrealNetwork.h"
#include "Utility/EasyOnlineFunctionLibrary.h"

UEasyOnlineLobbyModeComponent::UEasyOnlineLobbyModeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UEasyOnlineLobbyModeComponent::BeginPlay()
{
	Super::BeginPlay();
		
#if WITH_SERVER_CODE
	UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(GetWorld());
	if (MapSubsystem.HasLoadedMapAssets())
	{
		ResetDefaultMapID();
	}
	else
	{
		OnMapAssetsLoadedDelegateHandle = MapSubsystem.AddEasyOnlineOnMapAssetsLoadedDelegate_Handle(
			FEasyOnlineOnMapAssetsLoadedDelegate::CreateUObject(this, &ThisClass::ResetDefaultMapID));
	}
	
	UEasyOnlineGameModeSubsystem& GameModeSubsystem = UEasyOnlineGameModeSubsystem::GetRef(GetWorld());
	if (GameModeSubsystem.HasLoadedGameModeAssets())
	{
		ResetDefaultGameModeID();
	}
	else
	{
		OnGameModeAssetsLoadedDelegateHandle = GameModeSubsystem.AddEasyOnlineOnGameModeAssetsLoadedDelegate_Handle(
			FEasyOnlineOnGameModeAssetsLoadedDelegate::CreateUObject(this, &ThisClass::ResetDefaultGameModeID));
	}
#endif // WITH_SERVER_CODE
}

void UEasyOnlineLobbyModeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UEasyOnlineLobbyModeComponent, MapID);
	DOREPLIFETIME(UEasyOnlineLobbyModeComponent, GameModeID);
};

bool UEasyOnlineLobbyModeComponent::IsModeReady() const
{
	if (MapID.IsNone() || !bIsGameModeReady)
	{
		return false;
	}

	return true;
}

void UEasyOnlineLobbyModeComponent::ResetDefaultMapID()
{
#if WITH_SERVER_CODE
	UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(GetWorld());
	
	if (OnMapAssetsLoadedDelegateHandle.IsValid())
	{
		MapSubsystem.ClearEasyOnlineOnMapAssetsLoadedDelegate_Handle(OnMapAssetsLoadedDelegateHandle);
	}
	
	SetMapID(UEasyOnlineFunctionLibrary::GetDefaultMapID(this));
#endif // WITH_SERVER_CODE
}

void UEasyOnlineLobbyModeComponent::ResetDefaultGameModeID()
{
#if WITH_SERVER_CODE
	UEasyOnlineGameModeSubsystem& GameModeSubsystem = UEasyOnlineGameModeSubsystem::GetRef(GetWorld());
	
	if (OnGameModeAssetsLoadedDelegateHandle.IsValid())
	{
		GameModeSubsystem.ClearEasyOnlineOnGameModeAssetsLoadedDelegate_Handle(OnGameModeAssetsLoadedDelegateHandle);
	}
	
	SetGameModeID(UEasyOnlineFunctionLibrary::GetDefaultGameModeID(this));
#endif // WITH_SERVER_CODE
}

void UEasyOnlineLobbyModeComponent::SetMapID(const FName& NewMapID)
{
#if WITH_SERVER_CODE
	if (!HasAuthority() || MapID == NewMapID)
	{
		return;
	}
	
	MapID = NewMapID;
	OnRep_MapID();
#endif // WITH_SERVER_CODE
}

void UEasyOnlineLobbyModeComponent::OnRep_MapID()
{
	TriggerEasyOnlineOnMapChangedDelegates(MapID);
}

void UEasyOnlineLobbyModeComponent::SetGameModeID(const FName& NewGameModeID)
{
#if WITH_SERVER_CODE
	if (!HasAuthority() || GameModeID == NewGameModeID)
	{
		return;
	}
	
	GameModeID = NewGameModeID;
	OnRep_GameModeID();
#endif // WITH_SERVER_CODE
}

void UEasyOnlineLobbyModeComponent::OnRep_GameModeID()
{
	bIsGameModeReady = false;
	
	GameModeSoftClass = UEasyOnlineFunctionLibrary::GetGameModeSoftClass(this, GameModeID);
	if (GameModeSoftClass.IsNull())
	{
		ApplyGameMode(nullptr);
		return;
	}

	TArray<FSoftObjectPath> AssetList;
	AssetList.Add(GameModeSoftClass.ToSoftObjectPath());
	
	UAssetManager& AssetManager = UAssetManager::Get();
	AssetManager.LoadAssetList(AssetList, FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameModeLoaded));
	
}

void UEasyOnlineLobbyModeComponent::OnGameModeLoaded()
{
	ApplyGameMode(GameModeSoftClass.Get());
}

void UEasyOnlineLobbyModeComponent::ApplyGameMode(const TSubclassOf<AEasyOnlineGameMode_InGame>& NewGameModeClass)
{
	if (GameModeClass == NewGameModeClass)
	{
		return;
	}
	
	GameModeClass = NewGameModeClass;
	if (GameModeClass)
	{
		bIsGameModeReady = true;
	}
	
	TriggerEasyOnlineOnGameModeChangedDelegates(GameModeID);
}

int32 UEasyOnlineLobbyModeComponent::GetModeMaxPlayingPlayers() const
{
	if (GameModeClass)
	{
		const AEasyOnlineGameMode_InGame* GameModeCDO = GameModeClass->GetDefaultObject<AEasyOnlineGameMode_InGame>();
		if (IsValid(GameModeCDO))
		{
			return GameModeCDO->GetMaxPlayingPlayers();
		}		
	}

	return 1;
}
