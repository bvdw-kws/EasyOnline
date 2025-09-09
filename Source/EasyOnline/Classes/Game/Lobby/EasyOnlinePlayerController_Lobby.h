// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CommonPlayerController.h"

#include "EasyOnlinePlayerController_Lobby.generated.h"

UCLASS()
class EASYONLINE_API AEasyOnlinePlayerController_Lobby : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Lobby")
	bool CanOpenInviteDialog() const;
	
	UFUNCTION(BlueprintCallable, Category="Lobby")
	void OpenInviteDialog();
	
#pragma region RPC
public:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetIsSpectator(const bool bNewSpectator);
#pragma endregion RPC

	//~ Begin APlayerController Interface
protected:
	virtual void SetupInputComponent() override;
	virtual bool CanRestartPlayer() override;
	//~ End APlayerController Interface
};
