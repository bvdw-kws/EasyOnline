// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "TestStates/EasyGauntletState.h"
#include "Engine/World.h"

/**
 * Template state that waits for a specific GameState type to become active
 * Useful for waiting on game mode transitions
 */
template<typename TGameStateClass>
class EASYGAUNTLETTEST_API FEasyWaitForGameState : public FEasyGauntletState
{
public:
	FEasyWaitForGameState(UEasyGauntletController* InController, const FString& InCustomName = TEXT(""), float InTimeout = 30.0f);
	
	virtual void OnTick(float DeltaTime) override;
	virtual FString GetStateName() const override;

private:
	FString CustomName;
	TWeakObjectPtr<TGameStateClass> FoundGameState;
};