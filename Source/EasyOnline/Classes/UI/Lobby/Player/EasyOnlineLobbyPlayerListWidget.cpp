// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineLobbyPlayerListWidget.h"

#include "CommonRichTextBlock.h"
#include "EasyOnlineLobbyPlayerCardWidget.h"
#include "Game/Lobby/EasyOnlineGameState_Lobby.h"
#include "Game/Lobby/EasyOnlinePlayerState_Lobby.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "UEasyOnlineLobbyPlayerListWidget"

UEasyOnlineLobbyPlayerListWidget::UEasyOnlineLobbyPlayerListWidget()
	: Super()
{
	PlayerCountText = NSLOCTEXT("UEasyOnlineLobbyPlayerListWidget", "PlayerCountText", "{PlayerCount} / {PlayerMax}");
}

void UEasyOnlineLobbyPlayerListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	UpdatePlayerCards();
}

void UEasyOnlineLobbyPlayerListWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	UpdatePlayerCards();
}

void UEasyOnlineLobbyPlayerListWidget::SetPlayerCount(int32 PlayerCount, int32 MaxPlayerCount) const
{
	if (CommonRichTextBlock_PlayerCount)
	{
		FFormatNamedArguments Args;
		Args.Add("PlayerCount", PlayerCount);
		Args.Add("PlayerMax", MaxPlayerCount);
		
		CommonRichTextBlock_PlayerCount->SetText(FText::Format(PlayerCountText, Args));
	}
}

void UEasyOnlineLobbyPlayerListWidget::UpdatePlayerCards() const
{
	AEasyOnlineGameState_Lobby* LobbyGameState = Cast<AEasyOnlineGameState_Lobby>(UGameplayStatics::GetGameState(this));
	if (!IsValid(LobbyGameState))
	{
		return;
	}

	int32 PlayerCount = 0;
	
	for (int32 PlayerIndex = 0; PlayerIndex < EASY_ONLINE_PLAYER_CARD_MAX; PlayerIndex++)
	{
		const TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> PlayerCard = GetPlayerCard(PlayerIndex);
		if (!PlayerCard)
		{
			continue;
		}
		
		AEasyOnlinePlayerState_Lobby* PlayerState = LobbyGameState->PlayerArray.IsValidIndex(PlayerIndex) ?
			Cast<AEasyOnlinePlayerState_Lobby>(LobbyGameState->PlayerArray[PlayerIndex]) : nullptr;
		if (IsValid(PlayerState) && !PlayerState->IsInactive())
		{
			PlayerCard->SetFromPlayerState(PlayerState);
			PlayerCount++;
		}
		else
		{
			PlayerCard->SetFromEmptyPlayer();
		}
	}

	SetPlayerCount(PlayerCount, LobbyGameState->GetLobbyNumPublicConnections());
}

TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> UEasyOnlineLobbyPlayerListWidget::GetPlayerCard(int32 PlayerIndex) const
{
	switch (PlayerIndex)
	{
	case 0:
		return PlayerCard_1;
		
	case 1:
		return PlayerCard_2;
		
	case 2:
		return PlayerCard_3;
		
	case 3:
		return PlayerCard_4;
		
	default:
		checkNoEntry();
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
