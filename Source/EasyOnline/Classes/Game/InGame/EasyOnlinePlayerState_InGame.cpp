// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlinePlayerState_InGame.h"

#include "Net/UnrealNetwork.h"

void AEasyOnlinePlayerState_InGame::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEasyOnlinePlayerState_InGame, bSyncReady);
}

void AEasyOnlinePlayerState_InGame::SetSyncReady(bool bReady)
{
	if (!HasAuthority() || IsSyncReady() == bReady)
	{
		return;
	}

	bSyncReady = bReady;
	OnRep_SyncReady();
}

void AEasyOnlinePlayerState_InGame::OnRep_SyncReady()
{
}

bool AEasyOnlinePlayerState_InGame::IsSyncReady() const
{
	return bSyncReady;
}
