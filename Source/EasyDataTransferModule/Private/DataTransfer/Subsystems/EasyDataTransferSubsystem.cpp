// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Subsystems/EasyDataTransferSubsystem.h"

#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"
#include "DataTransfer/Settings/EasyDataTransferSettings.h"
#include "DataTransfer/Utils/EasyDataTransferValidation.h"
#include "DataTransfer/Utils/EasyDataTransferChunkProcessor.h"
#include "DataTransfer/Utils/EasyDataTransferStateManager.h"
#include "DataTransfer/Utils/EasyDataTransferBandwidthManager.h"
#include "DataTransfer/Utils/EasyDataTransferCompressionManager.h"
#include "DataTransfer/Utils/EasyDataTransferValidationManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "GameFramework/GameStateBase.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"

DEFINE_LOG_CATEGORY(LogEasyDataTransfer);

void UEasyDataTransferSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Initializing subsystem"), __FUNCTION__);
	
	// Initialize helper classes
	ChunkProcessor = MakeShared<FEasyDataTransferChunkProcessor>(*this);
	StateManager = MakeShared<FEasyDataTransferStateManager>(*this);
	BandwidthManager = MakeShared<FEasyDataTransferBandwidthManager>();
	CompressionManager = MakeShared<FEasyDataTransferCompressionManager>(*this);
	ValidationManager = MakeShared<FEasyDataTransferValidationManager>(*this);
	
	// Initialize timing for thread safety
	LastPeriodicUpdateTime = 0.0f;
	
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnPostWorldInit);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanup);
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Initialized"), __FUNCTION__);
}

void UEasyDataTransferSubsystem::Deinitialize()
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Deinitializing subsystem - ActiveTransfers count: %d"), __FUNCTION__, ActiveTransfers.Num());
	
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Deinitialized"), __FUNCTION__);
	
	Super::Deinitialize();
}

void UEasyDataTransferSubsystem::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	// Set up world context if available
	if (World)
	{
		CurrentWorld = World;
		
		// Start periodic update timer with delegate for proper cleanup
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UEasyDataTransferSubsystem::PeriodicUpdate);
		World->GetTimerManager().SetTimer(UpdateTimerHandle, TimerDelegate, 0.1f, true);
	}
}

void UEasyDataTransferSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (bCleanupResources && CurrentWorld == World)
	{
		// Clean up all active transfers
		for (auto& Transfer : ActiveTransfers)
		{
			UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Completing transfer %u during world cleanup"), __FUNCTION__, Transfer.Key);
			CompleteTransfer(Transfer.Key, false, TEXT("World cleanup"));
		}
	
		ActiveTransfers.Empty();
		PlayerTransfers.Empty();
	
		// Clean up helper classes
		if (CompressionManager)
		{
			CompressionManager->CancelAllCompressionTasks();
		}
		if (BandwidthManager)
		{
			BandwidthManager->Reset();
		}
		
		// Clear timer with proper validation
		if (CurrentWorld.IsValid())
		{
			if (UpdateTimerHandle.IsValid())
			{
				CurrentWorld->GetTimerManager().ClearTimer(UpdateTimerHandle);
			}
			if (ReceiverCleanupTimerHandle.IsValid())
			{
				CurrentWorld->GetTimerManager().ClearTimer(ReceiverCleanupTimerHandle);
			}
		}
		UpdateTimerHandle.Invalidate();
		ReceiverCleanupTimerHandle.Invalidate();
	
		CurrentWorld.Reset();
	}
}

void UEasyDataTransferSubsystem::PeriodicUpdate()
{
	// Calculate delta time using instance variable for thread safety
	const float CurrentTime = CurrentWorld.IsValid() ? CurrentWorld->GetTimeSeconds() : 0.0f;
	const float DeltaTime = LastPeriodicUpdateTime > 0.0f ? CurrentTime - LastPeriodicUpdateTime : 0.1f;
	LastPeriodicUpdateTime = CurrentTime;
	
	UpdateTransfers(DeltaTime);
}


int32 UEasyDataTransferSubsystem::OpenDataChannel(const FString& ChannelName, APlayerState* TargetPlayer, 
                                                   const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings)
{
	if (!TargetPlayer)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: OpenDataChannel failed - invalid target player"), __FUNCTION__);
		return 0;
	}
	
	// Get sender from current world context
	APlayerState* Sender = nullptr;
	if (CurrentWorld.IsValid())
	{
		// For now, assume we're the first player controller
		// In a real implementation, this would be determined by context
		if (const APlayerController* PC = CurrentWorld->GetFirstPlayerController())
		{
			Sender = PC->GetPlayerState<APlayerState>();
		}
	}
	
	if (!Sender)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: OpenDataChannel failed - no sender found"), __FUNCTION__);
		return 0;
	}
	
	return StartTransfer(ChannelName, Sender, TargetPlayer, Data, Settings);
}

