// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "UObject/Object.h"
#include "EasyOnlineHost.generated.h"

/**
 * EasyOnline Host Controller
 * 
 * Specialized hosting system designed for competitive multiplayer games with deterministic rollback netcode.
 * Provides frame-perfect session management, state synchronization, and optimized networking for Recall ECS integration.
 * 
 * Key Features:
 * - Deterministic session creation with frame-based synchronization
 * - Enhanced session settings for competitive gameplay balance
 * - Optimized for low-latency multiplayer with rollback support
 * - Integrated spectator mode and reconnection handling
 * - Recall ECS-aware networking protocols
 */
UCLASS()
class EASYONLINE_API UEasyOnlineHost : public UObject
{
	GENERATED_BODY()

public:
	/** 
	 * Creates deterministic lobby session with competitive multiplayer settings.
	 * Establishes frame-synchronized environment for player gathering and match configuration.
	 * Optimized for low-latency communication and Recall ECS state management.
	 */
	bool HostLobby(const FUniqueNetId& HostPlayerId, bool bPrivateSession, int32 NumPublicConnections);
	
	/** 
	 * Initializes competitive match session on specified map with rollback netcode support.
	 * Configures deterministic networking protocols and frame-perfect synchronization.
	 * Essential for competitive gameplay requiring precise state replication.
	 */
	bool HostGameMap(const FUniqueNetId& HostPlayerId, const FString& MapName, bool bPrivateSession, int32 NumPublicConnections);
	
	/** 
	 * Enables seamless background hosting for continuous session availability.
	 * Maintains session state while allowing host to remain in current context.
	 * Designed for always-available competitive servers with persistent state.
	 */
	bool HostGameInBackground(const FUniqueNetId& HostPlayerId, bool bPrivateSession, int32 NumPublicConnections);
	
	/** 
	 * Gracefully terminates hosting session with proper state cleanup.
	 * Ensures deterministic shutdown and notifies connected clients for clean disconnection.
	 */
	UFUNCTION(BlueprintCallable, Category=Online)
	void StopHost();

	/** 
	 * Determines current hosting status for competitive multiplayer session management.
	 * Critical for UI state updates and networking protocol decisions.
	 */
	bool IsHosting() const;
	
	/**
	 * Generates optimized session configuration for competitive multiplayer gameplay.
	 * Creates settings tailored for low-latency, frame-synchronized gaming experiences.
	 * Includes rollback netcode optimizations and Recall ECS networking parameters.
	 */
	FOnlineSessionSettings GenerateOnlineSessionSettings(bool bPrivateSession, int32 NumPublicConnections);
	
private:
	/** Accesses centralized online management system for competitive multiplayer coordination */
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;
	
	/** Handles completion of deterministic session creation for competitive map hosting */
	void OnCreateSessionToHostGameMapComplete(const FName& SessionName, bool bWasSuccessful);
	
	/** Manages background session creation completion for continuous server availability */
	void OnCreateSessionToHostGameInBackgroundComplete(const FName& SessionName, bool bWasSuccessful);
	
	/** Stores map name awaiting session establishment for deterministic hosting flow */
	TOptional<FString> PendingHostMap;
};
