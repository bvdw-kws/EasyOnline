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

/**
 * EasyOnline Quick Join System
 * 
 * High-performance session discovery and joining system optimized for competitive multiplayer.
 * Provides intelligent matchmaking with latency-aware session selection and rollback netcode compatibility.
 * 
 * Features:
 * - Smart session filtering based on connection quality and game state
 * - Frame-synchronization aware connection establishment
 * - Automatic friend session prioritization for competitive play
 * - Resilient connection handling with reconnection support
 * - MassStep ECS state synchronization during join process
 */
UCLASS(Within=EasyOnlineManagerSubsystem)
class EASYONLINE_API UEasyOnlineQuickJoin : public UObject
{
	GENERATED_BODY()

public:
	/** 
	 * Intelligently discovers and joins optimal competitive multiplayer session.
	 * Prioritizes low-latency connections suitable for rollback netcode gameplay.
	 * Performs automatic friend session discovery and connection quality assessment.
	 */
	bool QuickJoinSession(const ULocalPlayer* LocalPlayer);

	DEFINE_ONLINE_DELEGATE(EasyOnlineOnQuickSessionJoinComplete);
	DEFINE_ONLINE_DELEGATE(EasyOnlineOnQuickSessionJoinFailed);

private:
	
	/** Retrieves core online management system for competitive multiplayer coordination */
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;
		
	/** 
	 * Processes session discovery results with latency optimization for competitive play.
	 * Automatically selects best available session based on connection quality metrics.
	 */
	void OnFindSessionsToQuickJoinComplete(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);

	/** 
	 * Handles friend list refresh for priority-based session joining.
	 * Prioritizes friend sessions for enhanced competitive multiplayer experience.
	 */
	void OnRefreshFriendToQuickJoinComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<TSharedRef<FOnlineFriend>>& Friends);
	
	/** Delegate handle for session discovery completion in competitive matchmaking flow */
	FDelegateHandle FindSessionForQuickJoinDelegateHandle;
	
	/** Delegate handle for friend session refresh in priority-based joining system */
	FDelegateHandle RefreshFriendForQuickJoinDelegateHandle;
};