int32 UEasyDataTransferSubsystem::OpenDataChannel(const FString& ChannelName, APlayerState* Sender, APlayerState* TargetPlayer, 
                                                   const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings)
{
	// Validate parameters
	if (!Sender)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: OpenDataChannel failed - invalid sender"), __FUNCTION__);
		return 0;
	}
	
	if (!TargetPlayer)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: OpenDataChannel failed - invalid target player"), __FUNCTION__);
		return 0;
	}
	
	// Use the explicit sender and target player
	return StartTransfer(ChannelName, Sender, TargetPlayer, Data, Settings);
}


TArray<uint8> UEasyDataTransferSubsystem::GetRawTransferData(int32 Handle) const
{
	if (const FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle))
	{
		if (TransferState->Status == EDataTransferStatus::Completed)
		{
			// For receiver-side states, return the reassembled data
			if (TransferState->bIsReceiver)
			{
				return TransferState->ReassembledData;
			}
			else
			{
				return TransferState->OriginalData;
			}
		}
		else
		{
			UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Transfer %d not completed (Status: %d)"), __FUNCTION__, Handle, (int32)TransferState->Status);
		}
	}
	else
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Transfer handle %d not found"), __FUNCTION__, Handle);
	}
	
	return TArray<uint8>();
}

void UEasyDataTransferSubsystem::CloseDataChannel(int32 Handle, const FString& Reason)
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Closing data channel %u - Reason: %s"), __FUNCTION__, Handle, *Reason);
	
	if (FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle))
	{
		// Cancel any in-progress compression using CompressionManager
		if (CompressionManager)
		{
			CompressionManager->CancelCompression(Handle, *TransferState);
		}
		
		// Check if transfer is already completed or failed - avoid double completion
		const bool bAlreadyCompleted = (TransferState->Status == EDataTransferStatus::Completed || TransferState->Status == EDataTransferStatus::Failed);
		
		if (bAlreadyCompleted)
		{
			UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Transfer %u already completed/failed (Status: %d) - skipping cancellation notification"), 
			       __FUNCTION__, Handle, static_cast<int32>(TransferState->Status));
		}
		
		// Only notify participants and send cancellation if transfer is still in progress
		if (!bAlreadyCompleted && !TransferState->bIsReceiver && TransferState->Sender.IsValid())
		{
			if (UEasyDataTransferPlayerComponent* SenderComponent = GetPlayerComponent(TransferState->Sender.Get()))
			{
				SenderComponent->SendTransferCancelled(Handle, Reason);
			}
		}
		
		// Fire callback
		OnDataTransferClosed.Broadcast(Handle, TransferState->ChannelName, Reason);
		
		// Remove from tracking
		if (TransferState->Sender.IsValid())
		{
			if (FEasyDataTransferPlayerState* PlayerHandles = PlayerTransfers.Find(TransferState->Sender))
			{
				PlayerHandles->Transfers.Remove(Handle);
			}
		}
		
		if (TransferState->Receiver.IsValid())
		{
			if (FEasyDataTransferPlayerState* PlayerHandles = PlayerTransfers.Find(TransferState->Receiver))
			{
				PlayerHandles->Transfers.Remove(Handle);
			}
		}
		
		// Remove transfer state
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Removing transfer state %u from ActiveTransfers"), __FUNCTION__, Handle);
		ActiveTransfers.Remove(Handle);
	}
	else
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Attempted to close unknown transfer %u"), __FUNCTION__, Handle);
	}
}

float UEasyDataTransferSubsystem::GetTransferProgress(int32 Handle) const
{
	if (const FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle))
	{
		return TransferState->GetProgress();
	}
	
	return -1.0f; // Invalid handle
}

EDataTransferStatus UEasyDataTransferSubsystem::GetTransferStatus(int32 Handle) const
{
	if (const FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle))
	{
		return TransferState->Status;
	}
	
	return EDataTransferStatus::Invalid;
}

TArray<int32> UEasyDataTransferSubsystem::GetActiveTransfersForPlayer(APlayerState* PlayerState) const
{
	if (const FEasyDataTransferPlayerState* PlayerHandles = PlayerTransfers.Find(PlayerState))
	{
		return PlayerHandles->Transfers;
	}
	
	return TArray<int32>();
}

