// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "TestStates/EasyOnlineJoinState.h"
#include "Gauntlet/EasyOnlineGauntletController.h"
#include "EasyGauntletTest.h"

FEasyOnlineJoinState::FEasyOnlineJoinState(UEasyOnlineGauntletController* InController, float InTimeout)
	: FEasyGauntletState(InController, InTimeout)
	, OnlineController(InController)
{
	check(OnlineController);
}

void FEasyOnlineJoinState::OnEnterState()
{
	FEasyGauntletState::OnEnterState();
	
	if (!OnlineController->IsClient())
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("EasyOnlineJoinState can only be used by client instances"));
		FinishState(false);
		return;
	}
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Starting session search and join"));
}

void FEasyOnlineJoinState::OnTick(float DeltaTime)
{
	FEasyGauntletState::OnTick(DeltaTime);
	
	if (!bJoinCompleted)
	{
		SessionSearchTimer += DeltaTime;
		
		// Attempt to find and join session periodically
		if (!bJoinStarted && SessionSearchTimer >= SessionSearchInterval)
		{
			SessionSearchTimer = 0.0f;
			FindAndJoinSession();
		}
		
		// TODO: Check join status from EasyOnline
		// For now, simulate successful join after delay
		if (bJoinStarted && StateTimer > 5.0f) // Simulate 5 second join time
		{
			OnSessionJoined(true);
		}
	}
}

void FEasyOnlineJoinState::FindAndJoinSession()
{
	if (bJoinStarted)
	{
		return;
	}
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Searching for available sessions"));
	
	// TODO: Integrate with EasyOnline session discovery and joining
	// This would call into EasyOnline's session search and join functionality
	// For example:
	// if (UEasyOnlineManagerSubsystem* OnlineManager = GetWorld()->GetSubsystem<UEasyOnlineManagerSubsystem>())
	// {
	//     OnlineManager->FindSessions(
	//         FOnSessionsFoundDelegate::CreateLambda([this](const TArray<FSessionInfo>& Sessions)
	//         {
	//             if (Sessions.Num() > 0)
	//             {
	//                 OnlineManager->JoinSession(Sessions[0],
	//                     FOnSessionJoinedDelegate::CreateUObject(this, &FEasyOnlineJoinState::OnSessionJoined));
	//             }
	//         }));
	// }
	
	bJoinStarted = true;
}

void FEasyOnlineJoinState::OnSessionJoined(bool bWasSuccessful)
{
	bJoinCompleted = true;
	
	if (bWasSuccessful)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Successfully joined session"));
		FinishState(true);
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("Failed to join session"));
		FinishState(false);
	}
}

FString FEasyOnlineJoinState::GetStateName() const
{
	return TEXT("EasyOnlineJoinState");
}