// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "TestStates/EasyGauntletState.h"

class UEasyOnlineGauntletController;

/**
 * Test state for creating and hosting an online session
 * Integrates with EasyOnline session management
 */
class EASYGAUNTLETTEST_API FEasyOnlineHostState : public FEasyGauntletState
{
public:
	FEasyOnlineHostState(UEasyOnlineGauntletController* InController, int32 InMaxPlayers = 2, float InTimeout = 30.0f);
	
	virtual void OnEnterState() override;
	virtual void OnTick(float DeltaTime) override;
	virtual FString GetStateName() const override;

private:
	void CreateSession();
	void OnSessionCreated(bool bWasSuccessful);
	
	UEasyOnlineGauntletController* OnlineController;
	int32 MaxPlayers;
	bool bSessionCreationStarted = false;
	bool bSessionCreated = false;
};