void UEasyDataTransferSubsystem::CloseAllTransfersForPlayer(APlayerState* PlayerState, const FString& Reason)
{
	if (const FEasyDataTransferPlayerState* PlayerHandles = PlayerTransfers.Find(PlayerState))
	{
		// Copy handles since CloseDataChannel will modify the array
		TArray<int32> HandlesCopy = PlayerHandles->Transfers;
		
		for (int32 Handle : HandlesCopy)
		{
			CloseDataChannel(Handle, Reason);
		}
	}
	
	// Clean up player tracking
	PlayerTransfers.Remove(PlayerState);
	if (BandwidthManager)
	{
		BandwidthManager->RemovePlayer(PlayerState);
	}
}

int32 UEasyDataTransferSubsystem::StartTransfer(const FString& ChannelName, APlayerState* Sender, APlayerState* Receiver, 
                                                const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings)
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Starting transfer for channel '%s' - Sender: %s, Receiver: %s, Data size: %d bytes"), 
	       __FUNCTION__, *ChannelName, Sender ? *Sender->GetName() : TEXT("null"), Receiver ? *Receiver->GetName() : TEXT("null"), Data.Num());
	
	// Validate transfer using ValidationManager
	FString ValidationError;
	if (ValidationManager && !ValidationManager->ValidateNewTransfer(ChannelName, Settings, Data, Sender, Receiver, ValidationError))
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Transfer validation failed - %s"), __FUNCTION__, *ValidationError);
		return 0;
	}
	
	// Generate handle
	const int32 Handle = FEasyDataTransferHandleGenerator::GenerateHandle();
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Generated handle %u for transfer"), __FUNCTION__, Handle);
	
	// Create transfer state using StateManager
	FEasyDataTransferState& TransferState = StateManager->CreateSenderTransferState(Handle, ChannelName, Sender, Receiver, Data, Settings);
	
	// Bind to player destruction early
	if (!Sender->OnDestroyed.IsAlreadyBound(this, &UEasyDataTransferSubsystem::OnPlayerStateDestroyed))
	{
		Sender->OnDestroyed.AddDynamic(this, &UEasyDataTransferSubsystem::OnPlayerStateDestroyed);
	}
	if (!Receiver->OnDestroyed.IsAlreadyBound(this, &UEasyDataTransferSubsystem::OnPlayerStateDestroyed))
	{
		Receiver->OnDestroyed.AddDynamic(this, &UEasyDataTransferSubsystem::OnPlayerStateDestroyed);
	}
	
	// Start async compression if enabled, otherwise process synchronously
	if (TransferState.Settings.bEnableCompression && CompressionManager)
	{
		// Start async compression using CompressionManager
		if (!CompressionManager->StartAsyncCompression(Handle, TransferState, CurrentWorld.Get()))
		{
			UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Failed to start async compression for handle %u"), __FUNCTION__, Handle);
			ActiveTransfers.Remove(Handle);
			return 0;
		}
		
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Started async compression for transfer %u (%s) - %d bytes"), 
		       __FUNCTION__, Handle, *ChannelName, Data.Num());
	}
	else
	{
		// No compression needed, process synchronously
		TransferState.ProcessedData = TransferState.OriginalData;
		TransferState.bIsCompressed = false;
		
		// Create chunks
		if (!CreateChunks(TransferState))
		{
			UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Failed to create chunks for handle %u"), __FUNCTION__, Handle);
			ActiveTransfers.Remove(Handle);
			return 0;
		}
		
		// Continue with transfer - this calls SendNextChunks internally
		SendNextChunks(Handle);
	}
	
	return Handle;
}

bool UEasyDataTransferSubsystem::ProcessTransferData(FEasyDataTransferState& TransferState)
{
	// This method is now simplified - compression is handled asynchronously
	// It's kept for backward compatibility but just delegates to chunking
	return CreateChunks(TransferState);
}


