// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineLobbyMapPreviewWidget.h"

#include "CommonLazyImage.h"
#include "CommonRichTextBlock.h"
#include "Component/EasyOnlineLobbyModeComponent.h"
#include "Data/Map/EasyOnlineMapAsset.h"
#include "Data/Subsystems/EasyOnlineMapSubsystem.h"
#include "Game/Lobby/EasyOnlineGameState_Lobby.h"
#include "Kismet/GameplayStatics.h"

void UEasyOnlineLobbyMapPreviewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(GetWorld());
	if (MapSubsystem.HasLoadedMapAssets())
	{
		RefreshMapPreview();
	}
	else
	{
		OnMapAssetsLoadedDelegateHandle = MapSubsystem.AddEasyOnlineOnMapAssetsLoadedDelegate_Handle(
			FEasyOnlineOnMapAssetsLoadedDelegate::CreateUObject(this, &ThisClass::RefreshMapPreview));
	}

	AEasyOnlineGameState_Lobby* GameState = Cast<AEasyOnlineGameState_Lobby>(
		UGameplayStatics::GetGameState(this));
	if (IsValid(GameState))
	{
		GameState->GetLobbyModeComponentChecked()->AddEasyOnlineOnMapChangedDelegate_Handle(
			FEasyOnlineOnMapChangedDelegate::CreateUObject(this, &ThisClass::DisplayPreviewForMap));
	}
}

void UEasyOnlineLobbyMapPreviewWidget::RefreshMapPreview()
{
	UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(GetWorld());
	
	if (OnMapAssetsLoadedDelegateHandle.IsValid())
	{
		MapSubsystem.ClearEasyOnlineOnMapAssetsLoadedDelegate_Handle(OnMapAssetsLoadedDelegateHandle);
	}
	
	const AEasyOnlineGameState_Lobby* GameState = Cast<AEasyOnlineGameState_Lobby>(
		UGameplayStatics::GetGameState(this));
	if (IsValid(GameState))
	{
		DisplayPreviewForMap(GameState->GetLobbyModeComponentChecked()->GetMapID());
	}
}

void UEasyOnlineLobbyMapPreviewWidget::DisplayPreviewForMap(const FName& MapID)
{
	UEasyOnlineMapSubsystem& MapSubsystem = UEasyOnlineMapSubsystem::GetRef(GetWorld());
	if (MapID.IsNone() || !MapSubsystem.HasLoadedMapAssets())
	{
		return;
	}

	const TObjectPtr<const UEasyOnlineMapAsset> MapAsset = MapSubsystem.GetMapAsset(MapID);
	if (!ensureAlwaysMsgf(MapAsset,
		TEXT("%hs Failed to find map asset for map ID: %s"), __FUNCTION__, *MapID.ToString()))
	{
		return;
	}

	const FEasyOnlineMapData& MapData = MapAsset->MapData;
	
	if (CommonLazyImage_MapPreview)
	{
		CommonLazyImage_MapPreview->SetBrushFromLazyTexture(MapData.PreviewTexture, true);
	}

	if (CommonRichTextBlock_MapName)
	{
		CommonRichTextBlock_MapName->SetText(MapData.DisplayName);
	}
}
