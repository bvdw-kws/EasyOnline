// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Utils/EasyDataTransferCompressionManager.h"
#include "DataTransfer/Subsystems/EasyDataTransferSubsystem.h"
#include "DataTransfer/Utils/EasyDataTransferChunkProcessor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"


FEasyDataTransferCompressionManager::FEasyDataTransferCompressionManager(UEasyDataTransferSubsystem& InSubsystem)
	: Subsystem(InSubsystem)
{
}

FEasyDataTransferCompressionManager::~FEasyDataTransferCompressionManager()
{
	CancelAllCompressionTasks();
}

bool FEasyDataTransferCompressionManager::StartAsyncCompression(int32 Handle, FEasyDataTransferState& TransferState, UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Invalid world context for handle %u"), __FUNCTION__, Handle);
		return false;
	}

	CurrentWorld = World;

	// Create async compression task
	auto CompressionTask = MakeShared<FAsyncTask<FEasyDataCompressionTask>>(
		TransferState.OriginalData,
		true, // Compress
		0     // No uncompressed size needed for compression
	);

	// Store task reference in both our map and the transfer state
	ActiveCompressionTasks.Add(Handle, CompressionTask);
	TransferState.CompressionTask = CompressionTask;
	TransferState.Status = EDataTransferStatus::Compressing;

	// Start the async task
	CompressionTask->StartBackgroundTask();

	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Started async compression for transfer %u (%d bytes)"), 
	       __FUNCTION__, Handle, TransferState.OriginalData.Num());

	// Set up or update the timer to check for completion
	if (!CompressionCheckTimerHandle.IsValid())
	{
		World->GetTimerManager().SetTimer(
			CompressionCheckTimerHandle,
			FTimerDelegate::CreateRaw(this, &FEasyDataTransferCompressionManager::CheckCompressionCompletion),
			0.01f, // Check every 10ms
			true   // Loop
		);
	}

	return true;
}

void FEasyDataTransferCompressionManager::CheckCompressionCompletion()
{
	if (!CurrentWorld.IsValid())
	{
		CurrentWorld->GetTimerManager().ClearTimer(CompressionCheckTimerHandle);
		return;
	}

	// Check all active compression tasks
	TArray<int32> CompletedHandles;
	for (const auto& TaskPair : ActiveCompressionTasks)
	{
		if (TaskPair.Value.IsValid() && TaskPair.Value->IsDone())
		{
			CompletedHandles.Add(TaskPair.Key);
		}
	}

	// Process completed tasks
	for (int32 Handle : CompletedHandles)
	{
		// Get transfer state from subsystem
		if (FEasyDataTransferState* TransferState = Subsystem.ActiveTransfers.Find(Handle))
		{
			HandleCompressionComplete(Handle, *TransferState);
		}
		ActiveCompressionTasks.Remove(Handle);
	}

	// Clear timer if no more tasks
	if (ActiveCompressionTasks.Num() == 0 && CompressionCheckTimerHandle.IsValid())
	{
		CurrentWorld->GetTimerManager().ClearTimer(CompressionCheckTimerHandle);
	}
}

void FEasyDataTransferCompressionManager::HandleCompressionComplete(int32 Handle, FEasyDataTransferState& TransferState)
{
	if (!TransferState.CompressionTask.IsValid())
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Compression task lost for handle %u"), __FUNCTION__, Handle);
		Subsystem.CompleteTransfer(Handle, false, TEXT("Compression task lost"));
		return;
	}

	// Get compression results
	const FEasyDataCompressionTask& Task = TransferState.CompressionTask->GetTask();

	if (Task.WasSuccessful() && Task.WasBeneficial())
	{
		// Use compressed data
		TransferState.ProcessedData = Task.GetOutputData();
		TransferState.bIsCompressed = true;

		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Async compression completed for %u - %d bytes to %d bytes (%.1f%% reduction)"),
		       __FUNCTION__, Handle, TransferState.OriginalData.Num(), TransferState.ProcessedData.Num(),
		       100.0f * (1.0f - float(TransferState.ProcessedData.Num()) / float(TransferState.OriginalData.Num())));
	}
	else
	{
		// Compression failed or wasn't beneficial
		TransferState.ProcessedData = TransferState.OriginalData;
		TransferState.bIsCompressed = false;

		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Async compression not beneficial for %u, using original data"),
		       __FUNCTION__, Handle);
	}

	// Clear task reference
	TransferState.CompressionTask.Reset();

	// Create chunks
	if (!Subsystem.CreateChunks(TransferState))
	{
		Subsystem.CompleteTransfer(Handle, false, TEXT("Failed to create chunks"));
		return;
	}

	// Continue with transfer
	ContinueTransferAfterCompression(Handle, TransferState);
}

