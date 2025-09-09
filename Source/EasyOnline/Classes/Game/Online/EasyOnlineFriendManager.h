// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "UObject/Object.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSessionSettings.h"

#include "EasyOnlineFriendManager.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FEasyOnlineOnUpdateFriendsComplete, int32 LocalPlayerNum, bool bWasSuccessful, const TArray<TSharedRef<FOnlineFriend>>& Friends);
typedef FEasyOnlineOnUpdateFriendsComplete::FDelegate FEasyOnlineOnUpdateFriendsCompleteDelegate;

class UEasyOnlineManagerSubsystem;

/**
 * 
 */
UCLASS()
class EASYONLINE_API UEasyOnlineFriendManager : public UObject
{
	GENERATED_BODY()

public:
	/** Update friend list with presence data to cache*/
	UFUNCTION(BlueprintCallable, Category=Friend)
	bool UpdateFriendList(int32 LocalPlayerNum);
	
	/** Joins by given friend ID */
	bool JoinFriendSession(int32 LocalPlayerNum, const FUniqueNetId& FriendId);

	/** Get friend by friend id from "cache". Return null if don't exist*/
	TSharedPtr<FOnlineFriend> GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId) const;
	/** Get friend list from "cache" */
	bool GetFriendsList(int32 LocalUserNum, TArray<TSharedRef<FOnlineFriend>>& OutFriends) const;

	DEFINE_ONLINE_DELEGATE_THREE_PARAM(EasyOnlineOnUpdateFriendsComplete, int32, bool, const TArray<TSharedRef<FOnlineFriend>>&);

private:
	
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;

	void OnFinishReadFriendList(int32 LocalUserNum,	bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	
	/** Event when find friend session to join completed  */
	void OnFindFriendSessionToJoinComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult);

	FDelegateHandle FindFriendSessionToJoinDelegateHandle;
};
