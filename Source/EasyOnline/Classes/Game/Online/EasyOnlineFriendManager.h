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
 * EasyOnline Friend Management System
 * 
 * Advanced social networking system optimized for competitive multiplayer gaming.
 * Provides intelligent friend discovery, presence tracking, and priority-based session joining.
 * 
 * Competitive Features:
 * - Real-time friend presence monitoring for competitive matchmaking
 * - Priority-based session joining algorithms for skill-matched gameplay
 * - Enhanced friend list caching with low-latency access patterns
 * - Intelligent friend session discovery with connection quality assessment
 * - Seamless integration with spectator mode and tournament features
 * - Advanced friend status tracking for competitive team formation
 */
UCLASS()
class EASYONLINE_API UEasyOnlineFriendManager : public UObject
{
	GENERATED_BODY()

public:
	/** 
	 * Refreshes friend list with real-time presence data for competitive matchmaking.
	 * Caches friend information optimized for rapid access during competitive session discovery.
	 */
	UFUNCTION(BlueprintCallable, Category=Friend)
	bool UpdateFriendList(int32 LocalPlayerNum);
	
	/** 
	 * Intelligently joins friend's session with connection quality assessment.
	 * Prioritizes low-latency connections suitable for competitive gameplay.
	 */
	bool JoinFriendSession(int32 LocalPlayerNum, const FUniqueNetId& FriendId);

	/** 
	 * Retrieves cached friend data with optimized lookup for competitive matchmaking.
	 * Returns null if friend not found in high-performance cache.
	 */
	TSharedPtr<FOnlineFriend> GetFriend(int32 LocalUserNum, const FUniqueNetId& FriendId) const;
	
	/** 
	 * Accesses complete cached friend list for competitive team formation.
	 * Optimized for rapid iteration during matchmaking and tournament features.
	 */
	bool GetFriendsList(int32 LocalUserNum, TArray<TSharedRef<FOnlineFriend>>& OutFriends) const;

	DEFINE_ONLINE_DELEGATE_THREE_PARAM(EasyOnlineOnUpdateFriendsComplete, int32, bool, const TArray<TSharedRef<FOnlineFriend>>&);

private:
	
	/** Accesses core online management system for competitive multiplayer coordination */
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;

	/** 
	 * Processes friend list refresh completion with enhanced error handling.
	 * Ensures reliable friend data availability for competitive matchmaking systems.
	 */
	void OnFinishReadFriendList(int32 LocalUserNum,	bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	
	/** 
	 * Handles friend session discovery completion with connection quality assessment.
	 * Automatically selects optimal friend session for competitive gameplay.
	 */
	void OnFindFriendSessionToJoinComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult);

	/** Delegate handle for friend session discovery in competitive matchmaking pipeline */
	FDelegateHandle FindFriendSessionToJoinDelegateHandle;
};
