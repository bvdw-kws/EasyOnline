// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlinePlayerState_Lobby.h"

void AEasyOnlinePlayerState_Lobby::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AEasyOnlinePlayerState_Lobby::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AEasyOnlinePlayerState_Lobby::SeamlessTravelTo(class APlayerState* NewPlayerState)
{
	Super::SeamlessTravelTo(NewPlayerState);

	NewPlayerState->SetIsSpectator(IsSpectator());
}
