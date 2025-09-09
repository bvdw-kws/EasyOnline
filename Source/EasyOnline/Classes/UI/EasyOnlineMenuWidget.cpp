// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineMenuWidget.h"

#include "Components/CheckBox.h"
#include "Settings/EasyOnlineSettings.h"
#include "Utility/EasyOnlineFunctionLibrary.h"

void UEasyOnlineMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CheckBox_PrivateMode)
	{
		const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
		CheckBox_PrivateMode->SetIsChecked(EasyOnlineSettings->bIsPrivateMode);
		CheckBox_PrivateMode->OnCheckStateChanged.AddDynamic(this, &ThisClass::SetPrivateMode);
	}
}

void UEasyOnlineMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (CheckBox_PrivateMode)
	{
		CheckBox_PrivateMode->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::SetPrivateMode);
	}
}

bool UEasyOnlineMenuWidget::IsPrivateMode() const
{
	if (CheckBox_PrivateMode)
	{
		return CheckBox_PrivateMode->IsChecked();
	}
	else
	{		
		const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
		return EasyOnlineSettings->bIsPrivateMode;
	}
}

void UEasyOnlineMenuWidget::SetPrivateMode(bool bIsChecked)
{
	if (CheckBox_PrivateMode && CheckBox_PrivateMode->IsChecked() != bIsChecked)
	{
		CheckBox_PrivateMode->SetIsChecked(bIsChecked);
	}
}

void UEasyOnlineMenuWidget::CreateLobby()
{
	UEasyOnlineFunctionLibrary::CreateLobby(this, GetOwningPlayer(), IsPrivateMode());
}

void UEasyOnlineMenuWidget::QuickJoin()
{
	UEasyOnlineFunctionLibrary::QuickJoin(this, GetOwningPlayer());
}

void UEasyOnlineMenuWidget::QuickHost()
{
	UEasyOnlineFunctionLibrary::QuickHost(this, GetOwningPlayer(), IsPrivateMode());
}
