// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Game/Base/EasyOnlinePlayerState.h"

#include "EasyOnlinePlayerState_InGame.generated.h"

UCLASS()
class EASYONLINE_API AEasyOnlinePlayerState_InGame :
	public AEasyOnlinePlayerState
{
	GENERATED_BODY()
	
public:
	void SetSyncReady(bool bReady);
	bool IsSyncReady() const;
	
	//~ Begin AActor Interface
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_SyncReady)
	bool bSyncReady = false;

	UFUNCTION()
	virtual void OnRep_SyncReady();
};
