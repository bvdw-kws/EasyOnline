// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonActivatableWidget.h"

#include "EasyOnlineMenuWidget.generated.h"

class UCheckBox;
class UCommonButtonBase;

UCLASS(Abstract)
class EASYONLINE_API UEasyOnlineMenuWidget : public UExtendedCommonActivatableWidget
{
	GENERATED_BODY()

	//~ Begin UUserWidget interface
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget interface

public:
	UFUNCTION(BlueprintPure, Category=Lobby)
	bool IsPrivateMode() const;

	UFUNCTION(BlueprintCallable, Category=Lobby)
	void SetPrivateMode(bool bIsChecked);

protected:
	UPROPERTY(BlueprintReadOnly, meta=(OptionalBindWidget))
	TObjectPtr<UCheckBox> CheckBox_PrivateMode;

private:
	UFUNCTION(BlueprintCallable)
	void CreateLobby();
		
	UFUNCTION(BlueprintCallable)
	void QuickJoin();

	UFUNCTION(BlueprintCallable)
	void QuickHost();
};
