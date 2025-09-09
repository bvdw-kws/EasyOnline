// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "GameFramework/GameMode.h"

#include "EasyOnlineGameModeBase.generated.h"

UCLASS()
class EASYONLINE_API AEasyOnlineGameModeBase :
	public AGameMode
{
	GENERATED_UCLASS_BODY()

	//~ Begin AGameModeBase Interface
public:
	virtual void GenericPlayerInitialization(AController* C) override;
	//~ End AGameModeBase Interface
};
