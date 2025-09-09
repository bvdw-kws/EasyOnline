// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTransfer/EasyDataTransferTypes.h"
#include "DataTransfer/IEasyDataTransferPlayerInterface.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/PlayerState.h"

#include "EasyDataTransferSubsystem.generated.h"

class UEasyDataTransferPlayerComponent;
class UEasyDataTransferSettings;
class FEasyDataTransferChunkProcessor;
class FEasyDataTransferStateManager;
class FEasyDataTransferBandwidthManager;
class FEasyDataTransferCompressionManager;
class FEasyDataTransferValidationManager;

DECLARE_LOG_CATEGORY_EXTERN(LogEasyDataTransfer, Log, All);

// Forward declare delegates
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataReceived, int32, const FString&, const TArray<uint8>&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDataSent, int32, const FString&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataProgress, int32, const FString&, float);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataError, int32, const FString&, EDataTransferError);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataTransferClosed, int32, const FString&, const FString&);

USTRUCT()
struct EASYDATATRANSFERMODULE_API FEasyDataTransferPlayerState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<int32> Transfers;
};

/**
 * Central data transfer management subsystem.
 * Uses interface pattern to work with PlayerStates without circular dependency.
 */
UCLASS()
class EASYDATATRANSFERMODULE_API UEasyDataTransferSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface


	// Public API - works with any PlayerState that implements the interface
	
	/**
	 * Open a data channel to send data to another player.
	 * @param ChannelName Name of the data channel
	 * @param TargetPlayer Player to send data to
	 * @param Data Data to send
	 * @param Settings Transfer settings (uses defaults if not specified)
	 * @return Transfer handle, or 0 if failed to start
	 */
	UFUNCTION(BlueprintCallable, Category="EasyDataTransfer")
	int32 OpenDataChannel(const FString& ChannelName, APlayerState* TargetPlayer, 
	                      const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings = FEasyDataTransferOptions());

	/**
	 * Open a data channel to send data to another player (overload with explicit sender).
	 * This overload is useful for unit testing where sender context cannot be auto-detected.
	 * @param ChannelName Name of the data channel
	 * @param Sender Player sending the data
	 * @param TargetPlayer Player to send data to
	 * @param Data Data to send
	 * @param Settings Transfer settings (uses defaults if not specified)
	 * @return Transfer handle, or 0 if failed to start
	 */
	int32 OpenDataChannel(const FString& ChannelName, APlayerState* Sender, APlayerState* TargetPlayer, 
	                      const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings = FEasyDataTransferOptions());

	/**
	 * Get transfer data as a specific type (C++ only).
	 * Use this to retrieve typed data after a transfer completes.
	 * @param Handle Transfer handle
	 * @param OutData Output data structure
	 * @return True if data was successfully retrieved and deserialized
	 */
	template<typename T>
	bool GetTransferData(int32 Handle, T& OutData) const;
	
	/**
	 * Send data as a typed struct (C++ only).
	 * Convenience method for sending POD structs safely.
	 * @param ChannelName Name of the data channel
	 * @param TargetPlayer Player to send data to
	 * @param Data Struct data to send
	 * @param Settings Transfer settings (uses defaults if not specified)
	 * @return Transfer handle, or 0 if failed to start
	 */
	template<typename T>
	int32 SendData(const FString& ChannelName, APlayerState* TargetPlayer, const T& Data, 
	               const FEasyDataTransferOptions& Settings = FEasyDataTransferOptions());
	
	/**
	 * Get raw transfer data.
	 * @param Handle Transfer handle
	 * @return Raw data array, empty if transfer not found or not completed
	 */
	UFUNCTION(BlueprintCallable, Category="EasyDataTransfer")
	TArray<uint8> GetRawTransferData(int32 Handle) const;

	/**
	 * Close a data transfer.
	 * @param Handle Transfer handle
	 * @param Reason Reason for closing (optional)
	 */
	UFUNCTION(BlueprintCallable, Category="EasyDataTransfer")
	void CloseDataChannel(int32 Handle, const FString& Reason = TEXT("Manual close"));

	/**
	 * Get transfer progress.
	 * @param Handle Transfer handle
	 * @return Progress from 0.0 to 1.0, or -1.0 if handle is invalid
	 */
	UFUNCTION(BlueprintPure, Category="EasyDataTransfer")
	float GetTransferProgress(int32 Handle) const;

	/**
	 * Get transfer status.
	 * @param Handle Transfer handle
	 * @return Current status of the transfer
	 */
	UFUNCTION(BlueprintPure, Category="EasyDataTransfer")
	EDataTransferStatus GetTransferStatus(int32 Handle) const;

	/**
	 * Get all active transfers for a player.
	 * @param PlayerState The player state to check
	 * @return Array of active transfer handles
	 */
	UFUNCTION(BlueprintPure, Category="EasyDataTransfer")
	TArray<int32> GetActiveTransfersForPlayer(APlayerState* PlayerState) const;

	/**
	 * Get player component through interface (no circular dependency).
	 * @param PlayerState The player state
	 * @return The component, or nullptr if not available
	 */
	UEasyDataTransferPlayerComponent* GetPlayerComponent(APlayerState* PlayerState) const;

	/**
	 * Close all transfers for a player (useful when player disconnects).
	 * @param PlayerState The player state
	 * @param Reason Reason for closing (optional)
	 */
	void CloseAllTransfersForPlayer(APlayerState* PlayerState, const FString& Reason = TEXT("Player disconnected"));

	// Event handlers called by PlayerComponent
	void HandleReceivedChunk(const FEasyDataChunk& Chunk);
	void HandleTransferStarted(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState = nullptr, APlayerState* ReceiverPlayerState = nullptr);
	void HandleChunkAcknowledged(int32 Handle, int32 ChunkIndex, APlayerState* SenderPlayerState = nullptr);
	void HandleTransferComplete(int32 Handle, bool bSuccess, const FString& ErrorMessage);
	void HandleTransferCancelled(int32 Handle, const FString& Reason);
	
	// Callback delegates (C++ only for now) - Public access for helper classes
	FOnDataReceived OnDataReceived;
	FOnDataSent OnDataSent;
	FOnDataProgress OnDataProgress;
	FOnDataError OnDataError;
	FOnDataTransferClosed OnDataTransferClosed;

