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
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Game/EasyOnlineTypes.h"
#include "Game/InGame/EasyOnlineGameMode_InGame.h"
#include "Game/Online/EasyOnlineHost.h"
#include "Game/Online/EasyOnlineQuickJoin.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/EasyOnlineSettings.h"

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

TArray<UEasyOnlineMapAsset*> UEasyOnlineFunctionLibrary::GetSortedMapAssetsWithTag(
	const UObject* WorldContextObject, FGameplayTag Tag)
{
	const UWorld* World = IsValid(WorldContextObject) ? WorldContextObject->GetWorld() : nullptr;
	const UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(World);
	const TArray<TObjectPtr<const UEasyOnlineMapAsset>> MapAssets = MapSubsystem.GetMapAssetsWithTag(Tag);

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

FString UEasyOnlineFunctionLibrary::GetMapURLWithExtraOptions(const UObject* WorldContextObject,
                                              const UEasyOnlineMapAsset* MapAsset, const FString& ExtraOptions)
{
	if (!ensureAlwaysMsgf(IsValid(MapAsset),
		TEXT("%hs Invalid map asset"), __FUNCTION__))
	{
		return FString();
	}

	if (!ensureAlwaysMsgf(!MapAsset->MapData.Map.IsNull(),
		TEXT("%hs Null map: %s"), __FUNCTION__, *MapAsset->GetPathName()))
	{
		return FString();
	}

	FString OptionsString;

	for (const TPair<FString, FString>& Option : MapAsset->MapData.Options)
	{
		AddOption(OptionsString, Option.Key, Option.Value);
	}

	OptionsString += ExtraOptions;

	const FString MapName = MapAsset->MapData.Map.GetAssetName();
	const FString MapURL = FString::Printf(TEXT("%s%s"), *MapName, *OptionsString);

	return MapURL;
}

FString UEasyOnlineFunctionLibrary::GetMapURL(const UObject* WorldContextObject,
                                              const UEasyOnlineMapAsset* MapAsset, const FName& GameModeID, bool bListenServer, int32 NumBots,
                                              const FString& ExtraOptions)
{
	FString OptionsString;

	if (bListenServer)
	{
		AddListenServerOption(OptionsString);
	}

	AddNumBotsOption(OptionsString, NumBots);
	AddGameModeOption(WorldContextObject, OptionsString, GameModeID);

	OptionsString += ExtraOptions;

	return GetMapURLWithExtraOptions(WorldContextObject, MapAsset, OptionsString);
}

void UEasyOnlineFunctionLibrary::OpenMapWithExtraOptions(const UObject* WorldContextObject,
	const UEasyOnlineMapAsset* MapAsset, const FString& ExtraOptions)
{
	const FString MapURL = GetMapURLWithExtraOptions(WorldContextObject, MapAsset, ExtraOptions);
	if (!ensureAlwaysMsgf(!MapURL.IsEmpty(),
		TEXT("%hs Invalid map: %s"), __FUNCTION__, MapAsset ? *MapAsset->GetPathName() : TEXT("None")))
	{
		return;
	}

	UGameplayStatics::OpenLevel(WorldContextObject, FName(*MapURL));
}

void UEasyOnlineFunctionLibrary::OpenMap(const UObject* WorldContextObject,
	const UEasyOnlineMapAsset* MapAsset, const FName& GameModeID, bool bListenServer, int32 NumBots,
	const FString& ExtraOptions)
{
	const FString MapURL = GetMapURL(WorldContextObject, MapAsset, GameModeID, bListenServer, NumBots, ExtraOptions);
	if (!ensureAlwaysMsgf(!MapURL.IsEmpty(),
		TEXT("%hs Invalid map: %s"), __FUNCTION__, MapAsset ? *MapAsset->GetPathName() : TEXT("None")))
	{
		return;
	}

	UGameplayStatics::OpenLevel(WorldContextObject, FName(*MapURL));
}

void UEasyOnlineFunctionLibrary::AddOption(FString& Options, const FString& Key, const FString& Value)
{
	Options += FString::Printf(TEXT("?%s=%s"), *Key, *Value);
}

void UEasyOnlineFunctionLibrary::AddBoolOption(FString& Options, const FString& Key, bool bValue)
{
	AddOption(Options, Key, bValue ? TEXT("true") : TEXT("false"));
}

void UEasyOnlineFunctionLibrary::AddFlagOption(FString& Options, const FString& Flag)
{
	Options += FString::Printf(TEXT("?%s"), *Flag);
}

void UEasyOnlineFunctionLibrary::AddListenServerOption(FString& Options)
{
	AddFlagOption(Options, TEXT("listen"));
}

void UEasyOnlineFunctionLibrary::AddNumBotsOption(FString& Options, int32 NumBots)
{
	AddOption(Options, TEXT("NumBots"), FString::FromInt(NumBots));
}

void UEasyOnlineFunctionLibrary::AddGameModeOption(const UObject* WorldContextObject, FString& Options, FName GameModeID)
{
	const TSoftClassPtr<AEasyOnlineGameMode_InGame> GameModeClass = GetGameModeSoftClass(WorldContextObject, GameModeID);
	if (!GameModeClass.IsNull())
	{
		AddOption(Options, TEXT("game"), GameModeClass->GetPathName());
	}
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
