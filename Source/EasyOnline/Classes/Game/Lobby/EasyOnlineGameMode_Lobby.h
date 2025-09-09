// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Game/Base/EasyOnlineGameModeBase.h"

#include "EasyOnlineGameMode_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class EASYONLINE_API AEasyOnlineGameMode_Lobby : public AEasyOnlineGameModeBase
{
	GENERATED_BODY()

	AEasyOnlineGameMode_Lobby();
	
	//~ Begin AActor Interface
protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
	
	//~ Begin AGameModeBase interface
protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//~ End AGameModeBase interface
};
