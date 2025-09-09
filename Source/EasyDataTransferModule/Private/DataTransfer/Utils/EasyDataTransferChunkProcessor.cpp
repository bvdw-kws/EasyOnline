// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Utils/EasyDataTransferChunkProcessor.h"

#include "DataTransfer/Subsystems/EasyDataTransferSubsystem.h"
#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"
#include "DataTransfer/Utils/EasyDataTransferValidation.h"

DEFINE_LOG_CATEGORY_STATIC(LogEasyDataTransferChunkProcessor, Log, All);

FEasyDataTransferChunkProcessor::FEasyDataTransferChunkProcessor(UEasyDataTransferSubsystem& InSubsystem)
	: Subsystem(InSubsystem)
{
}

bool FEasyDataTransferChunkProcessor::ProcessReceivedChunk(const FEasyDataChunk& Chunk)
{
	// Validate chunk before processing
	if (!FEasyDataTransferValidation::ValidateChunk(Chunk, TEXT("ProcessReceivedChunk")))
	{
		UE_LOG(LogEasyDataTransferChunkProcessor, Warning, TEXT("%hs: Received invalid chunk for transfer %u"), __FUNCTION__, Chunk.TransferHandle);
		return false;
	}
	
	UE_LOG(LogEasyDataTransferChunkProcessor, Log, TEXT("%hs: Processing chunk %d/%d for transfer %u"), 
	       __FUNCTION__, Chunk.ChunkIndex + 1, Chunk.TotalChunks, Chunk.TransferHandle);
	
	// Find transfer state for this chunk
	FEasyDataTransferState* TransferState = FindTransferStateForChunk(Chunk);
	if (!TransferState)
	{
		UE_LOG(LogEasyDataTransferChunkProcessor, Warning, TEXT("%hs: No transfer state found for chunk transfer %u"), 
		       __FUNCTION__, Chunk.TransferHandle);
		return false;
	}
	
	// Update activity timing
	if (UWorld* World = Subsystem.GetWorld())
	{
		TransferState->UpdateActivity(World->GetTimeSeconds());
	}
	
	// Track received bytes
	TransferState->BytesReceived += Chunk.Data.Num();
	
	// Handle receiver-side chunk storage and completion
	if (TransferState->bIsReceiver)
	{
		if (!HandleReceiverChunkStorage(TransferState, Chunk))
		{
			return false;
		}
		
		// Send acknowledgment
		SendChunkAcknowledgment(TransferState, Chunk);
	}
	
	return true;
}

FEasyDataTransferState* FEasyDataTransferChunkProcessor::FindTransferStateForChunk(const FEasyDataChunk& Chunk)
{
	// Try direct lookup first
	if (FEasyDataTransferState* DirectState = Subsystem.ActiveTransfers.Find(Chunk.TransferHandle))
	{
		return DirectState;
	}
	
	UE_LOG(LogEasyDataTransferChunkProcessor, Verbose, TEXT("%hs: Transfer %u not found directly, checking for linked receiver states"), 
	       __FUNCTION__, Chunk.TransferHandle);
	
	// Check for sender handle redirected to receiver handle
	for (auto& TransferPair : Subsystem.ActiveTransfers)
	{
		if (TransferPair.Value.OriginalSenderHandle == Chunk.TransferHandle && TransferPair.Value.bIsReceiver)
		{
			UE_LOG(LogEasyDataTransferChunkProcessor, Log, TEXT("%hs: Redirecting chunk from sender handle %u to receiver handle %u"), 
			       __FUNCTION__, Chunk.TransferHandle, TransferPair.Key);
			return &TransferPair.Value;
		}
	}
	
	return nullptr;
}

