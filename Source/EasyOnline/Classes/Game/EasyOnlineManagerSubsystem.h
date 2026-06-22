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

/**
 * EasyOnline Manager Subsystem
 * 
 * Central coordination system for competitive multiplayer networking with deterministic rollback support.
 * Orchestrates all online functionality optimized for frame-perfect synchronization and low-latency gameplay.
 * 
 * Core Responsibilities:
 * - Deterministic session lifecycle management for competitive gaming
 * - Frame-synchronized networking protocol coordination
 * - Real-time connection quality monitoring and optimization
 * - Recall ECS integration for rollback netcode compatibility
 * - Advanced friend system with competitive matchmaking priorities
 * - Session invitation management with spectator mode support
 * - Network failure recovery with seamless reconnection handling
 * 
 * Architecture Features:
 * - Centralized online state management for consistent multiplayer experience
 * - Optimized delegate system for responsive networking events
 * - Platform-agnostic LAN fallback for development and tournament environments
 * - Enhanced error handling for competitive stability requirements
 */
UCLASS()
class EASYONLINE_API UEasyOnlineManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** 
	 * Initializes competitive multiplayer networking infrastructure.
	 * Establishes deterministic protocols and Recall ECS integration for rollback support.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Determines platform-specific networking mode for optimal competitive performance.
	 * Returns true for development/tournament environments requiring LAN-only connections.
	 */
	bool IsForceLanSessionPlatform() const;
	
	/** Accesses enhanced friend management system with competitive matchmaking integration */
	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineFriendManager* GetFriendManager() const { return FriendManager; }

	/** Retrieves intelligent session discovery system optimized for competitive play */
	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineQuickJoin* GetQuickJoin() const { return QuickJoin; }

	/** Accesses deterministic hosting controller for competitive multiplayer sessions */
	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineHost* GetHostManager() const { return HostManager; }

	/** Retrieves advanced invitation system with spectator mode and reconnection support */
	UFUNCTION(BlueprintCallable, Category=Online)
	FORCEINLINE UEasyOnlineSessionInvitationManager* GetSessionInvitationManager() const { return SessionInvitationManager; }
	
private: // Private Functions

	/**
	 * Monitors network connection status for competitive stability requirements.
	 * Implements automatic quality assessment and reconnection strategies for seamless gameplay.
	 */
	void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

	/**
	 * Processes session failures with intelligent recovery mechanisms.
	 * Ensures competitive integrity through proper error handling and state restoration.
	 */
	void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
	
	/** Enhanced friend management system with competitive multiplayer integration */
	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineFriendManager> FriendManager;

	/** Intelligent session discovery and joining system for competitive matchmaking */
	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineQuickJoin> QuickJoin;

	/** Deterministic hosting controller optimized for rollback netcode gameplay */
	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineHost> HostManager;

	/** Advanced invitation management with spectator support and reconnection handling */
	UPROPERTY(Transient)
	TObjectPtr<UEasyOnlineSessionInvitationManager> SessionInvitationManager;

	/** Current network connection status for competitive stability monitoring */
	EOnlineServerConnectionStatus::Type	CurrentConnectionStatus;

	/** Delegate handle for session failure recovery in competitive environments */
	FDelegateHandle SessionFailureDelegateHandle;
	
	/** Delegate handle for connection status monitoring in real-time multiplayer */
	FDelegateHandle ConnectionStatusChangedDelegateHandle;
};
