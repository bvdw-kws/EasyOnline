// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"

#include "DataTransfer/Subsystems/EasyDataTransferSubsystem.h"
#include "DataTransfer/Utils/EasyDataTransferValidation.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogEasyDataTransferComponent, Log, All);

UEasyDataTransferPlayerComponent::UEasyDataTransferPlayerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Enable replication
	SetIsReplicatedByDefault(true);
	
	// Set tick to false as we don't need per-frame updates
	PrimaryComponentTick.bCanEverTick = false;
}

void UEasyDataTransferPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Component is ready, register with subsystem if needed
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		// Subsystem can now track this component
	}
}

void UEasyDataTransferPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up any active transfers
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		if (APlayerState* OwnerPlayerState = Cast<APlayerState>(GetOwner()))
		{
			Subsystem->CloseAllTransfersForPlayer(OwnerPlayerState, TEXT("Player component destroyed"));
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void UEasyDataTransferPlayerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UEasyDataTransferPlayerComponent, ActiveTransferHandles);
}

void UEasyDataTransferPlayerComponent::SendDataChunk(const FEasyDataChunk& Chunk)
{
	// Rate limiting to prevent RPC spam
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastChunkSendTime < MinChunkSendInterval)
	{
		return; // Rate limiting active
	}
	LastChunkSendTime = CurrentTime;
	
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Sending chunk %d/%d for transfer %u (size: %d bytes)"), 
	       __FUNCTION__, Chunk.ChunkIndex + 1, Chunk.TotalChunks, Chunk.TransferHandle, Chunk.Data.Num());
	
	// Determine if we're sending to server or client
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		// Client to server
		Server_ReceiveDataChunk(Chunk);
	}
	else if (GetOwnerRole() == ROLE_Authority)
	{
		// Server to client
		Client_ReceiveDataChunk(Chunk);
	}
}

void UEasyDataTransferPlayerComponent::SendTransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, bool bToServer)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Called with bToServer=%s, Owner=%s, Role=%s, Sender=%s"), 
	       __FUNCTION__, bToServer ? TEXT("Yes") : TEXT("No"), 
	       GetOwner() ? *GetOwner()->GetName() : TEXT("null"),
	       GetOwnerRole() == ROLE_Authority ? TEXT("Authority") : TEXT("AutonomousProxy"),
	       SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"));
	
	if (bToServer)
	{
		UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Calling Server_TransferStarted"), __FUNCTION__);
		Server_TransferStarted(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState);
	}
	else
	{
		UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Calling Client_TransferStarted"), __FUNCTION__);
		Client_TransferStarted(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState);
	}
}

void UEasyDataTransferPlayerComponent::SendChunkAcknowledgment(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState, bool bToServer)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Sending ACK for chunk %d of transfer %d - bToServer=%s, Sender=%s, ComponentOwner=%s, ComponentRole=%s"),
	       __FUNCTION__, ChunkIndex, Handle, bToServer ? TEXT("Yes") : TEXT("No"),
	       SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"),
	       GetOwner() ? *GetOwner()->GetName() : TEXT("null"),
	       GetOwnerRole() == ROLE_Authority ? TEXT("Authority") : TEXT("AutonomousProxy"));
	       
	if (bToServer)
	{
		UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Calling Server_AcknowledgeChunk RPC"), __FUNCTION__);
		Server_AcknowledgeChunk(Handle, ChunkIndex, SenderPlayerState);
	}
	else
	{
		UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Calling Client_AcknowledgeChunk RPC"), __FUNCTION__);
		Client_AcknowledgeChunk(Handle, ChunkIndex, SenderPlayerState);
	}
}

void UEasyDataTransferPlayerComponent::SendTransferComplete(int32 Handle, bool bSuccess, const FString& ErrorMessage, bool bToServer)
{
	if (bToServer)
	{
		Server_TransferComplete(Handle, bSuccess, ErrorMessage);
	}
	else
	{
		Client_TransferComplete(Handle, bSuccess, ErrorMessage);
	}
}

void UEasyDataTransferPlayerComponent::SendTransferCancelled(int32 Handle, const FString& Reason)
{
	NetMulticast_TransferCancelled(Handle, Reason);
}

bool UEasyDataTransferPlayerComponent::Server_ReceiveDataChunk_Validate(const FEasyDataChunk& Chunk)
{
	return FEasyDataTransferValidation::ValidateChunk(Chunk, TEXT("Server_ReceiveDataChunk_Validate"));
}

bool UEasyDataTransferPlayerComponent::Client_ReceiveDataChunk_Validate(const FEasyDataChunk& Chunk)
{
	return FEasyDataTransferValidation::ValidateChunk(Chunk, TEXT("Client_ReceiveDataChunk_Validate"));
}

#if WITH_AUTOMATION_TESTS
bool UEasyDataTransferPlayerComponent::TestValidateChunk(const FEasyDataChunk& Chunk) const
{
	return FEasyDataTransferValidation::ValidateChunk(Chunk, TEXT("TestValidateChunk"));
}
#endif

