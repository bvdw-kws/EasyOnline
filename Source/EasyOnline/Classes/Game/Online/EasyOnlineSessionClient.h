// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionClient.h"

#include "EasyOnlineSessionClient.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FEasyOnlineOnCreateSessionComplete, const FName& SessionName, bool bWasSuccessful);
typedef FEasyOnlineOnCreateSessionComplete::FDelegate FEasyOnlineOnCreateSessionCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FEasyOnlineOnFindSessionsComplete, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);
typedef FEasyOnlineOnFindSessionsComplete::FDelegate FEasyOnlineOnFindSessionsCompleteDelegate;

/**
 * 
 */
UCLASS()
class EASYONLINE_API UEasyOnlineSessionClient : public UOnlineSessionClient
{
	GENERATED_BODY()

public:
	// UOnlineSession interface begin
	virtual void RegisterOnlineDelegates() override;
	virtual void JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult) override;
	// UOnlineSession interface end
	
	/** Create a Game Session*/
	bool CreateSession(const FName& SessionName, const FOnlineSessionSettings& SessionSettings, const FEasyOnlineOnCreateSessionCompleteDelegate& Callback = FEasyOnlineOnCreateSessionCompleteDelegate());
	
	/** Search for sessions */
	bool SearchSessions(const FUniqueNetId& CallerPlayer, bool bIsLanQuery, int32 MaxSearchResults);

	DEFINE_ONLINE_DELEGATE_TWO_PARAM(EasyOnlineOnCreateSessionComplete, const FName&, bool);
		
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(EasyOnlineOnFindSessionsComplete, bool, const TArray<FOnlineSessionSearchResult>&);

	/** Helper to get active online session client this class inside world */
	static UEasyOnlineSessionClient* GetInWorld(const UWorld* World);
	
protected:
	/** Callback which is intended to be called upon session creation */
	void OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
	
	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionComplete(bool bWasSuccessful);

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	
	/** Handle to various registered delegates */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle PendingOnCreateSessionCompleteDelegateHandle;
	FDelegateHandle SearchSessionCompleteDelegateHandle;
	
	FOnFindSessionsCompleteDelegate OnSearchSessionsCompleteDelegate;

	/** Current search settings */
	TSharedPtr<FOnlineSessionSearch> SearchSettings;
};
