// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "DataTransfer/EasyDataTransferTypes.h"

class UEasyDataTransferSubsystem;
class UEasyDataTransferSettings;
class UEasyDataTransferPlayerComponent;
class APlayerState;

/**
 * Manages validation logic for data transfers.
 * Handles transfer validation, player limits, channel whitelist, and component validation.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferValidationManager
{
public:
	explicit FEasyDataTransferValidationManager(UEasyDataTransferSubsystem& InSubsystem);

	/**
	 * Validate a new transfer request.
	 * @param ChannelName Channel name to validate
	 * @param Settings Transfer settings to validate
	 * @param Data Data to validate
	 * @param Sender Sender player state
	 * @param Receiver Receiver player state
	 * @param OutErrorMessage Error message if validation fails
	 * @return True if validation passed
	 */
	bool ValidateNewTransfer(const FString& ChannelName, const FEasyDataTransferOptions& Settings, 
	                         const TArray<uint8>& Data, APlayerState* Sender, APlayerState* Receiver,
	                         FString& OutErrorMessage);

	/**
	 * Validate transfer state for sending operations.
	 * @param Handle Transfer handle
	 * @param OutTransferState Output transfer state if valid
	 * @return True if transfer is valid for sending
	 */
	bool ValidateTransferForSending(int32 Handle, FEasyDataTransferState*& OutTransferState);

	/**
	 * Get and validate receiver component for transfer.
	 * @param Handle Transfer handle
	 * @param TransferState Transfer state
	 * @param OutComponent Output receiver component if valid
	 * @param OutErrorMessage Error message if validation fails
	 * @return True if receiver component is valid
	 */
	bool ValidateReceiverComponent(int32 Handle, FEasyDataTransferState* TransferState, 
	                               UEasyDataTransferPlayerComponent*& OutComponent,
	                               FString& OutErrorMessage);

	/**
	 * Check if player has reached concurrent transfer limit.
	 * @param PlayerState Player to check
	 * @param bAsSender Check as sender (true) or receiver (false)
	 * @return True if player has too many active transfers
	 */
	bool HasReachedTransferLimit(APlayerState* PlayerState, bool bAsSender) const;

	/**
	 * Map error message string to error enum.
	 * @param ErrorMessage The error message
	 * @return The corresponding error enum
	 */
	static EDataTransferError MapErrorMessageToError(const FString& ErrorMessage);

private:
	/**
	 * Validate channel name against whitelist.
	 * @param ChannelName Channel name to validate
	 * @param Settings Global transfer settings
	 * @return True if channel is allowed
	 */
	bool ValidateChannelWhitelist(const FString& ChannelName, const UEasyDataTransferSettings* Settings) const;

	/**
	 * Validate data size limits.
	 * @param DataSize Size of data to validate
	 * @param Settings Global transfer settings
	 * @return True if data size is within limits
	 */
	bool ValidateDataSize(int32 DataSize, const UEasyDataTransferSettings* Settings) const;

	/**
	 * Validate global concurrent transfer limits.
	 * @param Settings Global transfer settings
	 * @return True if within global limits
	 */
	bool ValidateGlobalLimits(const UEasyDataTransferSettings* Settings) const;

	/**
	 * Count active transfers for a player.
	 * @param PlayerState Player to count transfers for
	 * @param bAsSender Count as sender (true) or receiver (false)
	 * @return Number of active transfers
	 */
	int32 CountActiveTransfersForPlayer(APlayerState* PlayerState, bool bAsSender) const;

private:
	// Reference to parent subsystem
	UEasyDataTransferSubsystem& Subsystem;
};