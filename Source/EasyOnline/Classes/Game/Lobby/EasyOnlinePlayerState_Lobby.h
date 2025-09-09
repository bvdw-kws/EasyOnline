// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Game/Base/EasyOnlinePlayerState.h"

#include "EasyOnlinePlayerState_Lobby.generated.h"

UCLASS()
class EASYONLINE_API AEasyOnlinePlayerState_Lobby : public AEasyOnlinePlayerState
{
	GENERATED_BODY()

	//~ Begin AActor Interface
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void PostInitializeComponents() override;
	//~ End AActor Interface
	
	//~ Begin APlayerState Interface
protected:
	virtual void SeamlessTravelTo(class APlayerState* NewPlayerState) override;
	//~ End APlayerState Interface
};
