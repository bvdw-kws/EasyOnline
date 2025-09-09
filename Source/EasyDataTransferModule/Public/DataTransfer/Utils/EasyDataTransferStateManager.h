// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "DataTransfer/EasyDataTransferTypes.h"

class UEasyDataTransferSubsystem;

/**
 * Manages transfer state lifecycle and conflicts.
 * Handles the complex logic of creating and managing transfer states,
 * especially for receiver-side state creation and sender/receiver conflicts.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferStateManager
{
public:
	explicit FEasyDataTransferStateManager(UEasyDataTransferSubsystem& InSubsystem);

	/**
	 * Handle transfer started notification, managing existing state conflicts.
	 * @param Handle Transfer handle
	 * @param ChannelName Channel name
	 * @param TotalChunks Total number of chunks
	 * @param TransferSize Total transfer size in bytes
	 * @param bIsCompressed Whether data is compressed
	 * @param SenderPlayerState Sender player state
	 * @param ReceiverPlayerState Receiver player state
	 */
	void HandleTransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, 
		int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Initialize a new sender-side transfer state.
	 * @param Handle Transfer handle
	 * @param ChannelName Channel name
	 * @param Sender Sender player state
	 * @param Receiver Receiver player state
	 * @param Data Data to transfer
	 * @param Settings Transfer settings
	 * @return Reference to created transfer state
	 */
	FEasyDataTransferState& CreateSenderTransferState(int32 Handle, const FString& ChannelName,
		APlayerState* Sender, APlayerState* Receiver, const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings);

private:
	/**
	 * Handle existing transfer conflicts when starting a new transfer.
	 */
	void HandleExistingTransferConflict(int32 Handle, const FString& ChannelName, int32 TotalChunks, 
		int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Convert sender-side state to receiver-side or create separate receiver state.
	 */
	void HandleSenderToReceiverConversion(int32 Handle, FEasyDataTransferState* ExistingState, 
		const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, 
		APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Convert existing transfer state to receiver mode.
	 */
	void ConvertToReceiverState(FEasyDataTransferState* TransferState, int32 TransferSize, bool bIsCompressed);

	/**
	 * Create a separate receiver-side transfer state.
	 */
	void CreateSeparateReceiverState(int32 OriginalHandle, FEasyDataTransferState* ExistingState, 
		const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, 
		APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Create a new receiver-side transfer state.
	 */
	void CreateNewReceiverTransferState(int32 Handle, const FString& ChannelName, int32 TotalChunks, 
		int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Initialize receiver state with basic parameters.
	 */
	void InitializeReceiverState(FEasyDataTransferState& State, int32 Handle, const FString& ChannelName, 
		int32 TransferSize, bool bIsCompressed, int32 OriginalHandle);

	/**
	 * Apply default settings to a transfer state.
	 */
	void ApplyDefaultSettings(FEasyDataTransferState& State);

	/**
	 * Update transfer timing information.
	 */
	void UpdateTransferTiming(FEasyDataTransferState& State);

	/**
	 * Track a transfer for a specific player.
	 */
	void TrackTransferForPlayer(int32 Handle, APlayerState* PlayerState);

	UEasyDataTransferSubsystem& Subsystem;
};