// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "UObject/Object.h"
#include "OnlineSessionSettings.h"
#include "OnlineDelegateMacros.h"

#include "EasyOnlineQuickJoin.generated.h"

class UEasyOnlineManagerSubsystem;
class ULocalPlayer;

DECLARE_MULTICAST_DELEGATE(FEasyOnlineOnQuickSessionJoinComplete);
typedef FEasyOnlineOnQuickSessionJoinComplete::FDelegate FEasyOnlineOnQuickSessionJoinCompleteDelegate;

DECLARE_MULTICAST_DELEGATE(FEasyOnlineOnQuickSessionJoinFailed);
typedef FEasyOnlineOnQuickSessionJoinFailed::FDelegate FEasyOnlineOnQuickSessionJoinFailedDelegate;

UCLASS(Within=EasyOnlineManagerSubsystem)
class EASYONLINE_API UEasyOnlineQuickJoin : public UObject
{
	GENERATED_BODY()

public:
	/** Quick join the session public session*/
	bool QuickJoinSession(const ULocalPlayer* LocalPlayer);

	DEFINE_ONLINE_DELEGATE(EasyOnlineOnQuickSessionJoinComplete);
	DEFINE_ONLINE_DELEGATE(EasyOnlineOnQuickSessionJoinFailed);

private:
	
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;
		
	/** Used to join the first found session when attempting a quick join */
	void OnFindSessionsToQuickJoinComplete(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);

	/** Used to refresh join the first found friend session when attempting a quick join */
	void OnRefreshFriendToQuickJoinComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<TSharedRef<FOnlineFriend>>& Friends);
	
	FDelegateHandle FindSessionForQuickJoinDelegateHandle;
	FDelegateHandle RefreshFriendForQuickJoinDelegateHandle;
};
