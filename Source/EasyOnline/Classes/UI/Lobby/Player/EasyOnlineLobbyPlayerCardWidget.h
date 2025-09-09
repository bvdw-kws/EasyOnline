// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonUserWidget.h"

#include "EasyOnlineLobbyPlayerCardWidget.generated.h"

class AEasyOnlinePlayerState_Lobby;

UCLASS(Abstract)
class EASYONLINE_API UEasyOnlineLobbyPlayerCardWidget : public UExtendedCommonUserWidget
{
	GENERATED_BODY()

public:
	UEasyOnlineLobbyPlayerCardWidget();

	//~ Begin UUserWidget interface
public:	
	virtual void NativeConstruct() override;
	//~ End UUserWidget interface

public:
	void SetFromPlayerState(AEasyOnlinePlayerState_Lobby* InPlayerState);
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void SetFromEmptyPlayer();
	
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category=Lobby)
	TObjectPtr<class UCommonRichTextBlock> CommonRichTextBlock_Name;

	UPROPERTY(EditAnywhere, Category=Lobby)
	int32 PlayerIndex = 0;
	
	UPROPERTY(EditAnywhere, Category=Lobby)
	FText EmptyPlayerText;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<AEasyOnlinePlayerState_Lobby> PlayerState;
};