protected:
	/**
	 * Start a new transfer.
	 * @param ChannelName Channel name
	 * @param Sender Sending player
	 * @param Receiver Receiving player
	 * @param Data Data to transfer
	 * @param Settings Transfer settings
	 * @return Transfer handle, or 0 if failed
	 */
	int32 StartTransfer(const FString& ChannelName, APlayerState* Sender, APlayerState* Receiver, 
	                    const TArray<uint8>& Data, const FEasyDataTransferOptions& Settings);

	/**
	 * Process data for transfer (compression, chunking).
	 * @param TransferState The transfer state to process
	 * @return True if processing succeeded
	 */
	bool ProcessTransferData(FEasyDataTransferState& TransferState);


	/**
	 * Create chunks from processed data.
	 * @param TransferState The transfer state
	 * @return True if chunking succeeded
	 */
	bool CreateChunks(FEasyDataTransferState& TransferState);

	/**
	 * Send next available chunks for a transfer.
	 * @param Handle Transfer handle
	 */
	void SendNextChunks(int32 Handle);

	/**
	 * Validate transfer state for sending operations.
	 * @param Handle Transfer handle
	 * @return Transfer state if valid, nullptr otherwise
	 */
	FEasyDataTransferState* ValidateTransferForSending(int32 Handle);

	/**
	 * Get and validate receiver component for transfer.
	 * @param Handle Transfer handle
	 * @param TransferState Transfer state
	 * @return Receiver component if valid, nullptr otherwise
	 */
	UEasyDataTransferPlayerComponent* GetValidatedReceiverComponent(int32 Handle, FEasyDataTransferState* TransferState);

	/**
	 * Send new (not yet sent) chunks for a transfer.
	 * @param Handle Transfer handle
	 * @param TransferState Transfer state
	 * @param ReceiverComponent Receiver component
	 * @param CurrentTime Current world time
	 * @param MaxChunks Maximum chunks to send
	 * @return Number of chunks sent
	 */
	int32 SendNewChunks(int32 Handle, FEasyDataTransferState* TransferState, UEasyDataTransferPlayerComponent* ReceiverComponent, float CurrentTime, int32 MaxChunks);

	/**
	 * Retry sending unacknowledged chunks for a transfer.
	 * @param Handle Transfer handle
	 * @param TransferState Transfer state
	 * @param ReceiverComponent Receiver component
	 * @param CurrentTime Current world time
	 * @param MaxChunks Maximum chunks to retry
	 * @return Number of chunks retried
	 */
	int32 RetryUnacknowledgedChunks(int32 Handle, FEasyDataTransferState* TransferState, UEasyDataTransferPlayerComponent* ReceiverComponent, float CurrentTime, int32 MaxChunks);

	/**
	 * Check if transfer is complete and handle completion.
	 * @param Handle Transfer handle
	 * @param TransferState Transfer state
	 */
	void CheckTransferCompletion(int32 Handle, FEasyDataTransferState* TransferState);

	/**
	 * Complete a transfer.
	 * @param Handle Transfer handle
	 * @param bSuccess Whether transfer completed successfully
	 * @param Error Error type if failed
	 * @param ErrorMessage Optional error message for logging
	 */
	void CompleteTransfer(int32 Handle, bool bSuccess, const FString& ErrorMessage = TEXT(""));


	/**
	 * Handle existing transfer conflicts when starting a new transfer.
	 * @param Handle Transfer handle
	 * @param ChannelName Channel name
	 * @param TotalChunks Total chunks
	 * @param TransferSize Transfer size
	 * @param bIsCompressed Whether data is compressed
	 * @param SenderPlayerState Sender player state
	 * @param ReceiverPlayerState Receiver player state
	 */
	void HandleExistingTransferConflict(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Handle conversion from sender-side to receiver-side transfer state.
	 */
	void HandleSenderToReceiverConversion(int32 Handle, FEasyDataTransferState* ExistingState, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Convert an existing transfer state to receiver mode.
	 */
	void ConvertToReceiverState(FEasyDataTransferState* TransferState, int32 TransferSize, bool bIsCompressed);

	/**
	 * Create a separate receiver-side transfer state.
	 */
	void CreateSeparateReceiverState(int32 OriginalHandle, FEasyDataTransferState* ExistingState, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Create a new receiver-side transfer state.
	 */
	void CreateNewReceiverTransferState(int32 Handle, const FString& ChannelName, int32 TotalChunks, int32 TransferSize, bool bIsCompressed, APlayerState* SenderPlayerState, APlayerState* ReceiverPlayerState);

	/**
	 * Initialize a receiver transfer state with basic parameters.
	 */
	void InitializeReceiverState(FEasyDataTransferState& State, int32 Handle, const FString& ChannelName, int32 TransferSize, bool bIsCompressed, int32 OriginalHandle);

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

	/**
	 * Validate transfer settings and channel.
	 * @param ChannelName Channel name to validate
	 * @param Settings Settings to validate
	 * @param Data Data to validate
	 * @return True if validation passed
	 */
	bool ValidateTransfer(const FString& ChannelName, const FEasyDataTransferOptions& Settings, const TArray<uint8>& Data);


	/**
	 * Map error message to error enum.
	 * @param ErrorMessage The error message
	 * @return The corresponding error enum
	 */
	EDataTransferError MapErrorMessageToError(const FString& ErrorMessage);

	/**
	 * Get cached settings.
	 * @return The settings, or nullptr if not available
	 */
	const UEasyDataTransferSettings* GetSettings();


	/**
	 * Get a string representation of active transfer handles for debugging.
	 * @return Comma-separated list of active transfer handles
	 */
	FString GetActiveTransferHandlesString() const;

	/**
	 * Update transfers (called periodically).
	 * @param DeltaTime Time since last update
	 */
	void UpdateTransfers(float DeltaTime);

	/**
	 * Periodic update function called by timer.
	 */
	UFUNCTION()
	void PeriodicUpdate();

	/**
	 * Handle player state destruction.
	 * @param DestroyedActor The destroyed actor
	 */
	UFUNCTION()
	void OnPlayerStateDestroyed(AActor* DestroyedActor);

	/**
	 * Handle post world init
	 * @param World The world being initialized
	 * @param IVS Initialization values
	 */
	void OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	
	/**
	 * Handle world cleanup
	 * @param World The world being cleanup
	 * @param bSessionEnded True if session ended
	 * @param bCleanupResources True if resources should be cleanup
	 */
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	// Friend declarations for helper classes
	friend class FEasyDataTransferChunkProcessor;
	friend class FEasyDataTransferStateManager;
	friend class FEasyDataTransferBandwidthManager;
	friend class FEasyDataTransferCompressionManager;
	friend class FEasyDataTransferValidationManager;

private:
	// Handle to transfer state mapping
	UPROPERTY(Transient)
	TMap<int32, FEasyDataTransferState> ActiveTransfers;

	// Player to active transfers mapping for cleanup (uses interface)
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<APlayerState>, FEasyDataTransferPlayerState> PlayerTransfers;

	// Helper class instances
	TSharedPtr<FEasyDataTransferChunkProcessor> ChunkProcessor;
	TSharedPtr<FEasyDataTransferStateManager> StateManager;
	TSharedPtr<FEasyDataTransferBandwidthManager> BandwidthManager;
	TSharedPtr<FEasyDataTransferCompressionManager> CompressionManager;
	TSharedPtr<FEasyDataTransferValidationManager> ValidationManager;


	// Timer for periodic updates
	UPROPERTY(Transient)
	FTimerHandle UpdateTimerHandle;

	// Timer for receiver transfer cleanup (prevent memory leaks)
	UPROPERTY(Transient)
	FTimerHandle ReceiverCleanupTimerHandle;

	// Current world reference
	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> CurrentWorld;
	
	// Thread-safe timing for periodic updates
	UPROPERTY(Transient)
	float LastPeriodicUpdateTime = 0.0f;
};

// Template implementation for GetTransferData
template<typename T>
bool UEasyDataTransferSubsystem::GetTransferData(int32 Handle, T& OutData) const
{
	static_assert(TIsPODType<T>::Value || std::is_trivially_copyable_v<T>, "T must be POD or trivially copyable for safe serialization");
	
	// Get raw data
	const TArray<uint8> RawData = GetRawTransferData(Handle);
	if (RawData.Num() != sizeof(T))
	{
		UE_LOG(LogEasyDataTransfer, Warning, TEXT("GetTransferData: Size mismatch - Expected %d bytes, got %d bytes"), sizeof(T), RawData.Num());
		return false;
	}
	
	// Direct memory copy for POD types
	FMemory::Memcpy(&OutData, RawData.GetData(), sizeof(T));
	return true;
}

// Template implementation for SendData
template<typename T>
int32 UEasyDataTransferSubsystem::SendData(const FString& ChannelName, APlayerState* TargetPlayer, const T& Data, 
                                             const FEasyDataTransferOptions& Settings)
{
	static_assert(TIsPODType<T>::Value || std::is_trivially_copyable_v<T>, "T must be POD or trivially copyable for safe serialization");
	
	// Convert struct to byte array
	TArray<uint8> RawData;
	RawData.SetNum(sizeof(T));
	FMemory::Memcpy(RawData.GetData(), &Data, sizeof(T));
	
	// Use existing OpenDataChannel method
	return OpenDataChannel(ChannelName, TargetPlayer, RawData, Settings);
}