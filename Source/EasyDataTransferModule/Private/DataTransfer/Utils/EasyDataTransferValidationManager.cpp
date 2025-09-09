// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Utils/EasyDataTransferValidationManager.h"
#include "DataTransfer/Subsystems/EasyDataTransferSubsystem.h"
#include "DataTransfer/Settings/EasyDataTransferSettings.h"
#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"
#include "GameFramework/PlayerState.h"


FEasyDataTransferValidationManager::FEasyDataTransferValidationManager(UEasyDataTransferSubsystem& InSubsystem)
	: Subsystem(InSubsystem)
{
}

bool FEasyDataTransferValidationManager::ValidateNewTransfer(const FString& ChannelName, const FEasyDataTransferOptions& Settings,
                                                             const TArray<uint8>& Data, APlayerState* Sender, APlayerState* Receiver,
                                                             FString& OutErrorMessage)
{
	// Validate sender and receiver
	if (!Sender)
	{
		OutErrorMessage = TEXT("Invalid sender");
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	if (!Receiver)
	{
		OutErrorMessage = TEXT("Invalid receiver");
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	// Get global settings
	const UEasyDataTransferSettings* GlobalSettings = GetDefault<UEasyDataTransferSettings>();
	if (!GlobalSettings)
	{
		OutErrorMessage = TEXT("Global settings not available");
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	// Validate data size
	if (!ValidateDataSize(Data.Num(), GlobalSettings))
	{
		OutErrorMessage = FString::Printf(TEXT("Transfer too large - %d bytes (max %d)"), 
		                                  Data.Num(), GlobalSettings->MaxTransferSize);
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	// Validate channel whitelist
	if (!ValidateChannelWhitelist(ChannelName, GlobalSettings))
	{
		OutErrorMessage = FString::Printf(TEXT("Channel '%s' not in whitelist"), *ChannelName);
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	// Validate global concurrent transfer limits
	if (!ValidateGlobalLimits(GlobalSettings))
	{
		OutErrorMessage = FString::Printf(TEXT("Too many concurrent transfers - %d (max %d)"), 
		                                  Subsystem.ActiveTransfers.Num(), GlobalSettings->MaxConcurrentTransfersTotal);
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	// Check per-player concurrent transfer limits for sender
	if (HasReachedTransferLimit(Sender, true))
	{
		OutErrorMessage = FString::Printf(TEXT("Sender has too many concurrent transfers - %d (max %d)"), 
		                                  CountActiveTransfersForPlayer(Sender, true), 
		                                  GlobalSettings->MaxConcurrentTransfersPerPlayer);
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	// Check per-player concurrent transfer limits for receiver
	if (HasReachedTransferLimit(Receiver, false))
	{
		OutErrorMessage = FString::Printf(TEXT("Receiver has too many concurrent transfers - %d (max %d)"), 
		                                  CountActiveTransfersForPlayer(Receiver, false), 
		                                  GlobalSettings->MaxConcurrentTransfersPerPlayer);
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s"), __FUNCTION__, *OutErrorMessage);
		return false;
	}

	return true;
}

bool FEasyDataTransferValidationManager::ValidateTransferForSending(int32 Handle, FEasyDataTransferState*& OutTransferState)
{
	OutTransferState = Subsystem.ActiveTransfers.Find(Handle);
	if (!OutTransferState || OutTransferState->Status != EDataTransferStatus::InProgress)
	{
		// Transfer not found or inactive
		return false;
	}

	if (OutTransferState->bIsReceiver)
	{
		// Receiver-side transfers are handled elsewhere
		return false;
	}

	return true;
}

bool FEasyDataTransferValidationManager::ValidateReceiverComponent(int32 Handle, FEasyDataTransferState* TransferState,
                                                                   UEasyDataTransferPlayerComponent*& OutComponent,
                                                                   FString& OutErrorMessage)
{
	if (!TransferState)
	{
		OutErrorMessage = TEXT("Invalid transfer state");
		return false;
	}

	OutComponent = Subsystem.GetPlayerComponent(TransferState->Receiver.Get());
	if (!OutComponent)
	{
		OutErrorMessage = TEXT("Receiver component not available");
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("%hs: %s for transfer %u (Receiver: %s)"), 
		       __FUNCTION__, *OutErrorMessage, Handle, 
		       TransferState->Receiver.IsValid() ? *TransferState->Receiver->GetName() : TEXT("null"));
		return false;
	}

	return true;
}

bool FEasyDataTransferValidationManager::HasReachedTransferLimit(APlayerState* PlayerState, bool bAsSender) const
{
	const UEasyDataTransferSettings* GlobalSettings = GetDefault<UEasyDataTransferSettings>();
	if (!GlobalSettings)
	{
		return false;
	}

	const int32 ActiveTransfers = CountActiveTransfersForPlayer(PlayerState, bAsSender);
	return ActiveTransfers >= GlobalSettings->MaxConcurrentTransfersPerPlayer;
}

EDataTransferError FEasyDataTransferValidationManager::MapErrorMessageToError(const FString& ErrorMessage)
{
	if (ErrorMessage.Contains(TEXT("timeout"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::Timeout;
	}
	else if (ErrorMessage.Contains(TEXT("network"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::NetworkError;
	}
	else if (ErrorMessage.Contains(TEXT("validation"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::ValidationError;
	}
	else if (ErrorMessage.Contains(TEXT("compression"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::CompressionError;
	}
	else if (ErrorMessage.Contains(TEXT("size"), ESearchCase::IgnoreCase) || ErrorMessage.Contains(TEXT("limit"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::SizeLimitExceeded;
	}
	else if (ErrorMessage.Contains(TEXT("concurrent"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::TooManyConcurrentTransfers;
	}
	else if (ErrorMessage.Contains(TEXT("disconnect"), ESearchCase::IgnoreCase) || ErrorMessage.Contains(TEXT("destroy"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::PlayerDisconnected;
	}
	else if (ErrorMessage.Contains(TEXT("channel"), ESearchCase::IgnoreCase) || ErrorMessage.Contains(TEXT("whitelist"), ESearchCase::IgnoreCase))
	{
		return EDataTransferError::ChannelNotAllowed;
	}

	return EDataTransferError::UnknownError;
}

bool FEasyDataTransferValidationManager::ValidateChannelWhitelist(const FString& ChannelName, const UEasyDataTransferSettings* Settings) const
{
	if (!Settings->bRequireChannelWhitelist)
	{
		return true;
	}

	// Check channel whitelist with case-insensitive comparison
	for (const FString& AllowedChannel : Settings->AllowedChannelNames)
	{
		if (AllowedChannel.Equals(ChannelName, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

bool FEasyDataTransferValidationManager::ValidateDataSize(int32 DataSize, const UEasyDataTransferSettings* Settings) const
{
	return DataSize <= Settings->MaxTransferSize;
}

bool FEasyDataTransferValidationManager::ValidateGlobalLimits(const UEasyDataTransferSettings* Settings) const
{
	return Subsystem.ActiveTransfers.Num() < Settings->MaxConcurrentTransfersTotal;
}

int32 FEasyDataTransferValidationManager::CountActiveTransfersForPlayer(APlayerState* PlayerState, bool bAsSender) const
{
	int32 Count = 0;
	for (const auto& TransferPair : Subsystem.ActiveTransfers)
	{
		if (bAsSender)
		{
			if (TransferPair.Value.Sender == PlayerState)
			{
				Count++;
			}
		}
		else
		{
			if (TransferPair.Value.Receiver == PlayerState)
			{
				Count++;
			}
		}
	}
	return Count;
}