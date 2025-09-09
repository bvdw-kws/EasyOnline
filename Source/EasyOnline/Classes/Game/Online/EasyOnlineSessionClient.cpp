// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineSessionClient.h"

#include "Engine/LocalPlayer.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Game/EasyOnlineTypes.h"

void UEasyOnlineSessionClient::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete);
	OnSearchSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnSearchSessionComplete);
}

void UEasyOnlineSessionClient::JoinSession(FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	if(SearchResult.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs Cannot join to invalid session"), __FUNCTION__);
		return;
	}
	
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if(Sessions.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs IOnlineSessionPtr doesn't exist"), __FUNCTION__);
		return;
	}

	// These flags should be the same on Steam, but UE 5.5 broke this part.
	FOnlineSessionSearchResult NewSearchResult = SearchResult;
	NewSearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = NewSearchResult.Session.SessionSettings.bUsesPresence = true;
	
	const EOnlineSessionState::Type SessionState = Sessions->GetSessionState(SessionName);
	if (SessionState != EOnlineSessionState::NoSession)
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs End existing session to join"), __FUNCTION__);
		CachedSessionResult = NewSearchResult;
		EndExistingSession(SessionName, OnEndForJoinSessionCompleteDelegate);
		return;
	}

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if(IsValid(LocalPlayer) == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs Local player doesn't exist"), __FUNCTION__);
		return;
	}
	
	OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	const bool bJoinProcessStarted = Sessions->JoinSession(
		*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
		SessionName,
		NewSearchResult);
	
	if(bJoinProcessStarted == false && OnJoinSessionCompleteDelegateHandle.IsValid())
	{
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}
}

bool UEasyOnlineSessionClient::CreateSession(const FName& SessionName, const FOnlineSessionSettings& SessionSettings, const FEasyOnlineOnCreateSessionCompleteDelegate& Callback)
{
	if(Callback.IsBound())
	{
		PendingOnCreateSessionCompleteDelegateHandle = AddEasyOnlineOnCreateSessionCompleteDelegate_Handle(Callback);
	}
	
	// We need to make sure we don't already have a session, and if we do, we should destroy it before hand.
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if (Sessions.IsValid() && Sessions->GetNamedSession(SessionName) != nullptr)
	{
		Sessions->DestroySession(SessionName);
	}

	const ULocalPlayer* LocalPlayer = GetGameInstance()->GetFirstGamePlayer();
	if(IsValid(LocalPlayer) == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs Local player doesn't exist"), __FUNCTION__);
		return false;
	}
	
	OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
	if(
		Sessions->CreateSession(
			*LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
			SessionName,
			SessionSettings))
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs creating session SessionName: %s"), __FUNCTION__, *SessionName.ToString());
		return true;
	}

	UE_LOG(LogEasyOnline, Warning, TEXT("%hs create session failed"), __FUNCTION__);

	// Unsubscribe if create session was failed
	if(OnCreateSessionCompleteDelegateHandle.IsValid())
	{
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	return false;
}

UEasyOnlineSessionClient* UEasyOnlineSessionClient::GetInWorld(const UWorld* World)
{
	if(IsValid(World) == false) return nullptr;

	return Cast<UEasyOnlineSessionClient>(World->GetGameInstance()->GetOnlineSession());
}

void UEasyOnlineSessionClient::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if(Sessions.IsValid())
	{
		if(OnCreateSessionCompleteDelegateHandle.IsValid())
		{
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		}
	}

	if(bWasSuccessful && Sessions.IsValid() && Sessions->GetNamedSession(InSessionName))
	{
		UE_LOG(
			LogEasyOnline,
			Log,
			TEXT("%hs: SessionName(%s) SessionId(%s) bWasSuccessful(%d)"),
			__FUNCTION__,
			*InSessionName.ToString(),
			*Sessions->GetNamedSession(InSessionName)->GetSessionIdStr(),
			bWasSuccessful ? 1 : 0);
	}
	else
	{
		UE_LOG(
			LogEasyOnline,
			Warning,
			TEXT("%hs: Failed to create session. SessionName(%s) bWasSuccessful(%d)"),
			__FUNCTION__,
			*InSessionName.ToString(),
			bWasSuccessful ? 1 : 0);
	
	}

	TriggerEasyOnlineOnCreateSessionCompleteDelegates(InSessionName, bWasSuccessful);

	if(PendingOnCreateSessionCompleteDelegateHandle.IsValid())
	{
		ClearEasyOnlineOnCreateSessionCompleteDelegate_Handle(PendingOnCreateSessionCompleteDelegateHandle);
	}
}

bool UEasyOnlineSessionClient::SearchSessions(const FUniqueNetId& CallerPlayer, bool bIsLanQuery, int32 MaxSearchResults)
{
	if(CallerPlayer.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs Invalid CallerPlayer"), __FUNCTION__);
		return false;
	}
	
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if(Sessions.IsValid() == false)
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs IOnlineSessionPtr doesn't exist"), __FUNCTION__);
		return false;
	}

	SearchSettings = MakeShared<FOnlineSessionSearch>();
	SearchSettings->bIsLanQuery = bIsLanQuery;
	SearchSettings->MaxSearchResults = MaxSearchResults;
	// SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if(SearchSessionCompleteDelegateHandle.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(SearchSessionCompleteDelegateHandle);
	}
	SearchSessionCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnSearchSessionsCompleteDelegate);
	
	if(Sessions->FindSessions(CallerPlayer, SearchSettings.ToSharedRef()))
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs Searching sessions..."), __FUNCTION__);
		return true;
	}

	if(SearchSessionCompleteDelegateHandle.IsValid())
	{
		Sessions->ClearOnFindSessionsCompleteDelegate_Handle(SearchSessionCompleteDelegateHandle);
	}
	
	return false;
}

void UEasyOnlineSessionClient::OnSearchSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs: Search sessions was successful. Num Search Result: %d"), __FUNCTION__, SearchSettings->SearchResults.Num());
		
		for (int32 SearchIdx = 0; SearchIdx < SearchSettings->SearchResults.Num(); SearchIdx++)
		{
			const FOnlineSessionSearchResult& SearchResult = SearchSettings->SearchResults[SearchIdx];
			DumpSession(&SearchResult.Session);
		}
	}
	else
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs: Search sessions was not successful"), __FUNCTION__);
	}
	
	TriggerEasyOnlineOnFindSessionsCompleteDelegates(bWasSuccessful, SearchSettings->SearchResults);
}