void UEasyDataTransferPlayerComponent::Server_ReceiveDataChunk_Implementation(const FEasyDataChunk& Chunk)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Server received chunk %d/%d for transfer %u"), 
	       __FUNCTION__, Chunk.ChunkIndex + 1, Chunk.TotalChunks, Chunk.TransferHandle);
	HandleReceivedChunk(Chunk);
}

void UEasyDataTransferPlayerComponent::Client_ReceiveDataChunk_Implementation(const FEasyDataChunk& Chunk)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Client received chunk %d/%d for transfer %u"), 
	       __FUNCTION__, Chunk.ChunkIndex + 1, Chunk.TotalChunks, Chunk.TransferHandle);
	HandleReceivedChunk(Chunk);
}

void UEasyDataTransferPlayerComponent::Client_TransferStarted_Implementation(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Client received TransferStarted RPC - Handle: %u, Channel: %s, Chunks: %d, Size: %d, Compressed: %s, Owner: %s, Sender: %s"), 
	       __FUNCTION__, Handle, *ChannelName, TotalChunks, TransferSize, bIsCompressed ? TEXT("Yes") : TEXT("No"),
	       GetOwner() ? *GetOwner()->GetName() : TEXT("null"),
	       SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"));
	
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		// Pass the receiver (this component's owner) and sender to the subsystem
		APlayerState* ReceiverPlayerState = GetPlayerState<APlayerState>();
		Subsystem->HandleTransferStarted(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState, ReceiverPlayerState);
	}
	else
	{
		UE_LOG(LogEasyDataTransferComponent, Error, TEXT("%hs: Failed to get EasyDataTransfer subsystem for TransferStarted RPC"), __FUNCTION__);
	}
}

void UEasyDataTransferPlayerComponent::Server_TransferStarted_Implementation(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Server received TransferStarted RPC - Handle: %u, Channel: %s, Chunks: %d, Size: %d, Compressed: %s, Owner: %s, Sender: %s"), 
	       __FUNCTION__, Handle, *ChannelName, TotalChunks, TransferSize, bIsCompressed ? TEXT("Yes") : TEXT("No"),
	       GetOwner() ? *GetOwner()->GetName() : TEXT("null"),
	       SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"));
	
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		// Pass the receiver (this component's owner) and sender to the subsystem
		APlayerState* ReceiverPlayerState = GetPlayerState<APlayerState>();
		Subsystem->HandleTransferStarted(Handle, ChannelName, TotalChunks, TransferSize, bIsCompressed, SenderPlayerState, ReceiverPlayerState);
	}
	else
	{
		UE_LOG(LogEasyDataTransferComponent, Error, TEXT("%hs: Failed to get EasyDataTransfer subsystem for TransferStarted RPC"), __FUNCTION__);
	}
}

void UEasyDataTransferPlayerComponent::Server_AcknowledgeChunk_Implementation(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Server received ACK RPC for chunk %d of transfer %d - Owner: %s, Sender: %s"),
	       __FUNCTION__, ChunkIndex, Handle, GetOwner() ? *GetOwner()->GetName() : TEXT("null"),
	       SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"));
	       
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Calling subsystem HandleChunkAcknowledged"), __FUNCTION__);
		Subsystem->HandleChunkAcknowledged(Handle, ChunkIndex, SenderPlayerState);
	}
	else
	{
		UE_LOG(LogEasyDataTransferComponent, Error, TEXT("%hs: No subsystem found for ACK processing"), __FUNCTION__);
	}
}

void UEasyDataTransferPlayerComponent::Client_AcknowledgeChunk_Implementation(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState)
{
	UE_LOG(LogEasyDataTransferComponent, Log, TEXT("%hs: Client received ACK for chunk %d of transfer %d - Owner: %s, Sender: %s"),
	       __FUNCTION__, ChunkIndex, Handle, GetOwner() ? *GetOwner()->GetName() : TEXT("null"),
	       SenderPlayerState ? *SenderPlayerState->GetName() : TEXT("null"));
	       
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		Subsystem->HandleChunkAcknowledged(Handle, ChunkIndex, SenderPlayerState);
	}
}

void UEasyDataTransferPlayerComponent::Client_TransferComplete_Implementation(int32 Handle, bool bSuccess, const FString& ErrorMessage)
{
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		Subsystem->HandleTransferComplete(Handle, bSuccess, ErrorMessage);
	}
}

void UEasyDataTransferPlayerComponent::Server_TransferComplete_Implementation(int32 Handle, bool bSuccess, const FString& ErrorMessage)
{
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		Subsystem->HandleTransferComplete(Handle, bSuccess, ErrorMessage);
	}
}

void UEasyDataTransferPlayerComponent::NetMulticast_TransferCancelled_Implementation(int32 Handle, const FString& Reason)
{
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		Subsystem->HandleTransferCancelled(Handle, Reason);
	}
}


void UEasyDataTransferPlayerComponent::HandleReceivedChunk(const FEasyDataChunk& Chunk)
{
	if (UEasyDataTransferSubsystem* Subsystem = GetDataTransferSubsystem())
	{
		Subsystem->HandleReceivedChunk(Chunk);
	}
}

UEasyDataTransferSubsystem* UEasyDataTransferPlayerComponent::GetDataTransferSubsystem() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UEasyDataTransferSubsystem>();
		}
	}
	
	return nullptr;
}