// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Utils/EasyDataTransferBandwidthManager.h"

#include "DataTransfer/Settings/EasyDataTransferSettings.h"
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogEasyDataTransferBandwidth, Log, All);

FEasyDataTransferBandwidthManager::FEasyDataTransferBandwidthManager()
	: LastBandwidthReset(0.0f)
{
}

bool FEasyDataTransferBandwidthManager::CanSendDataForPlayer(APlayerState* Player, int32 DataSize, float CurrentTime, const UEasyDataTransferSettings* Settings)
{
	if (!Player)
	{
		return false;
	}
	
	// Get global bandwidth settings
	if (!Settings || Settings->GlobalBandwidthLimit <= 0)
	{
		return true; // No bandwidth limits configured
	}
	
	// Update bandwidth tracking
	float& PlayerUsage = PlayerBandwidthUsage.FindOrAdd(Player);
	
	// Reset usage if enough time has passed (sliding window)
	if (CurrentTime - LastBandwidthReset > BandwidthResetInterval)
	{
		PlayerUsage = 0.0f;
	}
	
	// Check if adding this data would exceed limits
	const float ProjectedUsage = PlayerUsage + DataSize;
	const float BandwidthLimitPerSecond = static_cast<float>(Settings->GlobalBandwidthLimit);
	
	// Prevent overflow by clamping to reasonable values
	const float MaxReasonableUsage = 1000.0f * 1024.0f * 1024.0f; // 1GB max
	if (ProjectedUsage > MaxReasonableUsage)
	{
		UE_LOG(LogEasyDataTransferBandwidth, Warning, TEXT("%hs: Bandwidth usage overflow detected - Usage: %.0f, Resetting"), 
		       __FUNCTION__, ProjectedUsage);
		PlayerUsage = 0.0f;
		return true; // Allow the transfer after reset
	}
	
	if (ProjectedUsage > BandwidthLimitPerSecond)
	{
		UE_LOG(LogEasyDataTransferBandwidth, Verbose, TEXT("%hs: Player bandwidth limit exceeded - Usage: %.0f, Limit: %.0f"), 
		       __FUNCTION__, ProjectedUsage, BandwidthLimitPerSecond);
		return false;
	}
	
	// Update usage tracking
	PlayerUsage = ProjectedUsage;
	return true;
}

void FEasyDataTransferBandwidthManager::UpdateBandwidthTracking(float CurrentTime)
{
	// Reset bandwidth tracking periodically
	if (CurrentTime - LastBandwidthReset > BandwidthResetInterval)
	{
		PlayerBandwidthUsage.Reset();
		LastBandwidthReset = CurrentTime;
	}
}

void FEasyDataTransferBandwidthManager::Reset()
{
	PlayerBandwidthUsage.Reset();
	LastBandwidthReset = 0.0f;
}

void FEasyDataTransferBandwidthManager::RemovePlayer(APlayerState* Player)
{
	PlayerBandwidthUsage.Remove(Player);
}