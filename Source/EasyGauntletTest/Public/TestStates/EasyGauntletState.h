// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

class UEasyGauntletController;

/**
 * Base class for all EasyGauntlet test states
 * Provides common functionality for state management, timeouts, and transitions
 */
class EASYGAUNTLETTEST_API FEasyGauntletState
{
public:
	FEasyGauntletState() = delete;
	FEasyGauntletState(UEasyGauntletController* InController, float InTimeout = -1.0f);
	virtual ~FEasyGauntletState() = default;

	virtual void OnEnterState();
	virtual void OnExitState();
	virtual void OnTick(float DeltaTime);
	virtual bool IsFinishState() const { return bIsFinished; }
	virtual FString GetStateName() const = 0;

	bool HasSucceeded() const { return bSucceeded; }
	float GetStateTimer() const { return StateTimer; }

protected:
	void FinishState(bool bSuccess = true);
	void EndTest(int32 ExitCode = 0);
	UWorld* GetWorld() const;
	
	UEasyGauntletController* Controller;
	TOptional<float> StateTimeout;
	float StateTimer = 0.0f;
	bool bIsFinished = false;
	bool bSucceeded = true;
};