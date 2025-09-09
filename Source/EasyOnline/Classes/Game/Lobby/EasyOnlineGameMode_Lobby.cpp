// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineGameMode_Lobby.h"

#include "EasyOnlineGameState_Lobby.h"
#include "EasyOnlineHUD_Lobby.h"
#include "EasyOnlinePlayerController_Lobby.h"
#include "EasyOnlinePlayerState_Lobby.h"

AEasyOnlineGameMode_Lobby::AEasyOnlineGameMode_Lobby()
	: Super()
{
	PlayerControllerClass = AEasyOnlinePlayerController_Lobby::StaticClass();
	GameStateClass = AEasyOnlineGameState_Lobby::StaticClass();
	PlayerStateClass = AEasyOnlinePlayerState_Lobby::StaticClass();
	HUDClass = AEasyOnlineHUD_Lobby::StaticClass();

	bUseSeamlessTravel = true;
}

void AEasyOnlineGameMode_Lobby::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AEasyOnlineGameMode_Lobby::BeginPlay()
{
	Super::BeginPlay();
}

void AEasyOnlineGameMode_Lobby::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AEasyOnlineGameMode_Lobby::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* AEasyOnlineGameMode_Lobby::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void AEasyOnlineGameMode_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AEasyOnlineGameState_Lobby* GS = GetGameState<AEasyOnlineGameState_Lobby>())
	{
		GS->OnNewPlayerJoined();
	}
}
