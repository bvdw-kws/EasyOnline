// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "EasyOnlineHUD_Lobby.generated.h"

UCLASS()
class EASYONLINE_API AEasyOnlineHUD_Lobby : public AHUD
{
	GENERATED_BODY()
	
	//~ Begin AActor Interface
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
