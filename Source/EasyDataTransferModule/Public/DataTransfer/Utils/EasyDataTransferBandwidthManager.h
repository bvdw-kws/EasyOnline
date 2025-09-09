// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

class APlayerState;
class UEasyDataTransferSettings;

/**
 * Manages bandwidth limiting and rate control for data transfers.
 * Provides centralized bandwidth management with per-player tracking.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferBandwidthManager
{
public:
	static constexpr float BandwidthResetInterval = 1.0f; // Reset every second

	FEasyDataTransferBandwidthManager();

	/**
	 * Check if player can send data within bandwidth limits.
	 * @param Player The player state
	 * @param DataSize Size of data to send in bytes
	 * @param CurrentTime Current world time
	 * @param Settings Global transfer settings
	 * @return True if player can send data
	 */
	bool CanSendDataForPlayer(APlayerState* Player, int32 DataSize, float CurrentTime, const UEasyDataTransferSettings* Settings);

	/**
	 * Update bandwidth tracking (call periodically).
	 * @param CurrentTime Current world time
	 */
	void UpdateBandwidthTracking(float CurrentTime);

	/**
	 * Reset all bandwidth tracking data.
	 */
	void Reset();

	/**
	 * Remove tracking data for a specific player.
	 * @param Player The player state to remove
	 */
	void RemovePlayer(APlayerState* Player);

private:
	// Per-player bandwidth usage tracking
	TMap<TObjectPtr<APlayerState>, float> PlayerBandwidthUsage;
	
	// Last time bandwidth was reset
	float LastBandwidthReset;
};