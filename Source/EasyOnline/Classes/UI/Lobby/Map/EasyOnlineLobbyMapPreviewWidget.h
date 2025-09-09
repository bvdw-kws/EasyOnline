// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonUserWidget.h"

#include "EasyOnlineLobbyMapPreviewWidget.generated.h"

/**
 * Widget to preview the map currently selected in the lobby.
 * The map choice should be stored in the game state so every player can know about it.
 */
UCLASS(Abstract)
class EASYONLINE_API UEasyOnlineLobbyMapPreviewWidget : public UExtendedCommonUserWidget
{
	GENERATED_BODY()

	//~ Begin UUserWidget interface
public:	
	virtual void NativeConstruct() override;
	//~ End UUserWidget interface

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<class UCommonLazyImage> CommonLazyImage_MapPreview;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<class UCommonRichTextBlock> CommonRichTextBlock_MapName;

private:
	FDelegateHandle OnMapAssetsLoadedDelegateHandle;
	void RefreshMapPreview();
	
	void DisplayPreviewForMap(const FName& MapID);
};
