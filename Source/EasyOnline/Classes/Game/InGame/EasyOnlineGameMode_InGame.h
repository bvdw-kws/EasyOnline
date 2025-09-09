// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Game/Base/EasyOnlineGameModeBase.h"

#include "EasyOnlineGameMode_InGame.generated.h"

class AEasyOnlinePlayerController_InGame;

/**
 * Base class for in-game game mode with EasyOnline framework support.
 */
UCLASS()
class EASYONLINE_API AEasyOnlineGameMode_InGame :
	public AEasyOnlineGameModeBase
{
	GENERATED_UCLASS_BODY()

public:
	int32 GetMaxPlayingPlayers() const { return MaxPlayingPlayers; }
	
public:
	/**
	 * Called when a player finished traveling to the map and its controller is ready.
	 */
	virtual void SyncReady(AEasyOnlinePlayerController_InGame* PC);

protected:
	/** The maximum number of playing players before new players must spectate */
	UPROPERTY(EditAnywhere, Category = GameMode)
	int32 MaxPlayingPlayers;
};
