// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "TestStates/EasyWaitForGameState.h"
#include "EasyGauntletTest.h"
#include "Engine/World.h"

template<typename TGameStateClass>
FEasyWaitForGameState<TGameStateClass>::FEasyWaitForGameState(UEasyGauntletController* InController, const FString& InCustomName, float InTimeout)
	: FEasyGauntletState(InController, InTimeout)
	, CustomName(InCustomName)
{
}

template<typename TGameStateClass>
void FEasyWaitForGameState<TGameStateClass>::OnTick(float DeltaTime)
{
	FEasyGauntletState::OnTick(DeltaTime);
	
	if (!IsFinishState())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			TGameStateClass* GameState = World->GetGameState<TGameStateClass>();
			if (IsValid(GameState))
			{
				FoundGameState = GameState;
				UE_LOG(LogEasyGauntletTest, Display, TEXT("Found GameState: %s"), *GameState->GetClass()->GetName());
				FinishState(true);
			}
		}
	}
}

template<typename TGameStateClass>
FString FEasyWaitForGameState<TGameStateClass>::GetStateName() const
{
	if (!CustomName.IsEmpty())
	{
		return CustomName;
	}
	return FString::Printf(TEXT("EasyWaitForGameState<%s>"), *TGameStateClass::StaticClass()->GetName());
}