bool UEasyDataTransferSubsystem::CreateChunks(FEasyDataTransferState& TransferState)
{
	const int32 ChunkSize = TransferState.Settings.ChunkSize;
	const int32 TotalDataSize = TransferState.ProcessedData.Num();
	
	if (TotalDataSize == 0)
	{
		// Empty data - create single empty chunk
		FEasyDataChunk& Chunk = TransferState.Chunks.AddDefaulted_GetRef();
		Chunk.TransferHandle = TransferState.Handle;
		Chunk.ChunkIndex = 0;
		Chunk.TotalChunks = 1;
		Chunk.Data.Empty();
		Chunk.Checksum = FEasyDataTransferCompression::CalculateChecksum(Chunk.Data);
		return true;
	}
	
	// Prevent integer overflow in chunk calculations
	if (ChunkSize <= 0)
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Invalid chunk size %d"), __FUNCTION__, ChunkSize);
		return false;
	}
	
	// Use 64-bit arithmetic to prevent overflow, then validate result
	const int64 TotalChunks64 = (static_cast<int64>(TotalDataSize) + ChunkSize - 1) / ChunkSize;
	if (TotalChunks64 > INT32_MAX || TotalChunks64 > 10000) // Also enforce our DoS limit
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Too many chunks required - %lld (max %d)"), __FUNCTION__, TotalChunks64, 10000);
		return false;
	}
	
	const int32 TotalChunks = static_cast<int32>(TotalChunks64);
	TransferState.Chunks.Reserve(TotalChunks);
	
	for (int32 ChunkIndex = 0; ChunkIndex < TotalChunks; ++ChunkIndex)
	{
		const int32 StartOffset = ChunkIndex * ChunkSize;
		const int32 EndOffset = FMath::Min(StartOffset + ChunkSize, TotalDataSize);
		const int32 CurrentChunkSize = EndOffset - StartOffset;
		
		FEasyDataChunk& Chunk = TransferState.Chunks.AddDefaulted_GetRef();
		Chunk.TransferHandle = TransferState.Handle;
		Chunk.ChunkIndex = ChunkIndex;
		Chunk.TotalChunks = TotalChunks;
		Chunk.Data.SetNum(CurrentChunkSize);
		
		// Copy data
		FMemory::Memcpy(Chunk.Data.GetData(), TransferState.ProcessedData.GetData() + StartOffset, CurrentChunkSize);
		
		// Calculate checksum
		Chunk.Checksum = FEasyDataTransferCompression::CalculateChecksum(Chunk.Data);
	}
	
	return true;
}

void UEasyDataTransferSubsystem::SendNextChunks(int32 Handle)
{
	// Use ValidationManager to validate transfer for sending
	FEasyDataTransferState* TransferState = nullptr;
	if (ValidationManager && !ValidationManager->ValidateTransferForSending(Handle, TransferState))
	{
		return;
	}
	
	// Use ValidationManager to validate receiver component
	UEasyDataTransferPlayerComponent* ReceiverComponent = nullptr;
	FString ValidationError;
	if (ValidationManager && !ValidationManager->ValidateReceiverComponent(Handle, TransferState, ReceiverComponent, ValidationError))
	{
		CompleteTransfer(Handle, false, ValidationError);
		return;
	}
	
	const float CurrentTime = CurrentWorld.IsValid() ? CurrentWorld->GetTimeSeconds() : 0.0f;
	if (BandwidthManager && !BandwidthManager->CanSendDataForPlayer(TransferState->Sender.Get(), 0, CurrentTime, GetSettings()))
	{
		return; // Skip sending this frame due to bandwidth limits
	}
	
	const int32 MaxChunksPerBatch = 3;
	int32 ChunksSent = 0;
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Sending chunks for transfer %u - NextChunkToSend: %d, TotalChunks: %d, Acknowledged: %d"), 
	       __FUNCTION__, Handle, TransferState->NextChunkToSend, TransferState->Chunks.Num(), TransferState->AcknowledgedChunks.Num());
	
	// Send new chunks first
	ChunksSent += SendNewChunks(Handle, TransferState, ReceiverComponent, CurrentTime, MaxChunksPerBatch);
	
	// Then retry unacknowledged chunks if we have remaining quota
	if (ChunksSent < MaxChunksPerBatch)
	{
		ChunksSent += RetryUnacknowledgedChunks(Handle, TransferState, ReceiverComponent, CurrentTime, MaxChunksPerBatch - ChunksSent);
	}
	
	if (ChunksSent > 0)
	{
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Sent %d chunks (including retries) for transfer %u"), __FUNCTION__, ChunksSent, Handle);
	}
	
	// Check if transfer is complete
	CheckTransferCompletion(Handle, TransferState);
}


int32 UEasyDataTransferSubsystem::SendNewChunks(int32 Handle, FEasyDataTransferState* TransferState, UEasyDataTransferPlayerComponent* ReceiverComponent, float CurrentTime, int32 MaxChunks)
{
	int32 ChunksSent = 0;
	
	while (TransferState->CanSendNextChunk() && ChunksSent < MaxChunks)
	{
		const int32 ChunkIndex = TransferState->NextChunkToSend;
		if (ChunkIndex >= TransferState->Chunks.Num())
		{
			UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Chunk index %d out of bounds for transfer %u (total chunks: %d)"), 
			       __FUNCTION__, ChunkIndex, Handle, TransferState->Chunks.Num());
			break;
		}
		
		const FEasyDataChunk& Chunk = TransferState->Chunks[ChunkIndex];
		
		if (BandwidthManager && !BandwidthManager->CanSendDataForPlayer(TransferState->Sender.Get(), Chunk.Data.Num(), CurrentTime, GetSettings()))
		{
			break; // Bandwidth limit reached
		}
		
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Sending chunk %d/%d for transfer %u (size: %d bytes)"), 
		       __FUNCTION__, ChunkIndex + 1, TransferState->Chunks.Num(), Handle, Chunk.Data.Num());
		
		ReceiverComponent->SendDataChunk(Chunk);
		
		TransferState->NextChunkToSend++;
		TransferState->BytesSent += Chunk.Data.Num();
		ChunksSent++;
		
		if (CurrentWorld.IsValid())
		{
			TransferState->UpdateActivity(CurrentWorld->GetTimeSeconds());
		}
	}
	
	return ChunksSent;
}