void FEasyDataTransferCompressionManager::ContinueTransferAfterCompression(int32 Handle, FEasyDataTransferState& TransferState)
{
	// Update status
	TransferState.Status = EDataTransferStatus::InProgress;

	// Notify receiver
	APlayerState* Receiver = TransferState.Receiver.Get();
	if (!Receiver)
	{
		Subsystem.CompleteTransfer(Handle, false, TEXT("Receiver disconnected"));
		return;
	}

	if (UEasyDataTransferPlayerComponent* ReceiverComponent = Subsystem.GetPlayerComponent(Receiver))
	{
		// Use the same logic as before
		UEasyDataTransferPlayerComponent* SenderComponent = Subsystem.GetPlayerComponent(TransferState.Sender.Get());
		const bool bToServer = SenderComponent && SenderComponent->GetOwnerRole() == ROLE_AutonomousProxy;

		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Sending TransferStarted RPC to receiver for handle %u"), 
		       __FUNCTION__, Handle);

		// Only send RPC if receiver is different from sender
		if (Receiver != TransferState.Sender.Get())
		{
			ReceiverComponent->SendTransferStarted(Handle, TransferState.ChannelName, 
			                                        TransferState.Chunks.Num(), TransferState.OriginalSize, 
			                                        TransferState.bIsCompressed, TransferState.Sender.Get(), bToServer);
		}
	}
	else
	{
		Subsystem.CompleteTransfer(Handle, false, TEXT("Receiver component not available"));
		return;
	}

	// Start sending chunks
	Subsystem.SendNextChunks(Handle);

	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Transfer %u (%s) ready - %d bytes in %d chunks"), 
	       __FUNCTION__, Handle, *TransferState.ChannelName, 
	       TransferState.ProcessedData.Num(), TransferState.Chunks.Num());
}

void FEasyDataTransferCompressionManager::CancelAllCompressionTasks()
{
	// Cancel all active tasks
	for (auto& TaskPair : ActiveCompressionTasks)
	{
		if (TaskPair.Value.IsValid())
		{
			if (!TaskPair.Value->IsDone())
			{
				TaskPair.Value->GetTask().Cancel();
				TaskPair.Value->Cancel();
			}
		}
	}
	ActiveCompressionTasks.Empty();

	// Clear timer
	if (CurrentWorld.IsValid() && CompressionCheckTimerHandle.IsValid())
	{
		CurrentWorld->GetTimerManager().ClearTimer(CompressionCheckTimerHandle);
	}
}

void FEasyDataTransferCompressionManager::CancelCompression(int32 Handle, FEasyDataTransferState& TransferState)
{
	// Cancel in transfer state
	if (TransferState.CompressionTask.IsValid())
	{
		if (!TransferState.CompressionTask->IsDone())
		{
			TransferState.CompressionTask->GetTask().Cancel();
			TransferState.CompressionTask->Cancel();
			UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Cancelled in-progress compression for transfer %u"), __FUNCTION__, Handle);
		}
		TransferState.CompressionTask.Reset();
	}

	// Remove from our tracking
	ActiveCompressionTasks.Remove(Handle);

	// Clear timer if no more tasks
	if (ActiveCompressionTasks.Num() == 0 && CurrentWorld.IsValid() && CompressionCheckTimerHandle.IsValid())
	{
		CurrentWorld->GetTimerManager().ClearTimer(CompressionCheckTimerHandle);
	}
}