// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/PlayerState.h"
#include <atomic>

#include "EasyDataTransferTypes.generated.h"

class UEasyDataTransferSettings;
class FEasyDataCompressionTask;

/**
 * Status of a data transfer.
 */
UENUM(BlueprintType)
enum class EDataTransferStatus : uint8
{
	Invalid,        // Handle is invalid
	Pending,        // Transfer is queued but not started
	Compressing,    // Transfer is compressing data asynchronously
	InProgress,     // Transfer is actively sending/receiving data
	Completed,      // Transfer completed successfully
	Failed,         // Transfer failed with error
	Cancelled       // Transfer was cancelled
};

/**
 * Errors that can occur during data transfer.
 */
UENUM(BlueprintType)
enum class EDataTransferError : uint8
{
	None,
	Timeout,
	NetworkError,
	ValidationError,
	CompressionError,
	SizeLimitExceeded,
	TooManyConcurrentTransfers,
	PlayerDisconnected,
	ChannelNotAllowed,
	UnknownError
};

/**
 * Per-transfer settings that can override global defaults.
 */
USTRUCT(BlueprintType)
struct EASYDATATRANSFERMODULE_API FEasyDataTransferOptions
{
	GENERATED_BODY()

	// Transfer priority (higher = more bandwidth allocation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	int32 Priority = 0;
	
	// Chunk size in bytes (0 = use global default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	int32 ChunkSize = 0;
	
	// Enable compression (uses global default if not specified)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	bool bEnableCompression = true;
	
	// Timeout in seconds (0 = use global default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	float TimeoutSeconds = 0.0f;
	
	// Maximum retry attempts (0 = use global default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	int32 MaxRetries = 0;
	
	// Bandwidth limit in bytes/sec (0 = no specific limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	int32 BandwidthLimit = 0;
	
	// Enable adaptive chunk sizing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer")
	bool bAdaptiveChunking = true;

	// Sliding window size for flow control (0 = use global default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transfer", meta=(ClampMin=1, ClampMax=50))
	int32 SlidingWindowSize = 0;

	/**
	 * Apply global defaults where not specified.
	 * @param GlobalSettings The global settings to use for defaults
	 */
	void ApplyDefaults(const UEasyDataTransferSettings* GlobalSettings);
};

/**
 * Data chunk for network transmission.
 * Uses custom NetSerialization for efficient networking.
 */
USTRUCT()
struct EASYDATATRANSFERMODULE_API FEasyDataChunk
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 TransferHandle = 0;
	
	UPROPERTY()
	int32 ChunkIndex = 0;
	
	UPROPERTY()
	int32 TotalChunks = 0;
	
	UPROPERTY()
	TArray<uint8> Data;
	
	UPROPERTY()
	int32 Checksum = 0;
	
	/**
	 * Custom network serialization for efficient data transfer.
	 * @param Ar The network archive
	 * @param Map The package map
	 * @param bOutSuccess Whether serialization succeeded
	 * @return True if serialization was handled
	 */
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FEasyDataChunk> : public TStructOpsTypeTraitsBase2<FEasyDataChunk>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * Internal state of an active data transfer.
 */
USTRUCT()
struct EASYDATATRANSFERMODULE_API FEasyDataTransferState
{
	GENERATED_BODY()

	// Transfer identification
	UPROPERTY()
	int32 Handle = 0;
	
	UPROPERTY()
	FString ChannelName;
	
	// Participants
	UPROPERTY()
	TWeakObjectPtr<APlayerState> Sender;
	
	UPROPERTY()
	TWeakObjectPtr<APlayerState> Receiver;
	
	// Transfer data
	UPROPERTY()
	TArray<uint8> OriginalData;
	
	UPROPERTY()
	TArray<uint8> ProcessedData; // Compressed if compression enabled
	
	UPROPERTY()
	bool bIsCompressed = false;
	
	UPROPERTY()
	int32 OriginalSize = 0;
	
	// Chunking state
	UPROPERTY()
	TArray<FEasyDataChunk> Chunks;
	
	UPROPERTY()
	TSet<int32> AcknowledgedChunks;
	
	UPROPERTY()
	int32 NextChunkToSend = 0;
	
	// RECEIVER-SIDE CHUNK TRACKING (NEW)
	UPROPERTY()
	TMap<int32, FEasyDataChunk> ReceivedChunks; // ChunkIndex -> Chunk mapping
	
	UPROPERTY()
	TArray<uint8> ReassembledData; // Final reassembled data after decompression
	
	UPROPERTY()
	bool bIsReceiver = false; // True if this is the receiver's transfer state
	
	// METADATA FOR RECEIVER (NEW)
	UPROPERTY()
	int32 ExpectedOriginalSize = 0; // Original uncompressed size
	
	UPROPERTY()
	bool bExpectedCompressed = false; // Whether data was compressed by sender
	
	// Link to original sender handle (for receiver-side states that share subsystem with sender)
	UPROPERTY()
	int32 OriginalSenderHandle = 0; // Handle of the original sender-side transfer state
	
	// Settings and status
	UPROPERTY()
	FEasyDataTransferOptions Settings;
	
