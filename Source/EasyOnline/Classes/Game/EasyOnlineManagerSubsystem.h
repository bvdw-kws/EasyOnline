// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "EasyOnlineManagerSubsystem.generated.h"

class ULocalPlayer;
class UEasyOnlineFriendManager;
class UEasyOnlineQuickJoin;
class UEasyOnlineHost;
class UEasyOnlineSessionInvitationManager;

UCLASS()
class EASYONLINE_API UEasyOnlineManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Subsystem Generic */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool IsForceLanSessionPlatform() const;
	
	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineFriendManager* GetFriendManager() const { return FriendManager; }

	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineQuickJoin* GetQuickJoin() const { return QuickJoin; }

	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineHost* GetHostManager() const { return HostManager; }

	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineSessionInvitationManager* GetSessionInvitationManager() const { return SessionInvitationManager; }
	
private: // Private Functions

	void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

	void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
	
	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineFriendManager> FriendManager;

	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineQuickJoin> QuickJoin;

	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineHost> HostManager;

	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineSessionInvitationManager> SessionInvitationManager;

	EOnlineServerConnectionStatus::Type	CurrentConnectionStatus;

	FDelegateHandle SessionFailureDelegateHandle;
	FDelegateHandle ConnectionStatusChangedDelegateHandle;
};
