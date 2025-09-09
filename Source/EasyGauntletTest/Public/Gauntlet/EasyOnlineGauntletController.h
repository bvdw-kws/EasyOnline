// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Gauntlet/EasyGauntletController.h"
#include "EasyOnlineGauntletController.generated.h"

/**
 * Enhanced gauntlet controller for multiplayer/online testing
 * Provides host/client role management and network validation
 */
UCLASS()
class EASYGAUNTLETTEST_API UEasyOnlineGauntletController : public UEasyGauntletController
{
	GENERATED_BODY()

public:
	UEasyOnlineGauntletController(const FObjectInitializer& ObjectInitializer);
	
	virtual void OnInit() override;
	virtual void OnTick(float DeltaTime) override;

	// Role management
	FORCEINLINE bool IsHost() const { return bIsHost; }
	FORCEINLINE bool IsClient() const { return !bIsHost; }
	
	// Network validation
	FORCEINLINE bool IsDesyncDetectionEnabled() const { return bDesyncDetectionEnabled; }
	FORCEINLINE int32 GetSessionTimeout() const { return SessionTimeout; }

protected:
	virtual void ParseOnlineParameters();

private:
	void ValidateNetworkState();
	
	// Role configuration
	bool bIsHost = false;
	
	// Network validation
	bool bDesyncDetectionEnabled = false;
	int32 SessionTimeout = 60; // seconds
	float NetworkValidationInterval = 1.0f; // seconds
	float LastNetworkValidationTime = 0.0f;
};