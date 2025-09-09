// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineLobbyPlayerCardWidget.h"

#include "CommonRichTextBlock.h"
#include "Game/Lobby/EasyOnlinePlayerState_Lobby.h"
#include "GameFramework/PlayerState.h"

#define LOCTEXT_NAMESPACE "UEasyOnlineLobbyPlayerCardWidget"

UEasyOnlineLobbyPlayerCardWidget::UEasyOnlineLobbyPlayerCardWidget()
	: Super()
{
	EmptyPlayerText = NSLOCTEXT("UEasyOnlineLobbyPlayerCardWidget", "EmptyPlayerText", "Empty player slot");
}

void UEasyOnlineLobbyPlayerCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetFromEmptyPlayer();
}

void UEasyOnlineLobbyPlayerCardWidget::SetFromPlayerState(AEasyOnlinePlayerState_Lobby* InPlayerState)
{
	if (PlayerState == InPlayerState)
	{
		return;
	}
	
	PlayerState = InPlayerState;
	
	if (!IsValid(PlayerState))
	{
		SetFromEmptyPlayer();
		return;
	}

	if (CommonRichTextBlock_Name)
	{
		CommonRichTextBlock_Name->SetText(FText::FromString(PlayerState->GetPlayerName()));
	}
}

void UEasyOnlineLobbyPlayerCardWidget::SetFromEmptyPlayer()
{
	PlayerState = nullptr;
	
	if (CommonRichTextBlock_Name)
	{
		CommonRichTextBlock_Name->SetText(EmptyPlayerText);
	}
}

#undef LOCTEXT_NAMESPACE
