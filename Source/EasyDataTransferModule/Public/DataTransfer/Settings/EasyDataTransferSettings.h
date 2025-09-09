// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/DeveloperSettings.h"

#include "EasyDataTransferSettings.generated.h"

/**
 * Settings for the EasyDataTransfer system.
 * Follows the same pattern as UEasyOnlineSettings.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Easy Data Transfer"))
class EASYDATATRANSFERMODULE_API UEasyDataTransferSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEasyDataTransferSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Default transfer settings
	UPROPERTY(EditAnywhere, config, Category="Transfer Defaults")
	int32 DefaultChunkSize = 1024;
	
	UPROPERTY(EditAnywhere, config, Category="Transfer Defaults")
	float DefaultTimeoutSeconds = 30.0f;
	
	UPROPERTY(EditAnywhere, config, Category="Transfer Defaults")
	int32 DefaultMaxRetries = 3;
	
	// Global limits
	UPROPERTY(EditAnywhere, config, Category="Limits", meta=(ClampMin=1, ClampMax=1000))
	int32 MaxTransferSize = 104857600; // 100MB
	
	UPROPERTY(EditAnywhere, config, Category="Limits", meta=(ClampMin=1, ClampMax=20))
	int32 MaxConcurrentTransfersPerPlayer = 5;
	
	UPROPERTY(EditAnywhere, config, Category="Limits", meta=(ClampMin=1, ClampMax=200))
	int32 MaxConcurrentTransfersTotal = 50;

	// Bandwidth management
	UPROPERTY(EditAnywhere, config, Category="Bandwidth")
	int32 GlobalBandwidthLimit = 0; // 0 = unlimited
	
	UPROPERTY(EditAnywhere, config, Category="Bandwidth", meta=(ClampMin=1, ClampMax=10))
	int32 DefaultSlidingWindowSize = 5;

	// Channel whitelist
	UPROPERTY(EditAnywhere, config, Category="Security")
	bool bRequireChannelWhitelist = false;
	
	UPROPERTY(EditAnywhere, config, Category="Security", meta=(EditCondition=bRequireChannelWhitelist))
	TArray<FString> AllowedChannelNames;
};