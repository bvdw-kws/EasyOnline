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
	// Retrieve competitive multiplayer settings optimized for deterministic gameplay
	const UEasyOnlineSettings* CompetitiveOnlineSettings = GetDefault<UEasyOnlineSettings>();
	
	// Delegate to map hosting with lobby-specific configuration for competitive player gathering
	return HostGameMap(HostPlayerId, CompetitiveOnlineSettings->LobbyMap.GetAssetName(), bPrivateSession, NumPublicConnections);
}

bool UEasyOnlineHost::HostGameMap(const FUniqueNetId& HostPlayerId, const FString& MapName, bool bPrivateSession, int32 NumPublicConnections)
{
	// Clear any previous pending map configuration for clean state initialization
	PendingHostMap.Reset();
	PendingHostMap.Emplace(MapName);

	// Access deterministic session client optimized for competitive multiplayer protocols
	UEasyOnlineSessionClient* CompetitiveSessionClient = UEasyOnlineSessionClient::GetInWorld(GetWorld());
	if (ensureAlwaysMsgf(CompetitiveSessionClient, TEXT("%hs: Failed to find competitive session client"), __FUNCTION__))
	{
		// Create competitive session with optimized settings for rollback netcode and frame synchronization
		return CompetitiveSessionClient->CreateSession(
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
	// Prevent concurrent hosting for competitive multiplayer integrity
	if(IsHosting())
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Cannot host multiple competitive sessions simultaneously"), __FUNCTION__);
		return false;
	}
	
	// Create background session for continuous competitive server availability with seamless hosting
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
	// Check for active competitive multiplayer session availability
	const IOnlineSessionPtr CompetitiveSessions = Online::GetSessionInterface(GetWorld());
	if (CompetitiveSessions.IsValid() && CompetitiveSessions->GetNamedSession(NAME_GameSession) == nullptr)
	{
		return false;
	}
	
	// Verify listen server mode for deterministic hosting status in competitive environment
	return GetWorld()->IsNetMode(NM_ListenServer);
}

void UEasyOnlineHost::StopHost()
{
	// Gracefully disconnect all remote players with competitive-appropriate messaging
	{
		if(const AGameModeBase* CompetitiveGameMode = GetWorld()->GetAuthGameMode())
		{
			const FText CompetitiveKickReason = NSLOCTEXT("EasyOnlineHost", "StopHost", "Competitive session host terminated connection.");
			
			// Iterate through all connected players for clean competitive disconnect
			for (FConstPlayerControllerIterator PlayerIterator = GetWorld()->GetPlayerControllerIterator(); PlayerIterator; ++PlayerIterator)
			{
				APlayerController* ConnectedPlayer = PlayerIterator->Get();
				if(IsValid(ConnectedPlayer) && ConnectedPlayer->IsLocalController() == false)
				{
					// Kick remote player with appropriate competitive session termination message
					CompetitiveGameMode->GameSession->KickPlayer(ConnectedPlayer, CompetitiveKickReason);
				}
			}
		}
	}

	// Destroy competitive multiplayer session with proper cleanup
	const IOnlineSessionPtr CompetitiveSessions = Online::GetSessionInterface(GetWorld());
	if (CompetitiveSessions.IsValid() && CompetitiveSessions->GetNamedSession(NAME_GameSession) != nullptr)
	{
		CompetitiveSessions->DestroySession(NAME_GameSession);
	}
	
	// Disable listen server mode for complete competitive hosting shutdown
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
	// Create competitive multiplayer session configuration optimized for deterministic gameplay
	FOnlineSessionSettings CompetitiveSessionSettings;

	// Configure competitive session parameters for optimal multiplayer experience
	CompetitiveSessionSettings.bAllowJoinInProgress = true;  // Enable spectator joining during competitive matches
	CompetitiveSessionSettings.bIsDedicated = false;         // Use listen server for reduced latency
	CompetitiveSessionSettings.bUsesPresence = true;         // Enable friend discovery for competitive teams
	CompetitiveSessionSettings.bUseLobbiesIfAvailable = true;              // Utilize platform lobbies for enhanced matchmaking
	CompetitiveSessionSettings.bUseLobbiesVoiceChatIfAvailable = true;     // Enable competitive team communication
	CompetitiveSessionSettings.bShouldAdvertise = true;                    // Make session discoverable for matchmaking
	CompetitiveSessionSettings.NumPublicConnections = NumPublicConnections; // Configure player capacity for competitive matches
	
	// Apply platform-specific networking optimizations for competitive gameplay
	if (GetOnlineManager()->IsForceLanSessionPlatform())
	{
		// Tournament/development environment configuration for maximum reliability
		CompetitiveSessionSettings.bIsLANMatch = true;
		CompetitiveSessionSettings.bAllowJoinViaPresence = false;
		CompetitiveSessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
		CompetitiveSessionSettings.bAllowInvites = false;
	}		
	else if (bPrivateSession)
	{
		// Private competitive session configuration for team practice and scrimmages
		CompetitiveSessionSettings.bIsLANMatch = false;
		CompetitiveSessionSettings.bAllowJoinViaPresence = false;
		CompetitiveSessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
		CompetitiveSessionSettings.bAllowInvites = true;  // Enable invitation-only access for team coordination
	}
	else
	{
		// Public competitive session configuration for open matchmaking
		CompetitiveSessionSettings.bIsLANMatch = false;
		CompetitiveSessionSettings.bAllowJoinViaPresence = true;              // Enable public discovery for competitive matchmaking
		CompetitiveSessionSettings.bAllowJoinViaPresenceFriendsOnly = true;   // Priority access for friends in competitive environment
		CompetitiveSessionSettings.bAllowInvites = true;                      // Allow invitations for team formation
	}
	
	return CompetitiveSessionSettings;
}




