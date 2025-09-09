// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Utils/EasyDataTransferStateManager.h"

#include "DataTransfer/Subsystems/EasyDataTransferSubsystem.h"
#include "DataTransfer/Settings/EasyDataTransferSettings.h"
#include "DataTransfer/Utils/EasyDataTransferValidation.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogEasyDataTransferStateManager, Log, All);

FEasyDataTransferStateManager::FEasyDataTransferStateManager(UEasyDataTransferSubsystem& InSubsystem)
	: Subsystem(InSubsystem)
{
}

void FEasyDataTransferStateManager::HandleTransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, 
	int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	UE_LOG(LogEasyDataTransferStateManager, Log, 
	       TEXT("%hs: Transfer %u (%s) started - %d bytes in %d chunks (compressed: %s)"), 
	       __FUNCTION__, Handle, *ChannelName, TransferSize, TotalChunks, bIsCompressed ? TEXT("Yes") : TEXT("No"));
	
	// Validate parameters using centralized validation
	if (!FEasyDataTransferValidation::ValidateTransferParameters(Handle, TotalChunks, TransferSize, TEXT("HandleTransferStarted")))
	{
		return;
	}
	
	// Handle existing transfer conflicts or create new receiver state
	HandleExistingTransferConflict(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState, ReceiverPlayerState);
}

FEasyDataTransferState& FEasyDataTransferStateManager::CreateSenderTransferState(int32 Handle, const FString& ChannelName,
	APlayerState* Sender, APlayerState* Receiver, const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings)
{
	UE_LOG(LogEasyDataTransferStateManager, Log, 
	       TEXT("%hs: Creating sender transfer state for handle %u - Channel: %s, Data size: %d bytes"), 
	       __FUNCTION__, Handle, *ChannelName, Data.Num());
	
	// Create the transfer state
	FEasyDataTransferState& TransferState = Subsystem.ActiveTransfers.Add(Handle);
	
	// Initialize basic properties
	TransferState.Handle = Handle;
	TransferState.ChannelName = ChannelName;
	TransferState.Sender = Sender;
	TransferState.Receiver = Receiver;
	TransferState.OriginalData = Data;
	TransferState.OriginalSize = Data.Num();
	TransferState.Settings = Settings;
	TransferState.Status = EDataTransferStatus::Pending;
	TransferState.bIsReceiver = false;
	
	// Apply default settings
	ApplyDefaultSettings(TransferState);
	
	// Set timing information
	UpdateTransferTiming(TransferState);
	
	// Track transfer for both players
	TrackTransferForPlayer(Handle, Sender);
	TrackTransferForPlayer(Handle, Receiver);
	
	return TransferState;
}

void FEasyDataTransferStateManager::HandleExistingTransferConflict(int32 Handle, const FString& ChannelName, 
	int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	if (Subsystem.ActiveTransfers.Contains(Handle))
	{
		FEasyDataTransferState* ExistingState = Subsystem.ActiveTransfers.Find(Handle);
		if (ExistingState && !ExistingState->bIsReceiver)
		{
			// Handle sender-to-receiver conversion
			HandleSenderToReceiverConversion(Handle, ExistingState, ChannelName, TotalChunks, TransferSize, 
				bIsCompressed, SenderPlayerState, ReceiverPlayerState);
		}
		else if (ExistingState && ExistingState->bIsReceiver)
		{
			UE_LOG(LogEasyDataTransferStateManager, Warning, 
			       TEXT("%hs: Receiver-side transfer state already exists for handle %u"), __FUNCTION__, Handle);
		}
		else
		{
			UE_LOG(LogEasyDataTransferStateManager, Warning, 
			       TEXT("%hs: Transfer state already exists for handle %u"), __FUNCTION__, Handle);
		}
		return;
	}
	
	// No existing transfer, create new receiver state
	CreateNewReceiverTransferState(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState, ReceiverPlayerState);
}

void FEasyDataTransferStateManager::HandleSenderToReceiverConversion(int32 Handle, FEasyDataTransferState* ExistingState, 
	const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, 
	APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	if (ReceiverPlayerState && ExistingState->Receiver == ReceiverPlayerState)
	{
		// Convert existing sender-side state to receiver-side
		ConvertToReceiverState(ExistingState, TransferSize, bIsCompressed);
		UE_LOG(LogEasyDataTransferStateManager, Log, TEXT("%hs: Converted transfer %u to receiver-side state"), __FUNCTION__, Handle);
	}
	else
	{
		// Create separate receiver-side state with different handle
		CreateSeparateReceiverState(Handle, ExistingState, ChannelName, TotalChunks, TransferSize, 
			bIsCompressed, SenderPlayerState, ReceiverPlayerState);
	}
}

