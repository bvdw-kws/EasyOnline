// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "TestStates/EasyGauntletState.h"
#include "Gauntlet/EasyGauntletController.h"
#include "EasyGauntletTest.h"
#include "Engine/World.h"

FEasyGauntletState::FEasyGauntletState(UEasyGauntletController* InController, float InTimeout)
	: Controller(InController)
{
	if (InTimeout > 0.0f)
	{
		StateTimeout = InTimeout;
	}
}

void FEasyGauntletState::OnEnterState()
{
	StateTimer = 0.0f;
	bIsFinished = false;
	bSucceeded = false;
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Entering state: %s"), *GetStateName());
	
	if (StateTimeout.IsSet())
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("State timeout set to: %.2f seconds"), StateTimeout.GetValue());
	}
}

void FEasyGauntletState::OnExitState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Exiting state: %s (Duration: %.2f seconds, Success: %s)"), 
		*GetStateName(), StateTimer, bSucceeded ? TEXT("Yes") : TEXT("No"));
}

void FEasyGauntletState::OnTick(float DeltaTime)
{
	StateTimer += DeltaTime;
	
	if (StateTimeout.IsSet() && StateTimer >= StateTimeout.GetValue())
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("State %s timed out after %.2f seconds"), 
			*GetStateName(), StateTimer);
		FinishState(false);
	}
}

void FEasyGauntletState::FinishState(bool bSuccess)
{
	if (!bIsFinished)
	{
		bIsFinished = true;
		bSucceeded = bSuccess;
		
		UE_LOG(LogEasyGauntletTest, Display, TEXT("State %s finished with result: %s"), 
			*GetStateName(), bSuccess ? TEXT("Success") : TEXT("Failure"));
	}
}

void FEasyGauntletState::EndTest(int32 ExitCode)
{
	if (Controller)
	{
		Controller->StopTesting(ExitCode);
	}
}

UWorld* FEasyGauntletState::GetWorld() const
{
	return Controller ? Controller->GetWorld() : nullptr;
}