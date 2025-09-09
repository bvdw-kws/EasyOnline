// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "DataTransfer/EasyDataTransferTypes.h"

class UEasyDataTransferSubsystem;
class UEasyDataTransferPlayerComponent;

/**
 * Handles chunk processing logic for data transfers.
 * Separates chunk-level operations from high-level transfer management.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferChunkProcessor
{
public:
	explicit FEasyDataTransferChunkProcessor(UEasyDataTransferSubsystem& InSubsystem);

	/**
	 * Process a received chunk and update transfer state.
	 * @param Chunk The received chunk
	 * @return True if chunk was processed successfully
	 */
	bool ProcessReceivedChunk(const FEasyDataChunk& Chunk);

private:
	/**
	 * Find the transfer state for a chunk, handling sender/receiver redirection.
	 * @param Chunk The chunk to find state for
	 * @return Transfer state if found, nullptr otherwise
	 */
	FEasyDataTransferState* FindTransferStateForChunk(const FEasyDataChunk& Chunk);

	/**
	 * Handle chunk storage for receiver-side transfers.
	 * @param TransferState The receiver transfer state
	 * @param Chunk The chunk to store
	 * @return True if chunk was stored and transfer should continue
	 */
	bool HandleReceiverChunkStorage(FEasyDataTransferState* TransferState, const FEasyDataChunk& Chunk);

	/**
	 * Process completed receiver transfer (reassembly and callbacks).
	 * @param TransferState The completed transfer state
	 * @return True if completion was handled successfully
	 */
	bool HandleReceiverTransferCompletion(FEasyDataTransferState* TransferState);

	/**
	 * Send acknowledgment for received chunk.
	 * @param TransferState The transfer state
	 * @param Chunk The chunk to acknowledge
	 */
	void SendChunkAcknowledgment(FEasyDataTransferState* TransferState, const FEasyDataChunk& Chunk);

	UEasyDataTransferSubsystem& Subsystem;
};