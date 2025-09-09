// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CommonPlayerController.h"

#include "EasyOnlinePlayerController_InGame.generated.h"

UCLASS()
class EASYONLINE_API AEasyOnlinePlayerController_InGame : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	bool IsSyncReady() const;

	//~ Begin AActor Interface
public:
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface

private:
	UPROPERTY(Transient)
	int32 NumPreSyncTick = 0;
	
#pragma region RPC
	UFUNCTION(Server, Unreliable)
	void Server_SetSyncReady();
#pragma endregion RPC	
};
