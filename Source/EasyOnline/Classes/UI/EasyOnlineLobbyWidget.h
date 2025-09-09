// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonActivatableWidget.h"

#include "EasyOnlineLobbyWidget.generated.h"

class UCommonButtonBase;

UCLASS(Abstract)
class EASYONLINE_API UEasyOnlineLobbyWidget : public UExtendedCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UEasyOnlineLobbyWidget();

	//~ Begin UCommonActivatableWidget interface
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	//~ End UCommonActivatableWidget interface
	
	//~ Begin UUserWidget interface
public:	
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget interface

protected:
	UPROPERTY(EditDefaultsOnly, Category=Lobby)
	TSoftClassPtr<class UEasyOnlineLobbyMapSelectionWidget> MapSelectionWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<class UEasyOnlineLobbyPlayerListWidget> PlayerList_Players;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CommonButton_StartGame;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CommonButton_LeaveLobby;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CommonButton_OpenInviteDialog;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> CommonButton_OpenMapSelection;
	
	UFUNCTION(BlueprintPure, Category=Lobby)
	bool CanStartGame() const;
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void StartGame();
	
	UFUNCTION(BlueprintPure, Category=Lobby)
	bool CanOpenInviteDialog() const;
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void OpenInviteDialog();
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void DestroyLobby();
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void OpenMapSelection();
	
	UFUNCTION(BlueprintPure, Category=Lobby)
	bool IsSpectator() const;
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void SetIsSpectator();
	
	UFUNCTION(BlueprintCallable, Category=Lobby)
	void SetIsPlaying();

};
