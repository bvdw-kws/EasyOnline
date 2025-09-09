// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Components/PlayerStateComponent.h"
#include "DataTransfer/EasyDataTransferTypes.h"
#include "Net/UnrealNetwork.h"

#include "EasyDataTransferPlayerComponent.generated.h"

/**
 * Player state component for handling data transfers.
 * Must inherit from UPlayerStateComponent for proper networking.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EASYDATATRANSFERMODULE_API UEasyDataTransferPlayerComponent : public UPlayerStateComponent
{
	GENERATED_BODY()

public:
	UEasyDataTransferPlayerComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

	/**
	 * Send a data chunk to this component's owner (unreliable RPC).
	 * @param Chunk The data chunk to send
	 */
	void SendDataChunk(const FEasyDataChunk& Chunk);

	/**
	 * Send control message for transfer start.
	 * @param Handle Transfer handle
	 * @param ChannelName Channel name
	 * @param TotalChunks Total number of chunks
	 * @param TransferSize Total transfer size
	 * @param bIsCompressed Whether the data is compressed
	 * @param SenderPlayerState The PlayerState that initiated the transfer
	 * @param bToServer True if sending to server, false if sending to client
	 */
	void SendTransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, bool bToServer);

	/**
	 * Send chunk acknowledgment.
	 * @param Handle Transfer handle
	 * @param ChunkIndex Index of acknowledged chunk
	 * @param SenderPlayerState The PlayerState that initiated the transfer
	 * @param bToServer True if sending to server, false if sending to client
	 */
	void SendChunkAcknowledgment(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState, bool bToServer);

	/**
	 * Send transfer completion notification.
	 * @param Handle Transfer handle
	 * @param bSuccess Whether transfer completed successfully
	 * @param ErrorMessage Error message if failed
	 * @param bToServer True if sending to server, false if sending to client
	 */
	void SendTransferComplete(int32 Handle, bool bSuccess, const FString& ErrorMessage, bool bToServer);

	/**
	 * Send transfer cancellation notification.
	 * @param Handle Transfer handle
	 * @param Reason Cancellation reason
	 */
	void SendTransferCancelled(int32 Handle, const FString& Reason);

	// Testing access methods
#if WITH_AUTOMATION_TESTS
	/**
	 * Get active transfer handles for testing.
	 * @return Array of active transfer handles
	 */
	const TArray<int32>& GetActiveTransferHandles() const { return ActiveTransferHandles; }
	
	/**
	 * Test chunk validation.
	 * @param Chunk The chunk to validate
	 * @return True if chunk is valid
	 */
	bool TestValidateChunk(const FEasyDataChunk& Chunk) const;
#endif

protected:
	// RPC validation as recommended by Unreal documentation
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_ReceiveDataChunk(const FEasyDataChunk& Chunk);
	bool Server_ReceiveDataChunk_Validate(const FEasyDataChunk& Chunk);
	void Server_ReceiveDataChunk_Implementation(const FEasyDataChunk& Chunk);

	UFUNCTION(Client, Unreliable, WithValidation)
	void Client_ReceiveDataChunk(const FEasyDataChunk& Chunk);
	bool Client_ReceiveDataChunk_Validate(const FEasyDataChunk& Chunk);
	void Client_ReceiveDataChunk_Implementation(const FEasyDataChunk& Chunk);

	// Reliable control messages
	UFUNCTION(Client, Reliable)
	void Client_TransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState);

	UFUNCTION(Server, Reliable)
	void Server_TransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState);

	UFUNCTION(Server, Reliable)
	void Server_AcknowledgeChunk(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState);

	UFUNCTION(Client, Reliable)
	void Client_AcknowledgeChunk(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState);

	UFUNCTION(Client, Reliable)
	void Client_TransferComplete(int32 Handle, bool bSuccess, const FString& ErrorMessage);

	UFUNCTION(Server, Reliable)
	void Server_TransferComplete(int32 Handle, bool bSuccess, const FString& ErrorMessage);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_TransferCancelled(int32 Handle, const FString& Reason);

private:

	/**
	 * Handle received data chunk.
	 * @param Chunk The received chunk
	 */
	void HandleReceivedChunk(const FEasyDataChunk& Chunk);

	/**
	 * Get the data transfer subsystem.
	 * @return The subsystem, or nullptr if not available
	 */
	class UEasyDataTransferSubsystem* GetDataTransferSubsystem() const;

	// Replicated properties for network state sync
	UPROPERTY(Replicated)
	TArray<int32> ActiveTransferHandles;

	// Rate limiting for unreliable RPCs
	float LastChunkSendTime = 0.0f;
	static constexpr float MinChunkSendInterval = 0.001f; // 1ms minimum interval
};