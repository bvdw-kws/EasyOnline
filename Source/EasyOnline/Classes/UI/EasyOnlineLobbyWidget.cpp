// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineLobbyWidget.h"

#include "CommonButtonBase.h"
#include "ExtendedPrimaryGameLayout.h"
#include "ExtendedPrimaryGameLayoutTypes.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Game/Lobby/EasyOnlineGameState_Lobby.h"
#include "Game/Lobby/EasyOnlinePlayerController_Lobby.h"
#include "Game/Online/EasyOnlineHost.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Lobby/Map/EasyOnlineLobbyMapSelectionWidget.h"

UEasyOnlineLobbyWidget::UEasyOnlineLobbyWidget()
	: Super()
{
	bAutoActivate = true;
	bAutoRestoreFocus = true;
}

void UEasyOnlineLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CommonButton_StartGame)
	{
		CommonButton_StartGame->OnClicked().AddUObject(this, &ThisClass::StartGame);
	}

	if (CommonButton_LeaveLobby)
	{
		CommonButton_LeaveLobby->OnClicked().AddUObject(this, &ThisClass::DestroyLobby);
	}

	if (CommonButton_OpenInviteDialog)
	{
		CommonButton_OpenInviteDialog->OnClicked().AddUObject(this, &ThisClass::OpenInviteDialog);
	}
	
	if (CommonButton_OpenMapSelection)
	{
		CommonButton_OpenMapSelection->OnClicked().AddUObject(this, &ThisClass::OpenMapSelection);
	}
}

UWidget* UEasyOnlineLobbyWidget::NativeGetDesiredFocusTarget() const
{
	if (CommonButton_StartGame)
	{
		return CommonButton_StartGame;
	}
	return Super::NativeGetDesiredFocusTarget();
}

void UEasyOnlineLobbyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

bool UEasyOnlineLobbyWidget::CanOpenInviteDialog() const
{
	if (const AEasyOnlinePlayerController_Lobby* PlayerController = Cast<AEasyOnlinePlayerController_Lobby>(GetOwningPlayer()))
	{
		return PlayerController->CanOpenInviteDialog();
	}
	return false;
}

void UEasyOnlineLobbyWidget::OpenInviteDialog()
{
	if (AEasyOnlinePlayerController_Lobby* PlayerController = Cast<AEasyOnlinePlayerController_Lobby>(GetOwningPlayer()))
	{
		PlayerController->OpenInviteDialog();
	}
}

bool UEasyOnlineLobbyWidget::CanStartGame() const
{
	if (const AEasyOnlineGameState_Lobby* GameState = Cast<AEasyOnlineGameState_Lobby>(UGameplayStatics::GetGameState(this)))
	{
		return GameState->CanLobbyStartGame();
	}
	return false;
}

void UEasyOnlineLobbyWidget::StartGame()
{
	if (AEasyOnlineGameState_Lobby* GameState = Cast<AEasyOnlineGameState_Lobby>(UGameplayStatics::GetGameState(this)))
	{
		GameState->StartGameCountdown();
	}
}

void UEasyOnlineLobbyWidget::DestroyLobby()
{
	const UEasyOnlineManagerSubsystem* OnlineManager = GetGameInstance()->GetSubsystem<UEasyOnlineManagerSubsystem>();
	if (!ensureAlwaysMsgf(IsValid(OnlineManager),
			TEXT("%hs Invalid UEasyOnlineManagerSubsystem, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	UEasyOnlineHost* HostManager = OnlineManager->GetHostManager();
	if (!ensureAlwaysMsgf(IsValid(HostManager),
			TEXT("%hs Invalid UEasyOnlineHost, set UEasyOnlineGameInstance as the GameInstance"), __FUNCTION__))
	{
		return;
	}
	
	if (HostManager->IsHosting())
	{
		HostManager->StopHost();
	}
	
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		GameInstance->ReturnToMainMenu();
	}
}

void UEasyOnlineLobbyWidget::OpenMapSelection()
{	
	if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayout(GetOwningPlayer()))
	{
		RootLayout->PushWidgetToLayerStackAsync<UEasyOnlineLobbyMapSelectionWidget>(
			TAG_UI_LAYER_MENU, true, MapSelectionWidgetClass);
	}
}

bool UEasyOnlineLobbyWidget::IsSpectator() const
{
	if (const APlayerState* PlayerState = GetOwningPlayerState())
	{
		return PlayerState->IsSpectator();
	}
	return false;
}

void UEasyOnlineLobbyWidget::SetIsSpectator()
{
	if (AEasyOnlinePlayerController_Lobby* PlayerController = Cast<AEasyOnlinePlayerController_Lobby>(GetOwningPlayer()))
	{
		PlayerController->Server_SetIsSpectator(true);
	}
}

void UEasyOnlineLobbyWidget::SetIsPlaying()
{
	if (AEasyOnlinePlayerController_Lobby* PlayerController = Cast<AEasyOnlinePlayerController_Lobby>(GetOwningPlayer()))
	{
		PlayerController->Server_SetIsSpectator(false);
	}
}
