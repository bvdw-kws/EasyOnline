// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineGameInstance.h"

#include "Game/Online/EasyOnlineSessionClient.h"

void UEasyOnlineGameInstance::Init()
{
	Super::Init();
}

void UEasyOnlineGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UEasyOnlineGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
}

TSubclassOf<UOnlineSession> UEasyOnlineGameInstance::GetOnlineSessionClass()
{
	return UEasyOnlineSessionClient::StaticClass();
}

void UEasyOnlineGameInstance::OnClientJoinSession(const FName& SessionName, bool bWasSuccessful, bool bForceJoinByService, const FString& Url)
{
	if (SessionName != NAME_GameSession)
	{
		return;
	}

	if (bWasSuccessful == false)
	{
		// TODO: We can add error popup here when failed to join any session
		return;
	}

	// Note: We can check condition to travel or add any popup needs here
	// This event can be called either user request or platform broadcasting.
	// bForceJoinByService is true when force joined by platform service
	if (APlayerController* PlayerController = GetFirstLocalPlayerController())
	{
		UE_LOG(LogTemp, Log, TEXT("%hs Joined session %s, Travelling to %s"),
			__FUNCTION__, *SessionName.ToString(), *Url);
		
		PlayerController->ClientTravel(Url, TRAVEL_Absolute);
	}
}
