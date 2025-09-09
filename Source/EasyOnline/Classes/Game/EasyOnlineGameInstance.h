// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CommonGameInstance.h"
#include "Engine/GameInstance.h"

#include "EasyOnlineGameInstance.generated.h"

UCLASS()
class EASYONLINE_API UEasyOnlineGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:
	//~ Begin UGameInstance Interface
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;
	virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;
	//~ End UGameInstance Interface

private:
	/** Called when joined the session with result and resolved connect URL */
	void OnClientJoinSession(const FName& SessionName, bool bWasSuccessful, bool bForceJoinByService, const FString& Url);

};
