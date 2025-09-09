// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "DataTransfer/EasyDataTransferTypes.h"
#include "Engine/TimerHandle.h"

class UEasyDataTransferSubsystem;
class UWorld;

/**
 * Manages async compression operations for data transfers.
 * Handles compression task lifecycle, completion callbacks, and error handling.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferCompressionManager
{
public:
	explicit FEasyDataTransferCompressionManager(UEasyDataTransferSubsystem& InSubsystem);
	~FEasyDataTransferCompressionManager();

	/**
	 * Start async compression for a transfer.
	 * @param Handle Transfer handle
	 * @param TransferState The transfer state to compress
	 * @param World World context for timer management
	 * @return True if compression was started successfully
	 */
	bool StartAsyncCompression(int32 Handle, FEasyDataTransferState& TransferState, UWorld* World);

	/**
	 * Cancel all active compression tasks.
	 * Called during subsystem shutdown.
	 */
	void CancelAllCompressionTasks();

	/**
	 * Cancel compression for a specific transfer.
	 * @param Handle Transfer handle
	 * @param TransferState The transfer state
	 */
	void CancelCompression(int32 Handle, FEasyDataTransferState& TransferState);

private:
	/**
	 * Check compression task completion.
	 * Called periodically by timer.
	 */
	void CheckCompressionCompletion();

	/**
	 * Handle compression task completion.
	 * @param Handle Transfer handle
	 * @param TransferState The transfer state
	 */
	void HandleCompressionComplete(int32 Handle, FEasyDataTransferState& TransferState);

	/**
	 * Continue transfer after compression completes.
	 * @param Handle Transfer handle
	 * @param TransferState The transfer state
	 */
	void ContinueTransferAfterCompression(int32 Handle, FEasyDataTransferState& TransferState);

private:
	// Reference to parent subsystem
	UEasyDataTransferSubsystem& Subsystem;

	// Map of active compression tasks by handle
	TMap<int32, TSharedPtr<FAsyncTask<FEasyDataCompressionTask>>> ActiveCompressionTasks;

	// Timer for checking compression completion
	FTimerHandle CompressionCheckTimerHandle;

	// World reference for timer management
	TWeakObjectPtr<UWorld> CurrentWorld;
};