int32 UEasyDataTransferSubsystem::RetryUnacknowledgedChunks(int32 Handle, FEasyDataTransferState* TransferState, UEasyDataTransferPlayerComponent* ReceiverComponent, float CurrentTime, int32 MaxChunks)
{
	if (TransferState->NextChunkToSend < TransferState->Chunks.Num())
	{
		return 0; // Still have new chunks to send
	}
	
	int32 ChunksSent = 0;
	
	for (int32 ChunkIndex = 0; ChunkIndex < TransferState->Chunks.Num() && ChunksSent < MaxChunks; ++ChunkIndex)
	{
		if (!TransferState->AcknowledgedChunks.Contains(ChunkIndex))
		{
			const FEasyDataChunk& Chunk = TransferState->Chunks[ChunkIndex];
			
			if (BandwidthManager && !BandwidthManager->CanSendDataForPlayer(TransferState->Sender.Get(), Chunk.Data.Num(), CurrentTime, GetSettings()))
			{
				break; // Bandwidth limit reached for retries
			}
			
			UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Retrying unacknowledged chunk %d/%d for transfer %u (size: %d bytes)"), 
			       __FUNCTION__, ChunkIndex + 1, TransferState->Chunks.Num(), Handle, Chunk.Data.Num());
			
			ReceiverComponent->SendDataChunk(Chunk);
			TransferState->BytesSent += Chunk.Data.Num();
			ChunksSent++;
			
			if (CurrentWorld.IsValid())
			{
				TransferState->UpdateActivity(CurrentWorld->GetTimeSeconds());
			}
		}
	}
	
	return ChunksSent;
}

void UEasyDataTransferSubsystem::CheckTransferCompletion(int32 Handle, FEasyDataTransferState* TransferState)
{
	if (TransferState->AcknowledgedChunks.Num() == TransferState->Chunks.Num())
	{
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: All chunks acknowledged for transfer %u, completing"), __FUNCTION__, Handle);
		CompleteTransfer(Handle, true);
	}
}

void UEasyDataTransferSubsystem::CompleteTransfer(int32 Handle, bool bSuccess, const FString& ErrorMessage)
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Completing transfer %u - Success: %s, Error: %s"), __FUNCTION__, Handle, bSuccess ? TEXT("Yes") : TEXT("No"), *ErrorMessage);
	
	FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle);
	if (!TransferState)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Transfer %u not found for completion"), __FUNCTION__, Handle);
		return;
	}
	
	// Cancel any in-progress compression using CompressionManager
	if (CompressionManager)
	{
		CompressionManager->CancelCompression(Handle, *TransferState);
	}
	
	// Update status
	TransferState->Status = bSuccess ? EDataTransferStatus::Completed : EDataTransferStatus::Failed;
	if (!bSuccess && !ErrorMessage.IsEmpty())
	{
		TransferState->LastError = FEasyDataTransferValidationManager::MapErrorMessageToError(ErrorMessage);
	}
	
	// Notify participants (only for sender-side transfers)
	if (!TransferState->bIsReceiver)
	{
		if (UEasyDataTransferPlayerComponent* SenderComponent = GetPlayerComponent(TransferState->Sender.Get()))
		{
			const bool bToServer = TransferState->Sender->GetLocalRole() == ROLE_Authority;
			SenderComponent->SendTransferComplete(Handle, bSuccess, ErrorMessage, bToServer);
		}
		
		if (UEasyDataTransferPlayerComponent* ReceiverComponent = GetPlayerComponent(TransferState->Receiver.Get()))
		{
			const bool bToServer = TransferState->Receiver->GetLocalRole() == ROLE_Authority;
			ReceiverComponent->SendTransferComplete(Handle, bSuccess, ErrorMessage, bToServer);
		}
		
		// Fire callbacks for sender-side transfers
		if (bSuccess)
		{
			OnDataSent.Broadcast(Handle, TransferState->ChannelName);
		}
		else
		{
			OnDataError.Broadcast(Handle, TransferState->ChannelName, TransferState->LastError);
		}
	}
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: %s transfer %u (%s) - %s"), 
	       __FUNCTION__, bSuccess ? TEXT("Completed") : TEXT("Failed"), Handle, *TransferState->ChannelName,
	       TransferState->bIsReceiver ? TEXT("Receiver") : TEXT("Sender"));
	
	// Clean up after a delay to allow final RPCs to be sent (only for sender-side transfers)
	if (!TransferState->bIsReceiver && CurrentWorld.IsValid())
	{
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Scheduling cleanup for sender transfer %u"), __FUNCTION__, Handle);
		FTimerHandle CleanupTimerHandle;
		FTimerDelegate CleanupDelegate = FTimerDelegate::CreateLambda([this, Handle]()
		{
			CloseDataChannel(Handle, TEXT("Transfer completed"));
		});
		CurrentWorld->GetTimerManager().SetTimer(CleanupTimerHandle, CleanupDelegate, 1.0f, false);
	}
}


