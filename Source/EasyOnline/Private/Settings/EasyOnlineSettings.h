// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"

#include "EasyOnlineSettings.generated.h"

/**
 * Settings for the easy online system.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Easy Online"))
class UEasyOnlineSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UEasyOnlineSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/**
	 * Force update on init by the online manager
	 */
	UPROPERTY(EditAnywhere, config, Category=Invite)
	bool bAutoUpdateFriendsOnInit = false;

	UPROPERTY(EditAnywhere, config, Category=Invite)
	bool bAutoOpenInviteDialog = true;
	
	UPROPERTY(EditAnywhere, config, Category=Invite)
	bool bAutoAcceptSessionInvitation = false;

	/**
	 * The lobby will automatically start the game once it is full.
	 */
	UPROPERTY(EditAnywhere, config, Category=Lobby)
	bool bAutoStartLobbyTimer = false;
	
	/**
	 * Default map used when quick hosting.
	 */
	UPROPERTY(EditAnywhere, config, Category=Lobby, meta=(AllowedClasses="/Script/Engine.World"))
	FSoftObjectPath QuickHostMap;

	UPROPERTY(EditAnywhere, config, Category=Lobby)
	bool bIsPrivateMode = false;

	UPROPERTY(EditAnywhere, config, Category=Lobby)
	int32 NumPublicConnections = 2;
	
	UPROPERTY(EditAnywhere, config, Category=Lobby)
	bool bFillEmptySlotWithBot = false;
	
	UPROPERTY(EditAnywhere, config, Category=Lobby, meta=(AllowedClasses="/Script/Engine.World"))
	FSoftObjectPath LobbyMap;
	
	UPROPERTY(EditAnywhere, config, Category=Lobby)
	TSoftClassPtr<class UEasyOnlineLobbyWidget> LobbyWidget;

	UPROPERTY(EditAnywhere, config, Category=Bot)
	bool bOverrideBotCount = false;

	UPROPERTY(EditAnywhere, config, Category=Bot, meta=(EditCondition="bOverrideBotCount"))
	int32 OverrideNumPlayerBotsToSpawn = 0;
};
