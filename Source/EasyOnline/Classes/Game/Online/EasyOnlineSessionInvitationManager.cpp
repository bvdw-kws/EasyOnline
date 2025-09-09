// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineSessionInvitationManager.h"

#include "EasyOnlineSessionClient.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Misc/CommandLine.h"
#include "OnlineSubsystemUtils.h"
#include "Game/EasyOnlineTypes.h"
#include "Settings/EasyOnlineSettings.h"

void UEasyOnlineSessionInvitationManager::Initialize()
{
	if(OnSessionInviteReceivedDelegateHandle.IsValid()) return;
	
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if (Sessions.IsValid())
	{
		OnSessionInviteReceivedDelegateHandle = Sessions->AddOnSessionInviteReceivedDelegate_Handle(
			FOnSessionInviteReceivedDelegate::CreateUObject(this, &ThisClass::OnSessionInviteReceived));
	}

	// Auto-join when received any valid invitation
	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	if (EasyOnlineSettings->bAutoAcceptSessionInvitation ||
		FParse::Param(FCommandLine::Get(), TEXT("easyOnline.AutoAcceptSessionInvitation")))
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs Enable Auto-Join Session Invitation!"), __FUNCTION__);
		AddEasyOnlineOnSessionInvitationReceivedDelegate_Handle(FEasyOnlineOnSessionInvitationReceivedDelegate::CreateLambda([this](const FUniqueNetId& UserId, const FUniqueNetId& InviteFromUserId)
		{
			this->AcceptInvitationFrom(UserId, InviteFromUserId);
		}));
	}
}

bool UEasyOnlineSessionInvitationManager::SendInvitation(int32 LocalUserNum, const FUniqueNetId& ToUserId)
{
	UE_LOG(LogEasyOnline, Log, TEXT("%hs ToUserId(%s)"), __FUNCTION__, *ToUserId.ToString());
	
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if (Sessions.IsValid() == false) return false;
	
	return Sessions->SendSessionInviteToFriend(LocalUserNum, NAME_GameSession, ToUserId);
}

void UEasyOnlineSessionInvitationManager::AcceptInvitationFrom(const FUniqueNetId& UserId, const FUniqueNetId& InviteFromUserId)
{
	const FUniqueNetIdRef SenderIdRef = InviteFromUserId.AsShared();
	if(Invitations.Contains(SenderIdRef))
	{
		FOnlineSessionSearchResult SearchResult;
		SearchResult.Session = Invitations.FindAndRemoveChecked(SenderIdRef);

		UE_LOG(
			LogEasyOnline,
			Log,
			TEXT("%hs SenderId(%s) SessionId(%s)"),
			__FUNCTION__,
			*InviteFromUserId.ToString(),
			*SearchResult.GetSessionIdStr());
		
		UEasyOnlineSessionClient::GetInWorld(GetWorld())->JoinSession(NAME_GameSession, SearchResult);
	}
	else
	{
		UE_LOG(
			LogEasyOnline,
			Warning,
			TEXT("%hs Invitation doesn't exist from %s"),
			__FUNCTION__,
			*InviteFromUserId.ToString());
	}
}

int32 UEasyOnlineSessionInvitationManager::GetInvitationUsers(TArray<FUniqueNetIdRef>& InvitationUser) const
{
	Invitations.GetKeys(InvitationUser);
	return InvitationUser.Num();
}

UEasyOnlineManagerSubsystem* UEasyOnlineSessionInvitationManager::GetOnlineManager() const
{
	return CastChecked<UEasyOnlineManagerSubsystem>(GetOuter());
}

void UEasyOnlineSessionInvitationManager::OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogEasyOnline, Log, TEXT("%hs FromId(%s) SessionId(%s)"), __FUNCTION__, *FromId.ToString(), *InviteResult.GetSessionIdStr());
	
	FOnlineSession& Session = Invitations.FindOrAdd(FromId.AsShared());
	Session = InviteResult.Session;

	TriggerEasyOnlineOnSessionInvitationReceivedDelegates(UserId, FromId);
}