UEasyDataTransferPlayerComponent* UEasyDataTransferSubsystem::GetPlayerComponent(APlayerState* PlayerState) const
{
	if (PlayerState && PlayerState->Implements<UEasyDataTransferPlayerInterface>())
	{
		return IEasyDataTransferPlayerInterface::Execute_GetDataTransferComponent(PlayerState);
	}
	
	return nullptr;
}

const UEasyDataTransferSettings* UEasyDataTransferSubsystem::GetSettings()
{
	// GetDefault is never null and caching is unnecessary
	return GetDefault<UEasyDataTransferSettings>();
}

void UEasyDataTransferSubsystem::UpdateTransfers(float DeltaTime)
{
	if (!CurrentWorld.IsValid())
	{
		return;
	}
	
	const float CurrentTime = CurrentWorld->GetTimeSeconds();
	
	// Update bandwidth tracking
	if (BandwidthManager)
	{
		BandwidthManager->UpdateBandwidthTracking(CurrentTime);
	}
	
	// Check for timeouts and continue transfers
	TArray<int32> TimedOutTransfers;
	TArray<int32> CompletedReceiverTransfers;
	
	for (auto& TransferPair : ActiveTransfers)
	{
		FEasyDataTransferState& TransferState = TransferPair.Value;
		
		if (TransferState.Status == EDataTransferStatus::InProgress)
		{
			// Check timeout
			if (TransferState.HasTimedOut(CurrentTime))
			{
				TimedOutTransfers.Add(TransferState.Handle);
				continue;
			}
			
			// Handle sender-side transfers (continue sending chunks)
			if (!TransferState.bIsReceiver)
			{
				SendNextChunks(TransferState.Handle);
			}
			// Handle receiver-side transfers (check for completion)
			else if (TransferState.AreAllChunksReceived())
			{
				// Mark as completed to prevent race conditions
				TransferState.Status = EDataTransferStatus::Completed;
				CompletedReceiverTransfers.Add(TransferState.Handle);
				continue;
			}
			
			// Update progress
			const float Progress = TransferState.GetProgress();
			OnDataProgress.Broadcast(TransferState.Handle, TransferState.ChannelName, Progress);
		}
	}
	
	// Handle timeouts
	for (int32 Handle : TimedOutTransfers)
	{
		CompleteTransfer(Handle, false, TEXT("Transfer timed out"));
	}
	
	// Handle completed receiver transfers
	for (int32 Handle : CompletedReceiverTransfers)
	{
		FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle);
		if (!TransferState)
		{
			UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Transfer state not found for completed receiver transfer %u"), __FUNCTION__, Handle);
			continue;
		}
		
		if (TransferState->ReassembleAndDecompressData())
		{
			// Store the final data
			TransferState->OriginalData = TransferState->ReassembledData;
			
			UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Receiver transfer %u completed - reassembled %d bytes"), 
			       __FUNCTION__, Handle, TransferState->OriginalData.Num());
			
			// Fire completion callback
			OnDataReceived.Broadcast(Handle, TransferState->ChannelName, TransferState->OriginalData);
			
			// Clean up after a delay
			if (CurrentWorld.IsValid())
			{
				// NOTE: Using single timer handle for all receiver transfers
				// In a production system, consider using a map of timer handles per transfer
				// or a more sophisticated cleanup system to handle multiple concurrent receiver transfers
				FTimerDelegate CleanupDelegate = FTimerDelegate::CreateLambda([this, Handle]()
				{
					CloseDataChannel(Handle, TEXT("Receiver transfer completed"));
				});
				CurrentWorld->GetTimerManager().SetTimer(ReceiverCleanupTimerHandle, CleanupDelegate, 1.0f, false);
			}
		}
		else
		{
			TransferState->Status = EDataTransferStatus::Failed;
			TransferState->LastError = EDataTransferError::CompressionError;
			
			UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Receiver transfer %u failed - reassembly/decompression failed"), 
			       __FUNCTION__, Handle);
			
			// Fire error callback
			OnDataError.Broadcast(Handle, TransferState->ChannelName, EDataTransferError::CompressionError);
			
			// Clean up immediately
			CloseDataChannel(Handle, TEXT("Receiver transfer failed"));
		}
	}
}