	UPROPERTY()
	EDataTransferStatus Status = EDataTransferStatus::Pending;
	
	UPROPERTY()
	EDataTransferError LastError = EDataTransferError::None;
	
	// Timing
	UPROPERTY()
	float StartTime = 0.0f;
	
	UPROPERTY()
	float LastActivityTime = 0.0f;
	
	UPROPERTY()
	int32 RetryCount = 0;
	
	// Bandwidth tracking
	UPROPERTY()
	int32 BytesSent = 0;
	
	UPROPERTY()
	int32 BytesReceived = 0;
	
	UPROPERTY()
	float LastBandwidthSample = 0.0f;

	// Async compression task handle (non-serialized)
	TSharedPtr<FAsyncTask<FEasyDataCompressionTask>> CompressionTask;

	/**
	 * Get transfer progress as a percentage.
	 * @return Progress from 0.0 to 1.0
	 */
	float GetProgress() const;
	
	/**
	 * Check if the transfer has timed out.
	 * @param CurrentTime The current world time
	 * @return True if the transfer has timed out
	 */
	bool HasTimedOut(float CurrentTime) const;
	
	/**
	 * Update the last activity time.
	 * @param CurrentTime The current world time
	 */
	void UpdateActivity(float CurrentTime);
	
	/**
	 * Check if we can send the next chunk (sliding window protocol).
	 * @return True if we can send the next chunk
	 */
	bool CanSendNextChunk() const;
	
	/**
	 * Check if all chunks have been received (receiver-side).
	 * @return True if all chunks are received
	 */
	bool AreAllChunksReceived() const;
	
	/**
	 * Reassemble data from received chunks and decompress if needed.
	 * @return True if reassembly and decompression succeeded
	 */
	bool ReassembleAndDecompressData();
};

/**
 * Thread-safe handle generator for transfer handles.
 * Uses atomic operations to ensure uniqueness across network.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferHandleGenerator
{
public:
	/**
	 * Generate a unique transfer handle.
	 * @return A unique handle (never 0)
	 */
	static int32 GenerateHandle();

	/**
	 * Check if a handle is valid.
	 * @param Handle The handle to check
	 * @return True if the handle is valid (not 0)
	 */
	static bool IsValidHandle(int32 Handle);

private:
	static std::atomic<int32> Counter;
};

/**
 * Compression utility using Unreal's built-in FCompression.
 * Handles automatic compression/decompression with fallback.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferCompression
{
public:
	/**
	 * Compress data using Unreal's built-in compression.
	 * @param Input Raw data to compress
	 * @param Output Compressed data output
	 * @return True if compression was successful and beneficial
	 */
	static bool CompressData(const TArray<uint8>& Input, TArray<uint8>& Output);

	/**
	 * Decompress data.
	 * @param Input Compressed data
	 * @param Output Decompressed data output
	 * @param UncompressedSize Expected uncompressed size
	 * @return True if decompression was successful
	 */
	static bool DecompressData(const TArray<uint8>& Input, TArray<uint8>& Output, int32 UncompressedSize);

	/**
	 * Calculate checksum for data integrity.
	 * @param Data The data to checksum
	 * @return CRC32 checksum
	 */
	static int32 CalculateChecksum(const TArray<uint8>& Data);
};

/**
 * Async compression task for background data compression.
 * Handles both compression and decompression operations asynchronously.
 */
class EASYDATATRANSFERMODULE_API FEasyDataCompressionTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FEasyDataCompressionTask>;

public:
	FEasyDataCompressionTask(
		const TArray<uint8>& InData,
		bool bInCompress,
		int32 InUncompressedSize = 0)
		: InputData(InData)
		, bCompress(bInCompress)
		, UncompressedSize(InUncompressedSize)
		, bSuccess(false)
		, bCancelled(false)
	{
	}

	// FNonAbandonableTask interface
	void DoWork();
	
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FEasyDataCompressionTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	// Getters for results
	const TArray<uint8>& GetOutputData() const { return OutputData; }
	bool WasSuccessful() const { return bSuccess && !bCancelled; }
	bool WasBeneficial() const { return bBeneficial; }
	
	// Cancel the task
	void Cancel() { bCancelled.store(true); }
	bool IsCancelled() const { return bCancelled.load(); }

private:
	// Input
	TArray<uint8> InputData;
	bool bCompress;
	int32 UncompressedSize;
	
	// Output
	TArray<uint8> OutputData;
	bool bSuccess;
	bool bBeneficial;
	
	// Cancellation flag
	std::atomic<bool> bCancelled;
};

// Forward declarations for delegates
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataReceived, int32 /*Handle*/, const FString& /*ChannelName*/, const TArray<uint8>& /*Data*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDataSent, int32 /*Handle*/, const FString& /*ChannelName*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataProgress, int32 /*Handle*/, const FString& /*ChannelName*/, float /*Progress*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataError, int32 /*Handle*/, const FString& /*ChannelName*/, EDataTransferError /*Error*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataTransferClosed, int32 /*Handle*/, const FString& /*ChannelName*/, const FString& /*Reason*/);

