// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/EasyOnlineFunctionLibrary.h"

#include "Data/GameMode/EasyOnlineGameModeAsset.h"
#include "Data/Map/EasyOnlineMapAsset.h"
#include "Data/Subsystems/EasyOnlineGameModeSubsystem.h"
#include "Data/Subsystems/EasyOnlineMapSubsystem.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Game/InGame/EasyOnlineGameMode_InGame.h"
#include "Game/Online/EasyOnlineHost.h"
#include "Game/Online/EasyOnlineQuickJoin.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/EasyOnlineSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Game/EasyOnlineTypes.h"

FName UEasyOnlineFunctionLibrary::GetDefaultMapID(const UObject* WorldContextObject)
{
	const TArray<UEasyOnlineMapAsset*> MapAssets = GetSortedMapAssets(WorldContextObject);

	if (MapAssets.Num())
	{
		if (const UEasyOnlineMapAsset* MapAsset = MapAssets[0])
		{
			return MapAsset->MapID;
		}
	}

	return NAME_None;
}

TArray<UEasyOnlineMapAsset*> UEasyOnlineFunctionLibrary::GetSortedMapAssets(
	const UObject* WorldContextObject)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	const UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(World);
	const TArray<TObjectPtr<const UEasyOnlineMapAsset>> MapAssets = MapSubsystem.GetAllMapAssets();

	TArray<UEasyOnlineMapAsset*> Results;
	Algo::Transform(MapAssets, Results, [](const TObjectPtr<const UEasyOnlineMapAsset>& MapAsset)
	{
		return const_cast<UEasyOnlineMapAsset*>(MapAsset.Get());
	});
	
	Results.Sort([](
		const UEasyOnlineMapAsset& lMapAsset, const UEasyOnlineMapAsset& rMapAsset)
	{
		return lMapAsset.MenuSortOrder >= rMapAsset.MenuSortOrder;
	});

	return Results;
}

UEasyOnlineMapAsset* UEasyOnlineFunctionLibrary::GetMapAsset(const UObject* WorldContextObject, FName MapID)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return nullptr;
	}
	
	const UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(World);	
	const TObjectPtr<const UEasyOnlineMapAsset> MapAsset = MapSubsystem.GetMapAsset(MapID);
	return const_cast<UEasyOnlineMapAsset*>(MapAsset.Get());
}

FName UEasyOnlineFunctionLibrary::GetDefaultGameModeID(const UObject* WorldContextObject)
{
	const TArray<UEasyOnlineGameModeAsset*> GameModeAssets = GetSortedGameModeAssets(WorldContextObject);

	if (GameModeAssets.Num())
	{
		if (const UEasyOnlineGameModeAsset* GameModeAsset = GameModeAssets[0])
		{
			return GameModeAsset->GameModeID;
		}
	}

	return NAME_None;
}

TArray<UEasyOnlineGameModeAsset*> UEasyOnlineFunctionLibrary::GetSortedGameModeAssets(const UObject* WorldContextObject)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	const UEasyOnlineGameModeSubsystem& GameModeSubsystem = UEasyOnlineGameModeSubsystem::GetRef(World);
	const TArray<TObjectPtr<const UEasyOnlineGameModeAsset>> GameModeAssets = GameModeSubsystem.GetAllGameModeAssets();

	TArray<UEasyOnlineGameModeAsset*> Results;
	Algo::Transform(GameModeAssets, Results, [](const TObjectPtr<const UEasyOnlineGameModeAsset>& GameModeAsset)
	{
		return const_cast<UEasyOnlineGameModeAsset*>(GameModeAsset.Get());
	});
	
	Results.Sort([](
		const UEasyOnlineGameModeAsset& lGameModeAsset, const UEasyOnlineGameModeAsset& rGameModeAsset)
	{
		return lGameModeAsset.MenuSortOrder >= rGameModeAsset.MenuSortOrder;
	});

	return Results;
}

UEasyOnlineGameModeAsset* UEasyOnlineFunctionLibrary::GetGameModeAsset(const UObject* WorldContextObject, FName GameModeID)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return nullptr;
	}
	
	const UEasyOnlineGameModeSubsystem& GameModeSubsystem = UEasyOnlineGameModeSubsystem::GetRef(World);	
	const TObjectPtr<const UEasyOnlineGameModeAsset> GameModeAsset = GameModeSubsystem.GetGameModeAsset(GameModeID);
	return const_cast<UEasyOnlineGameModeAsset*>(GameModeAsset.Get());
}

TSoftClassPtr<AEasyOnlineGameMode_InGame> UEasyOnlineFunctionLibrary::GetGameModeSoftClass(const UObject* WorldContextObject,
	FName GameModeID)
{
	if (const UEasyOnlineGameModeAsset* GameModeAsset = GetGameModeAsset(WorldContextObject, GameModeID))
	{
		return GameModeAsset->GameModeClass;
	}
	return nullptr;
}