void UEasyDataTransferSubsystem::OnPlayerStateDestroyed(AActor* DestroyedActor)
{
	if (APlayerState* PlayerState = Cast<APlayerState>(DestroyedActor))
	{
		CloseAllTransfersForPlayer(PlayerState, TEXT("Player disconnected"));
	}
}

void UEasyDataTransferSubsystem::HandleReceivedChunk(const FEasyDataChunk& Chunk)
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Delegating chunk processing to ChunkProcessor - Transfer: %u, Chunk: %d/%d"), 
	       __FUNCTION__, Chunk.TransferHandle, Chunk.ChunkIndex + 1, Chunk.TotalChunks);
	
	// Delegate to ChunkProcessor for all chunk handling logic
	if (ChunkProcessor)
	{
		ChunkProcessor->ProcessReceivedChunk(Chunk);
	}
	else
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: ChunkProcessor not initialized"), __FUNCTION__);
	}
}

void UEasyDataTransferSubsystem::HandleTransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Delegating transfer start to StateManager - Handle: %u, Channel: %s"), 
	       __FUNCTION__, Handle, *ChannelName);
	
	// Delegate to StateManager for all transfer state management
	if (StateManager)
	{
		StateManager->HandleTransferStarted(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState, ReceiverPlayerState);
	}
	else
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: StateManager not initialized"), __FUNCTION__);
	}
}



void UEasyDataTransferSubsystem::HandleSenderToReceiverConversion(int32 Handle, FEasyDataTransferState* ExistingState, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	if (ReceiverPlayerState && ExistingState->Receiver == ReceiverPlayerState)
	{
		// Convert existing sender-side state to receiver-side
		ConvertToReceiverState(ExistingState, TransferSize, bIsCompressed);
		UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Converted transfer %u to receiver-side state"), __FUNCTION__, Handle);
	}
	else
	{
		// Create separate receiver-side state with different handle
		CreateSeparateReceiverState(Handle, ExistingState, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState, ReceiverPlayerState);
	}
}

void UEasyDataTransferSubsystem::ConvertToReceiverState(FEasyDataTransferState* TransferState, int32 TransferSize, bool bIsCompressed)
{
	TransferState->bIsReceiver = true;
	TransferState->ExpectedOriginalSize = TransferSize;
	TransferState->bExpectedCompressed = bIsCompressed;
	
	ApplyDefaultSettings(*TransferState);
	UpdateTransferTiming(*TransferState);
}

void UEasyDataTransferSubsystem::CreateSeparateReceiverState(int32 OriginalHandle, FEasyDataTransferState* ExistingState, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	const int32 ReceiverHandle = OriginalHandle + 1000000;
	
	if (ActiveTransfers.Contains(ReceiverHandle))
	{
		UE_LOG(LogEasyDataTransfer, Error, TEXT("%hs: Generated receiver handle %u already exists!"), __FUNCTION__, ReceiverHandle);
		return;
	}
	
	FEasyDataTransferState& ReceiverState = ActiveTransfers.Add(ReceiverHandle);
	InitializeReceiverState(ReceiverState, ReceiverHandle, ChannelName, TransferSize, bIsCompressed, OriginalHandle);
	
	ReceiverState.Sender = ExistingState->Sender;
	ReceiverState.Receiver = ExistingState->Receiver;
	
	ApplyDefaultSettings(ReceiverState);
	UpdateTransferTiming(ReceiverState);
	TrackTransferForPlayer(ReceiverHandle, ReceiverPlayerState);
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Created separate receiver-side transfer state %u (linked to sender %u)"), 
	       __FUNCTION__, ReceiverHandle, OriginalHandle);
}

void UEasyDataTransferSubsystem::CreateNewReceiverTransferState(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	FEasyDataTransferState& ReceiverState = ActiveTransfers.Add(Handle);
	InitializeReceiverState(ReceiverState, Handle, ChannelName, TransferSize, bIsCompressed, 0);
	
	ReceiverState.Sender = SenderPlayerState;
	ReceiverState.Receiver = ReceiverPlayerState;
	
	ApplyDefaultSettings(ReceiverState);
	UpdateTransferTiming(ReceiverState);
	TrackTransferForPlayer(Handle, ReceiverPlayerState);
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Created receiver-side transfer state for handle %u"), __FUNCTION__, Handle);
}

