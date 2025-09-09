// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "TestStates/EasyOnlineHostState.h"
#include "Gauntlet/EasyOnlineGauntletController.h"
#include "EasyGauntletTest.h"

FEasyOnlineHostState::FEasyOnlineHostState(UEasyOnlineGauntletController* InController, int32 InMaxPlayers, float InTimeout)
	: FEasyGauntletState(InController, InTimeout)
	, OnlineController(InController)
	, MaxPlayers(InMaxPlayers)
{
	check(OnlineController);
	check(MaxPlayers > 0);
}

void FEasyOnlineHostState::OnEnterState()
{
	FEasyGauntletState::OnEnterState();
	
	if (!OnlineController->IsHost())
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("EasyOnlineHostState can only be used by host instances"));
		FinishState(false);
		return;
	}
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Starting session creation for %d players"), MaxPlayers);
	CreateSession();
}

void FEasyOnlineHostState::OnTick(float DeltaTime)
{
	FEasyGauntletState::OnTick(DeltaTime);
	
	// Check if session creation is complete
	if (bSessionCreationStarted && !bSessionCreated)
	{
		// TODO: Poll EasyOnline session status
		// This would integrate with EasyOnline's session management system
		// For now, we'll simulate success after a short delay
		if (StateTimer > 2.0f) // Simulate 2 second creation time
		{
			OnSessionCreated(true);
		}
	}
}

void FEasyOnlineHostState::CreateSession()
{
	if (bSessionCreationStarted)
	{
		return;
	}
	
	bSessionCreationStarted = true;
	
	// TODO: Integrate with EasyOnline session creation
	// This would call into EasyOnline's host session functionality
	// For example:
	// if (UEasyOnlineManagerSubsystem* OnlineManager = GetWorld()->GetSubsystem<UEasyOnlineManagerSubsystem>())
	// {
	//     OnlineManager->CreateSession(MaxPlayers, 
	//         FOnSessionCreatedDelegate::CreateUObject(this, &FEasyOnlineHostState::OnSessionCreated));
	// }
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Session creation initiated"));
}

void FEasyOnlineHostState::OnSessionCreated(bool bWasSuccessful)
{
	bSessionCreated = true;
	
	if (bWasSuccessful)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Session created successfully"));
		FinishState(true);
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("Failed to create session"));
		FinishState(false);
	}
}

FString FEasyOnlineHostState::GetStateName() const
{
	return FString::Printf(TEXT("EasyOnlineHostState(%d players)"), MaxPlayers);
}