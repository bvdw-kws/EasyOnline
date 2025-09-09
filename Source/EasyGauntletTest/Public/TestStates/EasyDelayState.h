// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "TestStates/EasyGauntletState.h"

/**
 * Simple delay state that waits for a specified duration
 * Useful for synchronization between test phases
 */
class EASYGAUNTLETTEST_API FEasyDelayState : public FEasyGauntletState
{
public:
	FEasyDelayState(UEasyGauntletController* InController, float InDelayTime);
	
	virtual void OnTick(float DeltaTime) override;
	virtual FString GetStateName() const override;

private:
	float DelayTime;
};