void UEasyDataTransferSubsystem::InitializeReceiverState(FEasyDataTransferState& State, int32 Handle, const FString& ChannelName, int32 TransferSize, bool bIsCompressed, int32 OriginalHandle)
{
	State.Handle = Handle;
	State.ChannelName = ChannelName;
	State.Status = EDataTransferStatus::InProgress;
	State.bIsReceiver = true;
	State.ExpectedOriginalSize = TransferSize;
	State.bExpectedCompressed = bIsCompressed;
	if (OriginalHandle != 0)
	{
		State.OriginalSenderHandle = OriginalHandle;
	}
}

void UEasyDataTransferSubsystem::ApplyDefaultSettings(FEasyDataTransferState& State)
{
	const UEasyDataTransferSettings* GlobalSettings = GetSettings();
	if (GlobalSettings)
	{
		State.Settings.ApplyDefaults(GlobalSettings);
	}
}

void UEasyDataTransferSubsystem::UpdateTransferTiming(FEasyDataTransferState& State)
{
	const float CurrentTime = CurrentWorld.IsValid() ? CurrentWorld->GetTimeSeconds() : 0.0f;
	State.StartTime = CurrentTime;
	State.UpdateActivity(CurrentTime);
}

void UEasyDataTransferSubsystem::TrackTransferForPlayer(int32 Handle, APlayerState* PlayerState)
{
	if (PlayerState)
	{
		PlayerTransfers.FindOrAdd(PlayerState).Transfers.Add(Handle);
	}
}

void UEasyDataTransferSubsystem::HandleChunkAcknowledged(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState)
{
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Received acknowledgment for chunk %d of transfer %u from sender %s"), 
	       __FUNCTION__, ChunkIndex + 1, Handle, SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"));
	       
	FEasyDataTransferState* TransferState = ActiveTransfers.Find(Handle);
	if (!TransferState)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Transfer state not found for handle %u"), __FUNCTION__, Handle);
		return;
	}
	
	// Validate that the acknowledgment is coming from the correct sender (if we have sender info)
	if (SenderPlayerState && TransferState->Sender.IsValid() && TransferState->Sender.Get() != SenderPlayerState)
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: Acknowledgment for transfer %u from wrong sender (expected %s, got %s)"), 
		       __FUNCTION__, Handle, *TransferState->Sender->GetName(), *SenderPlayerState->GetName());
		return;
	}
	
	// Mark chunk as acknowledged
	TransferState->AcknowledgedChunks.Add(ChunkIndex);
	
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Chunk %d acknowledged for transfer %u - Total acknowledged: %d"), 
	       __FUNCTION__, ChunkIndex + 1, Handle, TransferState->AcknowledgedChunks.Num());
	
	// Update activity
	if (CurrentWorld.IsValid())
	{
		TransferState->UpdateActivity(CurrentWorld->GetTimeSeconds());
	}
	
	// Continue sending if needed
	SendNextChunks(Handle);
	
	// Log with appropriate information for sender vs receiver
	if (TransferState->bIsReceiver)
	{
		UE_LOG(LogEasyDataTransfer, Verbose, TEXT("%hs: Chunk %d acknowledged for receiver transfer %u (%d acknowledged)"), 
		       __FUNCTION__, ChunkIndex, Handle, TransferState->AcknowledgedChunks.Num());
	}
	else
	{
		UE_LOG(LogEasyDataTransfer, Verbose, TEXT("%hs: Chunk %d acknowledged for sender transfer %u (%d/%d)"), 
		       __FUNCTION__, ChunkIndex, Handle, TransferState->AcknowledgedChunks.Num(), TransferState->Chunks.Num());
	}
}

void UEasyDataTransferSubsystem::HandleTransferComplete(int32 Handle, bool bSuccess, const FString& ErrorMessage)
{
	// This is handled by CompleteTransfer
	UE_LOG(LogEasyDataTransfer, Log, TEXT("%hs: Transfer %u completed - Success: %s"), 
	       __FUNCTION__, Handle, bSuccess ? TEXT("Yes") : TEXT("No"));
}

void UEasyDataTransferSubsystem::HandleTransferCancelled(int32 Handle, const FString& Reason)
{
	// Don't call CloseDataChannel here as it would create an infinite loop!
	// CloseDataChannel sends a multicast RPC which calls this method again.
	// Instead, just complete the transfer without sending more RPCs.
	CompleteTransfer(Handle, false, Reason);
}


FString UEasyDataTransferSubsystem::GetActiveTransferHandlesString() const
{
	FString Result = TEXT("[");
	bool bFirst = true;
	for (const auto& TransferPair : ActiveTransfers)
	{
		if (!bFirst)
		{
			Result += TEXT(", ");
		}
		Result += FString::Printf(TEXT("%u"), TransferPair.Key);
		bFirst = false;
	}
	Result += TEXT("]");
	return Result;
}