void FEasyDataTransferStateManager::ConvertToReceiverState(FEasyDataTransferState* TransferState, int32 TransferSize, bool bIsCompressed)
{
	TransferState->bIsReceiver = true;
	TransferState->ExpectedOriginalSize = TransferSize;
	TransferState->bExpectedCompressed = bIsCompressed;
	
	ApplyDefaultSettings(*TransferState);
	UpdateTransferTiming(*TransferState);
}

void FEasyDataTransferStateManager::CreateSeparateReceiverState(int32 OriginalHandle, FEasyDataTransferState* ExistingState, 
	const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, 
	APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	const int32 ReceiverHandle = OriginalHandle + 1000000; // Offset to avoid conflicts
	
	if (Subsystem.ActiveTransfers.Contains(ReceiverHandle))
	{
		UE_LOG(LogEasyDataTransferStateManager, Error, 
		       TEXT("%hs: Generated receiver handle %u already exists!"), __FUNCTION__, ReceiverHandle);
		return;
	}
	
	FEasyDataTransferState& ReceiverState = Subsystem.ActiveTransfers.Add(ReceiverHandle);
	InitializeReceiverState(ReceiverState, ReceiverHandle, ChannelName, TransferSize, bIsCompressed, OriginalHandle);
	
	// Copy player references from existing state
	ReceiverState.Sender = ExistingState->Sender;
	ReceiverState.Receiver = ExistingState->Receiver;
	
	ApplyDefaultSettings(ReceiverState);
	UpdateTransferTiming(ReceiverState);
	TrackTransferForPlayer(ReceiverHandle, ReceiverPlayerState);
	
	UE_LOG(LogEasyDataTransferStateManager, Log, 
	       TEXT("%hs: Created separate receiver-side transfer state %u (linked to sender %u)"), 
	       __FUNCTION__, ReceiverHandle, OriginalHandle);
}

void FEasyDataTransferStateManager::CreateNewReceiverTransferState(int32 Handle, const FString& ChannelName, 
	int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState)
{
	FEasyDataTransferState& ReceiverState = Subsystem.ActiveTransfers.Add(Handle);
	InitializeReceiverState(ReceiverState, Handle, ChannelName, TransferSize, bIsCompressed, 0);
	
	ReceiverState.Sender = SenderPlayerState;
	ReceiverState.Receiver = ReceiverPlayerState;
	
	ApplyDefaultSettings(ReceiverState);
	UpdateTransferTiming(ReceiverState);
	TrackTransferForPlayer(Handle, ReceiverPlayerState);
	
	UE_LOG(LogEasyDataTransferStateManager, Log, 
	       TEXT("%hs: Created receiver-side transfer state for handle %u"), __FUNCTION__, Handle);
}

void FEasyDataTransferStateManager::InitializeReceiverState(FEasyDataTransferState& State, int32 Handle, 
	const FString& ChannelName, int32 TransferSize, bool bIsCompressed, int32 OriginalHandle)
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

void FEasyDataTransferStateManager::ApplyDefaultSettings(FEasyDataTransferState& State)
{
	const UEasyDataTransferSettings* GlobalSettings = GetDefault<UEasyDataTransferSettings>();
	if (GlobalSettings)
	{
		State.Settings.ApplyDefaults(GlobalSettings);
	}
}

void FEasyDataTransferStateManager::UpdateTransferTiming(FEasyDataTransferState& State)
{
	if (UWorld* World = Subsystem.GetWorld())
	{
		const float CurrentTime = World->GetTimeSeconds();
		State.StartTime = CurrentTime;
		State.UpdateActivity(CurrentTime);
	}
}

void FEasyDataTransferStateManager::TrackTransferForPlayer(int32 Handle, APlayerState* PlayerState)
{
	if (PlayerState)
	{
		Subsystem.PlayerTransfers.FindOrAdd(PlayerState).Transfers.Add(Handle);
	}
}