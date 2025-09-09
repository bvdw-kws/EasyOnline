// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineLobbyMapSelectionWidget.h"

#include "Component/EasyOnlineLobbyModeComponent.h"
#include "Data/Map/EasyOnlineMapAsset.h"
#include "Game/Lobby/EasyOnlineGameState_Lobby.h"
#include "Interface/ExtendedCommonListWidgetInterface.h"
#include "Kismet/GameplayStatics.h"
#include "List/ExtendedCommonHierarchicalScrollBoxListWidget.h"
#include "Utility/EasyOnlineFunctionLibrary.h"

UEasyOnlineLobbyMapSelectionWidget::UEasyOnlineLobbyMapSelectionWidget()
	: Super()
{
	bIsBackHandler = true;
	bIsBackActionDisplayedInActionBar = true;
}

void UEasyOnlineLobbyMapSelectionWidget::NativeConstruct()
{
	SortedMapAssets = UEasyOnlineFunctionLibrary::GetSortedMapAssets(this);

	MapListItems.SetNum(SortedMapAssets.Num());
	for (int32 BuildingIndex = 0; BuildingIndex < SortedMapAssets.Num(); BuildingIndex++)
	{
		const UEasyOnlineMapAsset* BuildingDeviceAsset = SortedMapAssets[BuildingIndex];
		if (!ensure(IsValid(BuildingDeviceAsset)))
		{
			continue;
		}
		
		FExtendedCommonListWidgetItem& Item = MapListItems[BuildingIndex];
		Item.LocalizedName = BuildingDeviceAsset->MapData.DisplayName;
		Item.LocalizedDescription = BuildingDeviceAsset->MapData.DisplayDescription;
		Item.IconTexture = BuildingDeviceAsset->MapData.PreviewTexture;
	}
	
	if (ExtendedCommonHierarchicalScrollBoxListWidget_List)
	{
		IExtendedCommonListWidgetInterface::Execute_SetItems(
			ExtendedCommonHierarchicalScrollBoxListWidget_List, MapListItems);
		ExtendedCommonHierarchicalScrollBoxListWidget_List->OnItemSelectedEvent().AddDynamic(
			this, &ThisClass::OnSelectItem);
	}
	
	Super::NativeConstruct();
}

void UEasyOnlineLobbyMapSelectionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	SortedMapAssets.Empty();

	if (ExtendedCommonHierarchicalScrollBoxListWidget_List)
	{
		ExtendedCommonHierarchicalScrollBoxListWidget_List->OnItemSelectedEvent().RemoveAll(this);
	}
}

UWidget* UEasyOnlineLobbyMapSelectionWidget::NativeGetDesiredFocusTarget() const
{
	return ExtendedCommonHierarchicalScrollBoxListWidget_List;
}

void UEasyOnlineLobbyMapSelectionWidget::OnSelectItem(int32 ItemIndex)
{
	if (ensure(SortedMapAssets.IsValidIndex(ItemIndex)))
	{
		const TObjectPtr<const UEasyOnlineMapAsset>& BuildingDeviceAsset = SortedMapAssets[ItemIndex];
		if (ensure(BuildingDeviceAsset))
		{
			AEasyOnlineGameState_Lobby* GameState = Cast<AEasyOnlineGameState_Lobby>(
				UGameplayStatics::GetGameState(this));
			if (IsValid(GameState))
			{
				GameState->GetLobbyModeComponentChecked()->SetMapID(BuildingDeviceAsset->MapID);
			}
		}
	}
	
	HandleBackAction();
}