bool FEasyDataTransferChunkProcessor::HandleReceiverChunkStorage(FEasyDataTransferState* TransferState, const FEasyDataChunk& Chunk)
{
	// Store the chunk
	TransferState->ReceivedChunks.Add(Chunk.ChunkIndex, Chunk);
	
	UE_LOG(LogEasyDataTransferChunkProcessor, Log, 
	       TEXT("%hs: Stored chunk %d/%d for receiver transfer %u (Total received: %d, Expected: %d)"), 
	       __FUNCTION__, Chunk.ChunkIndex + 1, Chunk.TotalChunks, TransferState->Handle, 
	       TransferState->ReceivedChunks.Num(), Chunk.TotalChunks);
	
	// Check if all chunks are received
	if (TransferState->AreAllChunksReceived())
	{
		UE_LOG(LogEasyDataTransferChunkProcessor, Log, TEXT("%hs: All chunks received for transfer %u, processing completion"), 
		       __FUNCTION__, TransferState->Handle);
		
		return HandleReceiverTransferCompletion(TransferState);
	}
	else
	{
		// Log progress for debugging
		TArray<int32> ReceivedIndices;
		TransferState->ReceivedChunks.GetKeys(ReceivedIndices);
		ReceivedIndices.Sort();
		
		FString ReceivedChunksStr;
		for (int32 i = 0; i < FMath::Min(ReceivedIndices.Num(), 10); i++) // Show first 10 for brevity
		{
			if (i > 0) ReceivedChunksStr += TEXT(", ");
			ReceivedChunksStr += FString::Printf(TEXT("%d"), ReceivedIndices[i] + 1);
		}
		if (ReceivedIndices.Num() > 10)
		{
			ReceivedChunksStr += TEXT("...");
		}
		
		UE_LOG(LogEasyDataTransferChunkProcessor, Verbose, 
		       TEXT("%hs: Transfer %u still waiting - Received chunks: [%s], Total expected: %d"), 
		       __FUNCTION__, TransferState->Handle, *ReceivedChunksStr, Chunk.TotalChunks);
	}
	
	return true;
}

bool FEasyDataTransferChunkProcessor::HandleReceiverTransferCompletion(FEasyDataTransferState* TransferState)
{
	// Attempt data reassembly and decompression
	if (TransferState->ReassembleAndDecompressData())
	{
		// Mark as completed
		TransferState->Status = EDataTransferStatus::Completed;
		
		// Send completion notification to sender
		if (TransferState->Sender.IsValid())
		{
			if (UEasyDataTransferPlayerComponent* SenderComponent = Subsystem.GetPlayerComponent(TransferState->Sender.Get()))
			{
				const bool bToServer = TransferState->Sender->GetLocalRole() == ROLE_Authority;
				SenderComponent->SendTransferComplete(TransferState->Handle, true, TEXT(""), bToServer);
			}
		}
		
		// Trigger completion callback
		Subsystem.OnDataReceived.Broadcast(TransferState->Handle, TransferState->ChannelName, TransferState->ReassembledData);
		
		UE_LOG(LogEasyDataTransferChunkProcessor, Log, TEXT("%hs: Transfer %u completed successfully - %d bytes reassembled"), 
		       __FUNCTION__, TransferState->Handle, TransferState->ReassembledData.Num());
		
		return true;
	}
	else
	{
		// Reassembly failed
		TransferState->Status = EDataTransferStatus::Failed;
		TransferState->LastError = EDataTransferError::ValidationError;
		
		// Send failure notification to sender
		if (TransferState->Sender.IsValid())
		{
			if (UEasyDataTransferPlayerComponent* SenderComponent = Subsystem.GetPlayerComponent(TransferState->Sender.Get()))
			{
				const bool bToServer = TransferState->Sender->GetLocalRole() == ROLE_Authority;
				SenderComponent->SendTransferComplete(TransferState->Handle, false, TEXT("Data reassembly failed"), bToServer);
			}
		}
		
		// Trigger error callback
		Subsystem.OnDataError.Broadcast(TransferState->Handle, TransferState->ChannelName, EDataTransferError::ValidationError);
		
		UE_LOG(LogEasyDataTransferChunkProcessor, Error, TEXT("%hs: Transfer %u failed - data reassembly failed"), 
		       __FUNCTION__, TransferState->Handle);
		
		return false;
	}
}

void FEasyDataTransferChunkProcessor::SendChunkAcknowledgment(FEasyDataTransferState* TransferState, const FEasyDataChunk& Chunk)
{
	if (!TransferState->bIsReceiver || !TransferState->Sender.IsValid())
	{
		return;
	}
	
	UE_LOG(LogEasyDataTransferChunkProcessor, Log, 
	       TEXT("%hs: Sending acknowledgment for chunk %d/%d of transfer %u"), 
	       __FUNCTION__, Chunk.ChunkIndex + 1, Chunk.TotalChunks, TransferState->Handle);
	
	// Always send acknowledgment to server in MassStep restore scenario
	const bool bToServer = true;
	
	// Send via the receiver's component
	UEasyDataTransferPlayerComponent* ReceiverComponent = Subsystem.GetPlayerComponent(TransferState->Receiver.Get());
	if (!ReceiverComponent)
	{
		UE_LOG(LogEasyDataTransferChunkProcessor, Warning, 
		       TEXT("%hs: Cannot send ACK - receiver component not found for %s"), 
		       __FUNCTION__, TransferState->Receiver.IsValid() ? *TransferState->Receiver->GetName() : TEXT("null"));
		return;
	}
	
	ReceiverComponent->SendChunkAcknowledgment(TransferState->Handle, Chunk.ChunkIndex, TransferState->Sender.Get(), bToServer);
}