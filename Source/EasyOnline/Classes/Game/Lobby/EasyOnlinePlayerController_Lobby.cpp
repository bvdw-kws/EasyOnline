// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlinePlayerController_Lobby.h"

#include "Components/InputComponent.h"
#include "EasyOnlineGameState_Lobby.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemUtils.h"
#include "Settings/EasyOnlineSettings.h"

void AEasyOnlinePlayerController_Lobby::SetupInputComponent()
{
	Super::SetupInputComponent();

	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	if (EasyOnlineSettings->bAutoOpenInviteDialog)
	{
		OpenInviteDialog();
	}
}

bool AEasyOnlinePlayerController_Lobby::CanRestartPlayer()
{
	return false;
}

bool AEasyOnlinePlayerController_Lobby::CanOpenInviteDialog() const
{
	if (!HasAuthority())
	{
		return false;
	}
	
	const IOnlineExternalUIPtr ExternalUIPtr = Online::GetExternalUIInterface(GetWorld());
	if (ExternalUIPtr.IsValid() == false)
	{
		return false;
	}

	return true;
}

void AEasyOnlinePlayerController_Lobby::OpenInviteDialog()
{
	if (!CanOpenInviteDialog())
	{
		UE_LOG(LogTemp, Warning, TEXT("%hs Support open invite dialog for host only"), __FUNCTION__);
		return;
	}
	
	const IOnlineExternalUIPtr ExternalUIPtr = Online::GetExternalUIInterface(GetWorld());
	if (ExternalUIPtr.IsValid() == false)
	{
		UE_LOG(LogTemp, Log, TEXT("%hs ExternalUIPtr doesn't exists"), __FUNCTION__);
		return;
	}

	if (const ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if(ExternalUIPtr->ShowInviteUI(LocalPlayer->GetLocalPlayerIndex(), NAME_GameSession))
		{
			UE_LOG(LogTemp, Log, TEXT("%hs Opened invite dialog"), __FUNCTION__);
			return;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%hs Failed to open invite dialog"), __FUNCTION__);
}

#pragma region RPC
void AEasyOnlinePlayerController_Lobby::Server_SetIsSpectator_Implementation(const bool bNewSpectator)
{
#if WITH_SERVER_CODE
	AEasyOnlineGameState_Lobby* GameState = Cast<AEasyOnlineGameState_Lobby>(UGameplayStatics::GetGameState(this));
	if (!IsValid(GameState) || GameState->IsGameStartCountdown())
	{
		return;
	}

	if (!bNewSpectator && !GameState->CanAddPlayingPlayer())
	{
		return;
	}
	
	if (IsValid(PlayerState) && PlayerState->IsSpectator() != bNewSpectator)
	{
		PlayerState->SetIsSpectator(bNewSpectator);
	}
#endif // WITH_SERVER_CODE
}

bool AEasyOnlinePlayerController_Lobby::Server_SetIsSpectator_Validate(const bool bNewSpectator)
{
	return true;
}
#pragma endregion RPC
