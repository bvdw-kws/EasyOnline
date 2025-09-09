// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Gauntlet/EasyOnlineGauntletController.h"
#include "EasyGauntletTest.h"
#include "Engine/World.h"

UEasyOnlineGauntletController::UEasyOnlineGauntletController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UEasyOnlineGauntletController::OnInit()
{
	Super::OnInit();
	
	ParseOnlineParameters();
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("EasyOnlineGauntletController initialized - Role: %s"), 
		bIsHost ? TEXT("Host") : TEXT("Client"));
}

void UEasyOnlineGauntletController::ParseOnlineParameters()
{
	// Parse client mode (required for online tests)
	bool bParseFailed = false;
	if (FString ClientMode; FParse::Value(FCommandLine::Get(), TEXT("easy.clientmode="), ClientMode))
	{
		if (ClientMode.Equals(TEXT("Server"), ESearchCase::IgnoreCase) || 
			ClientMode.Equals(TEXT("Host"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogEasyGauntletTest, Display, TEXT("Client mode: Host/Server"));
			bIsHost = true;
		}
		else if (ClientMode.Equals(TEXT("Client"), ESearchCase::IgnoreCase))
		{
			UE_LOG(LogEasyGauntletTest, Display, TEXT("Client mode: Client"));
			bIsHost = false;
		}
		else
		{
			UE_LOG(LogEasyGauntletTest, Error, TEXT("Invalid client mode: %s. Expected Server/Host or Client"), *ClientMode);
			bParseFailed = true;
		}
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("Missing required parameter: easy.clientmode=[Server|Client]"));
		bParseFailed = true;
	}
	
	// Parse desync detection
	if (bool bDesyncDetect; FParse::Bool(FCommandLine::Get(), TEXT("easy.desyncDetect="), bDesyncDetect))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Desync detection: %s"), bDesyncDetect ? TEXT("Enabled") : TEXT("Disabled"));
		bDesyncDetectionEnabled = bDesyncDetect;
	}
	
	// Parse session timeout
	if (int32 InSessionTimeout; FParse::Value(FCommandLine::Get(), TEXT("easy.sessionTimeout="), InSessionTimeout))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Session timeout: %d seconds"), InSessionTimeout);
		SessionTimeout = InSessionTimeout;
	}
	
	if (bParseFailed)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("Failed to parse required online parameters - stopping test"));
		EndTest(1);
	}
}

void UEasyOnlineGauntletController::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);
	
	// Perform periodic network validation
	if (bDesyncDetectionEnabled)
	{
		LastNetworkValidationTime += DeltaTime;
		if (LastNetworkValidationTime >= NetworkValidationInterval)
		{
			LastNetworkValidationTime = 0.0f;
			ValidateNetworkState();
		}
	}
}

void UEasyOnlineGauntletController::ValidateNetworkState()
{
	// This would integrate with EasyOnline's desync detection system
	// For now, we'll add basic framework for future integration
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	// TODO: Integrate with EasyOnline desync detection when available
	// For example:
	// if (AEasyOnlineGameState* GameState = World->GetGameState<AEasyOnlineGameState>())
	// {
	//     if (GameState->HasDesyncDetected())
	//     {
	//         UE_LOG(LogEasyGauntletTest, Error, TEXT("Network desync detected!"));
	//         StopTesting(3); // Exit code 3 for desync
	//     }
	// }
}