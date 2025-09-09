// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonUserWidget.h"

#include "EasyOnlineLobbyPlayerListWidget.generated.h"

class UEasyOnlineLobbyPlayerCardWidget;

#define EASY_ONLINE_PLAYER_CARD_MAX 4

UCLASS(Abstract)
class EASYONLINE_API UEasyOnlineLobbyPlayerListWidget : public UExtendedCommonUserWidget
{
	GENERATED_BODY()

public:
	UEasyOnlineLobbyPlayerListWidget();

	//~ Begin UUserWidget interface
public:	
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget interface

protected:
	UPROPERTY(EditAnywhere, Category=Lobby)
	FText PlayerCountText;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category=Lobby)
	TObjectPtr<class UCommonRichTextBlock> CommonRichTextBlock_PlayerCount;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category=Lobby)
	TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> PlayerCard_1;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category=Lobby)
	TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> PlayerCard_2;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category=Lobby)
	TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> PlayerCard_3;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category=Lobby)
	TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> PlayerCard_4;

	UFUNCTION(BlueprintCallable, Category=Lobby)
	void SetPlayerCount(int32 PlayerCount, int32 MaxPlayerCount) const;
	
	void UpdatePlayerCards() const;
	TObjectPtr<UEasyOnlineLobbyPlayerCardWidget> GetPlayerCard(int32 PlayerIndex) const;
};
