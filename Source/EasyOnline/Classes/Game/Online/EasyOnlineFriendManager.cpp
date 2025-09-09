// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineFriendManager.h"

#include "EasyOnlineSessionClient.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystemUtils.h"
#include "Engine/LocalPlayer.h"
#include "Game/EasyOnlineTypes.h"

/**
 * Note: Friend list cache under OSS would replace with latest response so we cannot rely on cache from OSS.
 * We can do mapping caches under this class when we wanna to optimize API calling by specific list.
 * However, We can force use default list here to make it simple and save memory for now
 */
#define FRIEND_LIST_STRING EFriendsLists::ToString(EFriendsLists::Default)

bool UEasyOnlineFriendManager::UpdateFriendList(int32 LocalPlayerNum)
{
	const IOnlineFriendsPtr FriendPtr = Online::GetFriendsInterface(GetWorld());
	if(FriendPtr.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: IOnlineFriendsPtr doesn't exist"), __FUNCTION__);
		return false;
	}

	const FOnReadFriendsListComplete RefreshFriendListDelegate = FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnFinishReadFriendList);

	if(FriendPtr->ReadFriendsList(LocalPlayerNum, FRIEND_LIST_STRING, RefreshFriendListDelegate))
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Refreshing friend list"), __FUNCTION__);
		return true;
	}
	
	return false;
}

void UEasyOnlineFriendManager::OnFinishReadFriendList(int32 LocalUserNum,	bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	const IOnlineFriendsPtr FriendPtr = Online::GetFriendsInterface(GetWorld());
	if(FriendPtr.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: IOnlineFriendsPtr doesn't exist"), __FUNCTION__);
		return;
	}
	
	TArray<TSharedRef<FOnlineFriend>> Friends;
	
	if (bWasSuccessful)
	{
		// Note: We can add presence query step here if we needs rich data

		if (FriendPtr->GetFriendsList(LocalUserNum, ListName, Friends))
		{
			UE_LOG(LogEasyOnline, Log, TEXT("%hs: FriendNum: %d"), __FUNCTION__, Friends.Num());
			for(const TSharedRef<FOnlineFriend>& Friend : Friends)
			{
				UE_LOG(LogEasyOnline, Log, TEXT("> FriendId(%s) FriendName(%s)"), // , Presence(%s)"),
					*Friend->GetUserId()->ToString(),
					*Friend->GetDisplayName()); //,
					// *Friend->GetPresence().ToDebugString());
			}
		}
	}
	else
	{
		UE_LOG(LogEasyOnline, Warning,
			TEXT("%hs: Failed to update friend list => %s"), __FUNCTION__, *ErrorStr);
	}
	
	TriggerEasyOnlineOnUpdateFriendsCompleteDelegates(LocalUserNum, bWasSuccessful, Friends);
}

bool UEasyOnlineFriendManager::JoinFriendSession(int32 LocalPlayerNum, const FUniqueNetId& FriendId)
{
	const IOnlineSessionPtr SessionPtr = Online::GetSessionInterface(GetWorld());
	if(SessionPtr.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs: IOnlineSessionPtr doesn't exist"), __FUNCTION__);
		return false;
	}
	
	if(FindFriendSessionToJoinDelegateHandle.IsValid())
	{
		SessionPtr->ClearOnFindFriendSessionCompleteDelegate_Handle(LocalPlayerNum, FindFriendSessionToJoinDelegateHandle);
	}
	FindFriendSessionToJoinDelegateHandle = SessionPtr->AddOnFindFriendSessionCompleteDelegate_Handle(
		LocalPlayerNum,
		FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnFindFriendSessionToJoinComplete));

	if(SessionPtr->FindFriendSession(
		LocalPlayerNum,
		FriendId))
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Finding session to join friend(%s)"), __FUNCTION__, *FriendId.ToString());
		return true;
	}

	// Cleanup if find friend session was failed
	if(FindFriendSessionToJoinDelegateHandle.IsValid())
	{
		SessionPtr->ClearOnFindFriendSessionCompleteDelegate_Handle(LocalPlayerNum, FindFriendSessionToJoinDelegateHandle);
	}
	
	return false;
}

TSharedPtr<FOnlineFriend> UEasyOnlineFriendManager::GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId) const
{
	const IOnlineFriendsPtr FriendPtr = Online::GetFriendsInterface(GetWorld());
	if(FriendPtr.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs: IOnlineFriendsPtr doesn't exist"), __FUNCTION__);
		return nullptr;
	}
	return FriendPtr->GetFriend(LocalUserNum, FriendId, FRIEND_LIST_STRING);
}

void UEasyOnlineFriendManager::OnFindFriendSessionToJoinComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult)
{
	const IOnlineSessionPtr SessionPtr = Online::GetSessionInterface(GetWorld());
	if(SessionPtr.IsValid())
	{
		if(FindFriendSessionToJoinDelegateHandle.IsValid())
		{
			SessionPtr->ClearOnFindFriendSessionCompleteDelegate_Handle(LocalUserNum, FindFriendSessionToJoinDelegateHandle);
		}
	}
			
	if(bWasSuccessful && SearchResult.Num() > 0 && SearchResult[0].IsValid())
	{
		UEasyOnlineSessionClient::GetInWorld(GetWorld())->JoinSession(
			NAME_GameSession,
			SearchResult[0]);
		
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Joining session %s"), __FUNCTION__, *SearchResult[0].Session.GetSessionIdStr());
		return;
	}
	UE_LOG(LogEasyOnline, Log, TEXT("%hs Session not found."), __FUNCTION__);
}

UEasyOnlineManagerSubsystem* UEasyOnlineFriendManager::GetOnlineManager() const
{
	return CastChecked<UEasyOnlineManagerSubsystem>(GetOuter());
}

bool UEasyOnlineFriendManager::GetFriendsList(int32 LocalUserNum, TArray<TSharedRef<FOnlineFriend>>& OutFriends) const
{
	const IOnlineFriendsPtr FriendPtr = Online::GetFriendsInterface(GetWorld());
	if(FriendPtr.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs: IOnlineFriendsPtr doesn't exist"), __FUNCTION__);
		return false;
	}
	return FriendPtr->GetFriendsList(LocalUserNum, FRIEND_LIST_STRING, OutFriends);
}
