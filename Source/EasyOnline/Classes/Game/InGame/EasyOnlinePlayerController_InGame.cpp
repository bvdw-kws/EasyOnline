// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlinePlayerController_InGame.h"

#include "EasyOnlineGameMode_InGame.h"
#include "EasyOnlinePlayerState_InGame.h"
#include "Kismet/GameplayStatics.h"

void AEasyOnlinePlayerController_InGame::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Broadcast to the host that this player finished traveling to the map and is synced.
	if (IsLocalPlayerController() && !IsSyncReady())
	{
		NumPreSyncTick++;

		// Broadcast once per second max.
		if (NumPreSyncTick % 60 == 0)
		{
			Server_SetSyncReady();
		}
	}
}

bool AEasyOnlinePlayerController_InGame::IsSyncReady() const
{
	const AEasyOnlinePlayerState_InGame* EOPlayerState = GetPlayerState<AEasyOnlinePlayerState_InGame>();
	if (IsValid(EOPlayerState))
	{
		return EOPlayerState->IsSyncReady();
	}
	else
	{
		return false;		
	}
}

#pragma region RPC
void AEasyOnlinePlayerController_InGame::Server_SetSyncReady_Implementation()
{
	AEasyOnlinePlayerState_InGame* EOPlayerState = GetPlayerState<AEasyOnlinePlayerState_InGame>();
	if (!IsValid(EOPlayerState) || EOPlayerState->IsSyncReady())
	{
		return;
	}
	
	EOPlayerState->SetSyncReady(true);
	
	if (AEasyOnlineGameMode_InGame* GameMode = Cast<AEasyOnlineGameMode_InGame>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->SyncReady(this);
	}
}
#pragma endregion RPC
