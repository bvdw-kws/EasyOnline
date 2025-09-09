// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "TestStates/EasyDelayState.h"
#include "EasyGauntletTest.h"

FEasyDelayState::FEasyDelayState(UEasyGauntletController* InController, float InDelayTime)
	: FEasyGauntletState(InController)
	, DelayTime(InDelayTime)
{
	check(DelayTime > 0.0f);
}

void FEasyDelayState::OnTick(float DeltaTime)
{
	FEasyGauntletState::OnTick(DeltaTime);
	
	if (!IsFinishState() && StateTimer >= DelayTime)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Delay completed after %.2f seconds"), StateTimer);
		FinishState(true);
	}
}

FString FEasyDelayState::GetStateName() const
{
	return FString::Printf(TEXT("EasyDelayState(%.2fs)"), DelayTime);
}