// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineHost.h"

#include "EasyOnlineSessionClient.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystemUtils.h"
#include "Game/EasyOnlineTypes.h"
#include "Online/OnlineSessionNames.h"
#include "Settings/EasyOnlineSettings.h"

bool UEasyOnlineHost::HostLobby(const FUniqueNetId& HostPlayerId, bool bPrivateSession, int32 NumPublicConnections)
{
	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	return HostGameMap(HostPlayerId, EasyOnlineSettings->LobbyMap.GetAssetName(), bPrivateSession, NumPublicConnections);
}

bool UEasyOnlineHost::HostGameMap(const FUniqueNetId& HostPlayerId, const FString& MapName, bool bPrivateSession, int32 NumPublicConnections)
{
	PendingHostMap.Reset();
	PendingHostMap.Emplace(MapName);

	UEasyOnlineSessionClient* OnlineSessionClient = UEasyOnlineSessionClient::GetInWorld(GetWorld());
	if (ensureAlwaysMsgf(OnlineSessionClient, TEXT("%hs: Failed to find online session client"), __FUNCTION__))
	{
		return OnlineSessionClient->CreateSession(
			NAME_GameSession,
			GenerateOnlineSessionSettings(bPrivateSession, NumPublicConnections),
			FEasyOnlineOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionToHostGameMapComplete));
	}
	else
	{
		return false;
	}
}

bool UEasyOnlineHost::HostGameInBackground(const FUniqueNetId& HostPlayerId, bool bPrivateSession, int32 NumPublicConnections)
{
	if(IsHosting())
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Cannot host multiple game"), __FUNCTION__);
		return false;
	}
	
	return UEasyOnlineSessionClient::GetInWorld(GetWorld())->CreateSession(
		NAME_GameSession,
		GenerateOnlineSessionSettings(bPrivateSession, NumPublicConnections),
		FEasyOnlineOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionToHostGameInBackgroundComplete));
}

UEasyOnlineManagerSubsystem* UEasyOnlineHost::GetOnlineManager() const
{
	return CastChecked<UEasyOnlineManagerSubsystem>(GetOuter());
}

bool UEasyOnlineHost::IsHosting() const
{
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession) == nullptr)
	{
		return false;
	}
	return GetWorld()->IsNetMode(NM_ListenServer);
}

void UEasyOnlineHost::StopHost()
{
	// Iterate kick active connections and remote players
	{
		if(const AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			const FText KickReason = NSLOCTEXT("EasyOnlineHost", "StopHost", "Host closed the connection.");
			for (FConstPlayerControllerIterator Itr = GetWorld()->GetPlayerControllerIterator(); Itr; ++Itr)
			{
				APlayerController* PlayerController = Itr->Get();
				if(IsValid(PlayerController) && PlayerController->IsLocalController() == false)
				{
					GameMode->GameSession->KickPlayer(PlayerController, KickReason);
				}
			}
		}
	}

	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if (Sessions.IsValid() && Sessions->GetNamedSession(NAME_GameSession) != nullptr)
	{
		Sessions->DestroySession(NAME_GameSession);
	}
	
	GetOnlineManager()->GetGameInstance()->EnableListenServer(false);
}

void UEasyOnlineHost::OnCreateSessionToHostGameMapComplete(const FName& SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Create session was successful"), __FUNCTION__);
		
		if (UWorld* World = GetWorld())
		{
			const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
			if(Sessions.IsValid() && PendingHostMap.IsSet())
			{
				if(const FNamedOnlineSession* OnlineSession = Sessions->GetNamedSession(SessionName))
				{
					const FString FinalTravelUrl = FString::Printf(
						TEXT("%s?listen?MaxPlayers=%d"),
						*PendingHostMap.GetValue(),
						FMath::Max(OnlineSession->SessionSettings.NumPublicConnections, OnlineSession->SessionSettings.NumPrivateConnections));
					
					// Travel to the specified match URL
					if(World->ServerTravel(FinalTravelUrl))
					{
						UE_LOG(LogEasyOnline, Log, TEXT("%hs: Travelling to %s"), __FUNCTION__, *FinalTravelUrl);
					}
					PendingHostMap.Reset();
				}
				return;
			}
		}
	}

	PendingHostMap.Reset();
	UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Failed to host game"), __FUNCTION__);
}

void UEasyOnlineHost::OnCreateSessionToHostGameInBackgroundComplete(const FName& SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Create session was successful"), __FUNCTION__);
		if(GetOnlineManager()->GetGameInstance()->EnableListenServer(true))
		{
			UE_LOG(LogEasyOnline, Log, TEXT("%hs: Started listen server!"), __FUNCTION__);
		}
		else
		{
			UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Failed to start listen server"), __FUNCTION__);
		}
	}
	else
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Failed to host game"), __FUNCTION__);
	}
}


FOnlineSessionSettings UEasyOnlineHost::GenerateOnlineSessionSettings(bool bPrivateSession, int32 NumPublicConnections)
{
	FOnlineSessionSettings SessionSettings;

	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = NumPublicConnections;
	
	if (GetOnlineManager()->IsForceLanSessionPlatform())
	{
		SessionSettings.bIsLANMatch = true;
		SessionSettings.bAllowJoinViaPresence = false;
		SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
		SessionSettings.bAllowInvites = false;
	}		
	else if (bPrivateSession)
	{
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bAllowJoinViaPresence = false;
		SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
		SessionSettings.bAllowInvites = true;
	}
	else
	{
		SessionSettings.bIsLANMatch = false;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bAllowJoinViaPresenceFriendsOnly = true;
		SessionSettings.bAllowInvites = true;
	}
	
	return SessionSettings;
}




