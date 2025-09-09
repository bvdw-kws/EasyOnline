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
 * Test state for joining an existing online session
 * Integrates with EasyOnline session discovery and joining
 */
class EASYGAUNTLETTEST_API FEasyOnlineJoinState : public FEasyGauntletState
{
public:
	FEasyOnlineJoinState(UEasyOnlineGauntletController* InController, float InTimeout = 30.0f);
	
	virtual void OnEnterState() override;
	virtual void OnTick(float DeltaTime) override;
	virtual FString GetStateName() const override;

private:
	void FindAndJoinSession();
	void OnSessionJoined(bool bWasSuccessful);
	
	UEasyOnlineGauntletController* OnlineController;
	bool bJoinStarted = false;
	bool bJoinCompleted = false;
	float SessionSearchTimer = 0.0f;
	const float SessionSearchInterval = 2.0f; // Search every 2 seconds
};