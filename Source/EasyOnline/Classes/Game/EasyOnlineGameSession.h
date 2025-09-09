// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "GameFramework/GameSession.h"

#include "EasyOnlineGameSession.generated.h"

UCLASS(Blueprintable)
class EASYONLINE_API AEasyOnlineGameSession : public AGameSession
{
	GENERATED_BODY()

	AEasyOnlineGameSession(const FObjectInitializer& ObjectInitializer);
};
