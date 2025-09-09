// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "UObject/Object.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSessionSettings.h"

#include "EasyOnlineSessionInvitationManager.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FEasyOnlineOnSessionInvitationReceived, const FUniqueNetId& UserId, const FUniqueNetId& FromUserId);
typedef FEasyOnlineOnSessionInvitationReceived::FDelegate FEasyOnlineOnSessionInvitationReceivedDelegate;

class UEasyOnlineManagerSubsystem;

UCLASS()
class EASYONLINE_API UEasyOnlineSessionInvitationManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	/** Send invitation to target friend */
	bool SendInvitation(int32 LocalUserNum, const FUniqueNetId& ToUserId);
	
	/** Accept invitation by sender user ID */
	void AcceptInvitationFrom(const FUniqueNetId& UserId, const FUniqueNetId& InviteFromUserId);

	/** Get list of invitation by user. Return number of invitations */
	int32 GetInvitationUsers(TArray<FUniqueNetIdRef>& InvitationUser) const;

	/** Delegate when invitation received from service */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(EasyOnlineOnSessionInvitationReceived, const FUniqueNetId&, const FUniqueNetId&);
	
private:
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;

	void OnSessionInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult);
	
	FDelegateHandle OnSessionInviteReceivedDelegateHandle;

	TUniqueNetIdMap<FOnlineSession> Invitations;
};