FString UEasyOnlineFunctionLibrary::GetMapURL(const UObject* WorldContextObject,
                                              const FName& MapID, const FName& GameModeID, bool bListenServer, int32 NumBots)
{
	const UEasyOnlineMapAsset* MapAsset = GetMapAsset(WorldContextObject, MapID);
	if (!ensureAlwaysMsgf(IsValid(MapAsset),
		TEXT("%hs Invalid map asset: %s"), __FUNCTION__, *MapID.ToString()))
	{
		return FString();
	}
	
	if (!ensureAlwaysMsgf(!MapAsset->MapData.Map.IsNull(),
		TEXT("%hs Null map: %s"), __FUNCTION__, *MapAsset->GetPathName()))
	{
		return FString();
	}

	FString OptionsString;

	if (bListenServer)
	{
		OptionsString += FString::Printf(TEXT("?listen"));
	}

	OptionsString += FString::Printf(TEXT("?NumBots=%d"), NumBots);

	for (const TPair<FString, FString>& Option : MapAsset->MapData.Options)
	{
		OptionsString += FString::Printf(TEXT("?%s=%s"), *Option.Key, *Option.Value);
	}

	const TSoftClassPtr<AEasyOnlineGameMode_InGame> GameModeClass = GetGameModeSoftClass(WorldContextObject, GameModeID);
	if (!GameModeClass.IsNull())
	{
		OptionsString += FString::Printf(TEXT("?game=%s"), *GameModeClass->GetPathName());
	}
	
	const FString MapName = MapAsset->MapData.Map.GetAssetName();
	const FString MapURL = FString::Printf(TEXT("%s%s"), *MapName, *OptionsString);

	return MapURL;
}

void UEasyOnlineFunctionLibrary::CreateLobby(const UObject* WorldContextObject,
	APlayerController* HostingPlayer, bool bPrivateSession)
{
	if (!ensureAlwaysMsgf(IsValid(HostingPlayer) && HostingPlayer->IsLocalPlayerController(),
		TEXT("%hs Invalid HostingPlayer"), __FUNCTION__))
	{
		return;
	}
	
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!ensureAlwaysMsgf(IsValid(GameInstance),
		TEXT("%hs Invalid GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	const UEasyOnlineManagerSubsystem* OnlineManager = GameInstance->GetSubsystem<UEasyOnlineManagerSubsystem>();
	if (!ensureAlwaysMsgf(IsValid(OnlineManager),
			TEXT("%hs Invalid UEasyOnlineManagerSubsystem, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}

	UEasyOnlineHost* HostManager = OnlineManager->GetHostManager();
	if (!ensureAlwaysMsgf(IsValid(HostManager),
			TEXT("%hs Invalid UEasyOnlineHost, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	HostManager->HostLobby(
		*HostingPlayer->GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId(),
		bPrivateSession, EasyOnlineSettings->NumPublicConnections);
}

void UEasyOnlineFunctionLibrary::QuickJoin(const UObject* WorldContextObject, APlayerController* JoiningPlayer)
{
	if (!ensureAlwaysMsgf(IsValid(JoiningPlayer) && JoiningPlayer->IsLocalPlayerController(),
		TEXT("%hs Invalid JoiningPlayer"), __FUNCTION__))
	{
		return;
	}
	
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!ensureAlwaysMsgf(IsValid(GameInstance),
		TEXT("%hs Invalid GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	const UEasyOnlineManagerSubsystem* OnlineManager = GameInstance->GetSubsystem<UEasyOnlineManagerSubsystem>();
	if (!ensureAlwaysMsgf(IsValid(OnlineManager),
			TEXT("%hs Invalid UEasyOnlineManagerSubsystem, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}

	UEasyOnlineQuickJoin* QuickJoin = OnlineManager->GetQuickJoin();
	if (!ensureAlwaysMsgf(IsValid(QuickJoin),
			TEXT("%hs Invalid UEasyOnlineQuickJoin, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	if (QuickJoin->QuickJoinSession(JoiningPlayer->GetLocalPlayer()))
	{
		// TODO: Open dialog window while joining session
	}
}

void UEasyOnlineFunctionLibrary::QuickHost(const UObject* WorldContextObject, APlayerController* HostingPlayer,
	bool bPrivateSession)
{
	if (!ensureAlwaysMsgf(IsValid(HostingPlayer) && HostingPlayer->IsLocalPlayerController(),
		TEXT("%hs Invalid HostingPlayer"), __FUNCTION__))
	{
		return;
	}
	
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!ensureAlwaysMsgf(IsValid(GameInstance),
		TEXT("%hs Invalid GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	const UEasyOnlineManagerSubsystem* OnlineManager = GameInstance->GetSubsystem<UEasyOnlineManagerSubsystem>();
	if (!ensureAlwaysMsgf(IsValid(OnlineManager),
			TEXT("%hs Invalid UEasyOnlineManagerSubsystem, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}

	UEasyOnlineHost* HostManager = OnlineManager->GetHostManager();
	if (!ensureAlwaysMsgf(IsValid(HostManager),
			TEXT("%hs Invalid UEasyOnlineHost, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	if (HostManager->HostGameMap(
		*HostingPlayer->GetLocalPlayer()->GetPreferredUniqueNetId().GetUniqueNetId(),
		EasyOnlineSettings->QuickHostMap.GetAssetName(), bPrivateSession, EasyOnlineSettings->NumPublicConnections))
	{
		// TODO: Open dialog window (and register to delegates)
	}
}

bool UEasyOnlineFunctionLibrary::IsSessionActiveOrPending()
{
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			if (FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession))
			{
				EOnlineSessionState::Type SessionState = ExistingSession->SessionState;
				if (SessionState == EOnlineSessionState::InProgress)
				{
					UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Already connected to a session (State: %s)"), 
						__FUNCTION__, EOnlineSessionState::ToString(SessionState));
					return true;
				}
				else if (SessionState == EOnlineSessionState::Pending || SessionState == EOnlineSessionState::Starting)
				{
					UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Session join already in progress (State: %s)"), 
						__FUNCTION__, EOnlineSessionState::ToString(SessionState));
					return true;
				}
			}
		}
	}
	return false;
}
