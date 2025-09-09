// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineQuickJoin.h"

#include "EasyOnlineFriendManager.h"
#include "EasyOnlineSessionClient.h"
#include "Engine/LocalPlayer.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Game/EasyOnlineTypes.h"
#include "Utility/EasyOnlineFunctionLibrary.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"

bool UEasyOnlineQuickJoin::QuickJoinSession(const ULocalPlayer* LocalPlayer)
{
	if (!ensureAlwaysMsgf(IsValid(LocalPlayer),
		TEXT("%hs Invalid ULocalPlayer"), __FUNCTION__))
	{
		return false;
	}
	
	// Check if we already have an active session
	if (UEasyOnlineFunctionLibrary::IsSessionActiveOrPending())
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Session already active or pending - ignoring QuickJoin request"), __FUNCTION__);
		return true; // Return true since we're already in a session or joining one
	}
	
	// Check if we already have a search/join operation in progress
	if (FindSessionForQuickJoinDelegateHandle.IsValid() || RefreshFriendForQuickJoinDelegateHandle.IsValid())
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: QuickJoin operation already in progress - ignoring duplicate request"), __FUNCTION__);
		return true; // Return true since an operation is already running
	}
	
	const UEasyOnlineManagerSubsystem* OnlineManager = GetOnlineManager();
	if (!ensureAlwaysMsgf(IsValid(OnlineManager),
		TEXT("%hs Invalid UEasyOnlineManagerSubsystem"), __FUNCTION__))
	{
		return false;
	}
	
	if (OnlineManager->IsForceLanSessionPlatform())
	{
		UEasyOnlineSessionClient* OnlineSessionClient = UEasyOnlineSessionClient::GetInWorld(GetWorld());
		
		FindSessionForQuickJoinDelegateHandle = OnlineSessionClient->AddEasyOnlineOnFindSessionsCompleteDelegate_Handle(
			FEasyOnlineOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsToQuickJoinComplete));
		
		constexpr bool bIsLanQuery = true;
		constexpr int32 MaxSearchResults = 100;
		if (OnlineSessionClient->SearchSessions(*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
				bIsLanQuery, MaxSearchResults))
		{
			return true;
		}
		
		OnlineSessionClient->ClearEasyOnlineOnFindSessionsCompleteDelegate_Handle(FindSessionForQuickJoinDelegateHandle);
	}
	else
	{
		UEasyOnlineFriendManager* FriendManager = OnlineManager->GetFriendManager();
		
		RefreshFriendForQuickJoinDelegateHandle = FriendManager->AddEasyOnlineOnUpdateFriendsCompleteDelegate_Handle(
			FEasyOnlineOnUpdateFriendsCompleteDelegate::CreateUObject(this, &ThisClass::OnRefreshFriendToQuickJoinComplete));
		
		if (FriendManager->UpdateFriendList(LocalPlayer->GetLocalPlayerIndex()))
		{
			return true;
		}
		
		FriendManager->ClearEasyOnlineOnUpdateFriendsCompleteDelegate_Handle(RefreshFriendForQuickJoinDelegateHandle);
	}
	
	return false;
}


void UEasyOnlineQuickJoin::OnFindSessionsToQuickJoinComplete(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	UEasyOnlineSessionClient* OnlineSessionClient = UEasyOnlineSessionClient::GetInWorld(GetWorld());
	if (!ensureAlwaysMsgf(IsValid(OnlineSessionClient),
		TEXT("%hs Invalid UEasyOnlineSessionClient"), __FUNCTION__))
	{
		TriggerEasyOnlineOnQuickSessionJoinFailedDelegates();	
		return;
	}
	
	if (FindSessionForQuickJoinDelegateHandle.IsValid())
	{
		OnlineSessionClient->ClearEasyOnlineOnFindSessionsCompleteDelegate_Handle(FindSessionForQuickJoinDelegateHandle);
	}

	if (bWasSuccessful && SearchResults.Num() > 0)
	{
		const FOnlineSessionSearchResult& SelectedSearchResult = SearchResults[0];
		OnlineSessionClient->JoinSession(NAME_GameSession, SelectedSearchResult);
		
		TriggerEasyOnlineOnQuickSessionJoinCompleteDelegates();
		
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Joining session: %s"),
			__FUNCTION__, *SelectedSearchResult.Session.GetSessionIdStr());
	}
	else
	{
		TriggerEasyOnlineOnQuickSessionJoinFailedDelegates();		
	}
}

void UEasyOnlineQuickJoin::OnRefreshFriendToQuickJoinComplete(int32 LocalUserNum, bool bWasSuccessful,
	const TArray<TSharedRef<FOnlineFriend>>& Friends)
{
	const UEasyOnlineManagerSubsystem* OnlineManager = GetOnlineManager();
	if (!ensureAlwaysMsgf(IsValid(OnlineManager),
		TEXT("%hs Invalid UEasyOnlineManagerSubsystem"), __FUNCTION__))
	{
		TriggerEasyOnlineOnQuickSessionJoinFailedDelegates();	
		return;
	}
	
	UEasyOnlineFriendManager* FriendManager = OnlineManager->GetFriendManager();
	if (!ensureAlwaysMsgf(IsValid(FriendManager),
		TEXT("%hs Invalid UEasyOnlineFriendManager"), __FUNCTION__))
	{
		TriggerEasyOnlineOnQuickSessionJoinFailedDelegates();	
		return;
	}
	
	if (RefreshFriendForQuickJoinDelegateHandle.IsValid())
	{
		FriendManager->ClearEasyOnlineOnUpdateFriendsCompleteDelegate_Handle(RefreshFriendForQuickJoinDelegateHandle);
	}

	if (bWasSuccessful)
	{
		for (const TSharedRef<FOnlineFriend>& Friend : Friends)
		{
			const FOnlineUserPresence& Presence = Friend->GetPresence();
			if (Presence.bIsPlayingThisGame && Presence.bIsJoinable)
			{
				if (FriendManager->JoinFriendSession(LocalUserNum, Friend->GetUserId().Get()))
				{
					UE_LOG(LogEasyOnline, Log,
						TEXT("%hs Joining friend %s(%s) session"), __FUNCTION__,
						*Friend->GetDisplayName(),
						*Friend->GetUserId()->ToString());

					TriggerEasyOnlineOnQuickSessionJoinCompleteDelegates();					
					return;
				}
			}
		}
		
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: No valid session to join"), __FUNCTION__);
	}
	else
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Failed to find friend sessions"), __FUNCTION__);
	}

	TriggerEasyOnlineOnQuickSessionJoinFailedDelegates();
}

UEasyOnlineManagerSubsystem* UEasyOnlineQuickJoin::GetOnlineManager() const
{
	return CastChecked<UEasyOnlineManagerSubsystem>(GetOuter());
}

