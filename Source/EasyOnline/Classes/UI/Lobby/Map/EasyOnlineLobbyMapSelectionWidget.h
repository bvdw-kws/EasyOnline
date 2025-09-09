// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonActivatableWidget.h"
#include "Interface/ExtendedCommonListWidgetTypes.h"

#include "EasyOnlineLobbyMapSelectionWidget.generated.h"

UCLASS(Abstract)
class EASYONLINE_API UEasyOnlineLobbyMapSelectionWidget : public UExtendedCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UEasyOnlineLobbyMapSelectionWidget();
	
	// UUserWidget implementation Begin
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// UUserWidget implementation End
	
	// UCommonActivatableWidget implementation Begin
public:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	// UCommonActivatableWidget implementation End
	
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category=BuildingSelection)
	TObjectPtr<class UExtendedCommonHierarchicalScrollBoxListWidget> ExtendedCommonHierarchicalScrollBoxListWidget_List;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<UEasyOnlineMapAsset*> SortedMapAssets;
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FExtendedCommonListWidgetItem> MapListItems;
	
	UFUNCTION()
	void OnSelectItem(int32 ItemIndex);
};
