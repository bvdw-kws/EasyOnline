// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineHUD_Lobby.h"

#include "ExtendedPrimaryGameLayoutTypes.h"
#include "PrimaryGameLayout.h"
#include "Settings/EasyOnlineSettings.h"
#include "UI/EasyOnlineLobbyWidget.h"

void AEasyOnlineHUD_Lobby::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;

		const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();

		if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayout(PC))
		{
			RootLayout->PushWidgetToLayerStackAsync<UEasyOnlineLobbyWidget>(
				TAG_UI_LAYER_MENU, true, EasyOnlineSettings->LobbyWidget);
		}
	}
}

void AEasyOnlineHUD_Lobby::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
