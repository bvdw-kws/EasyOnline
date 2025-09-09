// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineManagerSubsystem.h"

#include "EasyOnlineTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Game/Online/EasyOnlineFriendManager.h"
#include "Game/Online/EasyOnlineHost.h"
#include "Game/Online/EasyOnlineQuickJoin.h"
#include "Game/Online/EasyOnlineSessionInvitationManager.h"
#include "OnlineSubsystemUtils.h"
#include "Settings/EasyOnlineSettings.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
/**
 * Console command to send invite to target user id ( by platform )
 */
static FAutoConsoleCommandWithWorldAndArgs CVarEasyOnlineSessionInvite
(
	TEXT("easyOnline.SessionInvite"),
	TEXT("Send session invite to target user id"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (Args.Num() <= 0) return;
		
			const UEasyOnlineManagerSubsystem* OnlineManager = World->GetGameInstance()->GetSubsystem<UEasyOnlineManagerSubsystem>();
			if(IsValid(OnlineManager) == false) return;

			TArray<TSharedRef<FOnlineFriend>> Friends;
			OnlineManager->GetFriendManager()->GetFriendsList(0, Friends);
			const TSharedRef<FOnlineFriend>* Friend = Friends.FindByPredicate([Args](const TSharedRef<FOnlineFriend>& FriendRef)
			{
				return FriendRef->GetUserId()->ToString().Compare(Args[0], ESearchCase::IgnoreCase) == 0;
			});
			if(Friend != nullptr)
			{
				OnlineManager->GetSessionInvitationManager()->SendInvitation(
					0,
					*Friend->Get().GetUserId());
			}
		}
	)
);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

void UEasyOnlineManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	
	FriendManager = NewObject<UEasyOnlineFriendManager>(this);
	if (EasyOnlineSettings->bAutoUpdateFriendsOnInit)
	{
		FriendManager->UpdateFriendList(0);
	}
	
	QuickJoin = NewObject<UEasyOnlineQuickJoin>(this);
	HostManager = NewObject<UEasyOnlineHost>(this);
	
	SessionInvitationManager = NewObject<UEasyOnlineSessionInvitationManager>(this);
	SessionInvitationManager->Initialize();
	
	// Start as not connected, wait for the current connection status to be broadcasted through the OnConnectionStatusChanged Delegate
	CurrentConnectionStatus = EOnlineServerConnectionStatus::NotConnected;
	
	if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		UE_LOG(LogEasyOnline, Log, TEXT("%hs Found Subsystem %s"), __FUNCTION__, *Subsystem->GetOnlineServiceName().ToString());

		ConnectionStatusChangedDelegateHandle = Subsystem->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &UEasyOnlineManagerSubsystem::HandleNetworkConnectionStatusChanged));

		const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
		if (Sessions.IsValid())
		{
			SessionFailureDelegateHandle = Sessions->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &UEasyOnlineManagerSubsystem::HandleSessionFailure));
		}
	}
	else
	{
		UE_LOG(LogEasyOnline, Error, TEXT("%hs Found no subsystem"), __FUNCTION__);
	}
}

bool UEasyOnlineManagerSubsystem::IsForceLanSessionPlatform() const
{
	if (const UWorld* World = GetWorld())
	{
		return Online::GetSubsystem(World)->GetSubsystemName() == NULL_SUBSYSTEM;
	}
	return true;
}

void UEasyOnlineManagerSubsystem::HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus)
{
	UE_LOG(LogEasyOnline, Log, TEXT("%hs: %s"),
		__FUNCTION__, EOnlineServerConnectionStatus::ToString(ConnectionStatus));

	CurrentConnectionStatus = ConnectionStatus;

	// TODO : Need to create a delegate that the UI layer can bind to in order to display disconnection messages properly (big certification headache)
}

void UEasyOnlineManagerSubsystem::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogEasyOnline, Warning, TEXT("%hs: %u"), __FUNCTION__, static_cast<uint32>(FailureType));
}
