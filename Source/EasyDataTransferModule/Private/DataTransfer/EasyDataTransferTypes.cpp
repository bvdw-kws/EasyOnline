// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/EasyDataTransferTypes.h"

#include "DataTransfer/Settings/EasyDataTransferSettings.h"
#include "DataTransfer/Utils/EasyDataTransferValidation.h"
#include "Engine/World.h"
#include "Misc/Compression.h"
#if WITH_EDITOR || UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#include "Misc/CRC.h"
#endif

void FEasyDataTransferOptions::ApplyDefaults(const UEasyDataTransferSettings* GlobalSettings)
{
	if (!GlobalSettings)
	{
		return;
	}

	if (ChunkSize <= 0)
	{
		ChunkSize = GlobalSettings->DefaultChunkSize;
	}
	
	if (TimeoutSeconds <= 0.0f)
	{
		TimeoutSeconds = GlobalSettings->DefaultTimeoutSeconds;
	}
	
	if (MaxRetries <= 0)
	{
		MaxRetries = GlobalSettings->DefaultMaxRetries;
	}
	
	if (SlidingWindowSize <= 0)
	{
		SlidingWindowSize = GlobalSettings->DefaultSlidingWindowSize;
	}
	
	// Use global defaults for boolean values if they weren't explicitly set
	// Note: Boolean defaults are handled at the settings level since there's no 
	//       "unset" state for bools
}

bool FEasyDataChunk::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = false;
	
	// Validate archive state first
	if (Ar.IsError())
	{
		UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Archive is in error state"), __FUNCTION__);
		return false;
	}
	
	// Serialize transfer handle
	Ar << TransferHandle;
	
	// Validate handle on loading
	if (Ar.IsLoading() && !FEasyDataTransferHandleGenerator::IsValidHandle(TransferHandle))
	{
		UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Invalid transfer handle %d"), __FUNCTION__, TransferHandle);
		return false;
	}
	
	// Serialize chunk metadata
	Ar << ChunkIndex;
	Ar << TotalChunks;
	Ar << Checksum;
	
	// Validate chunk metadata on loading
	if (Ar.IsLoading())
	{
		// Validate chunk indices using centralized validation
		if (ChunkIndex < 0 || TotalChunks <= 0 || ChunkIndex >= TotalChunks)
		{
			UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Invalid chunk metadata - Index:%d, Total:%d"), 
				   __FUNCTION__, ChunkIndex, TotalChunks);
			return false;
		}
		
		// Prevent unreasonable chunk counts (DoS protection)
		if (TotalChunks > FEasyDataTransferValidation::MaxChunksPerTransfer)
		{
			UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Too many chunks %d (max %d)"), 
				__FUNCTION__, TotalChunks, FEasyDataTransferValidation::MaxChunksPerTransfer);
			return false;
		}
	}
	
	// Serialize data array
	int32 DataSize = Data.Num();
	Ar << DataSize;
	
	if (Ar.IsLoading())
	{
		// Use centralized validation for data size
		if (DataSize < 0)
		{
			UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Negative data size %d"), __FUNCTION__, DataSize);
			return false;
		}
		
		if (DataSize > FEasyDataTransferValidation::MaxChunkSize)
		{
			UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Data size %d exceeds maximum %d"), 
				   __FUNCTION__, DataSize, FEasyDataTransferValidation::MaxChunkSize);
			return false;
		}
		
		// Allocate data buffer with zero-initialization for safety
		Data.SetNum(DataSize);
	}
	
	// Serialize data if present
	if (DataSize > 0)
	{
		Ar.Serialize(Data.GetData(), DataSize);
		
		// Validate checksum on loading for data integrity
		if (Ar.IsLoading())
		{
			const int32 CalculatedChecksum = FEasyDataTransferCompression::CalculateChecksum(Data);
			if (CalculatedChecksum != Checksum)
			{
				UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Checksum mismatch - Expected:%d, Calculated:%d"), 
					   __FUNCTION__, Checksum, CalculatedChecksum);
				return false;
			}
		}
	}
	else if (Ar.IsLoading() && Checksum != 0)
	{
		// Empty data should have zero checksum
		UE_LOG(LogNetSerialization, Warning, TEXT("%hs: Empty data with non-zero checksum %d"), __FUNCTION__, Checksum);
		return false;
	}
	
	bOutSuccess = true;
	return true;
}

float FEasyDataTransferState::GetProgress() const
{
	if (bIsReceiver)
	{
		// Receiver-side progress: based on received chunks
		if (ReceivedChunks.Num() == 0)
		{
			return 0.0f;
		}
		
		// Find total chunks from received chunk metadata
		int32 TotalChunks = 0;
		for (const auto& ChunkPair : ReceivedChunks)
		{
			TotalChunks = FMath::Max(TotalChunks, ChunkPair.Value.TotalChunks);
		}
		
		if (TotalChunks == 0)
		{
			return 0.0f;
		}
		
		// Count sequential chunks from 0 to TotalChunks-1
		int32 SequentialChunksReceived = 0;
		for (int32 i = 0; i < TotalChunks; ++i)
		{
			if (ReceivedChunks.Contains(i))
			{
				SequentialChunksReceived++;
			}
		}
		
		// Prevent division by zero and overflow
		if (TotalChunks > 0 && SequentialChunksReceived >= 0)
		{
			return FMath::Clamp(static_cast<float>(SequentialChunksReceived) / static_cast<float>(TotalChunks), 0.0f, 1.0f);
		}
		
		return 0.0f;
	}
	else
	{
		// Sender-side progress: based on acknowledged chunks
		if (Chunks.Num() == 0)
		{
			return 0.0f;
		}
		
		// Prevent division by zero and overflow
		if (Chunks.Num() > 0 && AcknowledgedChunks.Num() >= 0)
		{
			return FMath::Clamp(static_cast<float>(AcknowledgedChunks.Num()) / static_cast<float>(Chunks.Num()), 0.0f, 1.0f);
		}
		
		return 0.0f;
	}
}

bool FEasyDataTransferState::HasTimedOut(float CurrentTime) const
{
	return (CurrentTime - LastActivityTime) > Settings.TimeoutSeconds;
}

void FEasyDataTransferState::UpdateActivity(float CurrentTime)
{
	LastActivityTime = CurrentTime;
}

bool FEasyDataTransferState::CanSendNextChunk() const
{
	// Sliding window protocol: can send if we haven't exceeded window size
	const int32 WindowSize = Settings.SlidingWindowSize > 0 ? Settings.SlidingWindowSize : 5;
	const int32 UnacknowledgedChunks = NextChunkToSend - AcknowledgedChunks.Num();
	
	return UnacknowledgedChunks < WindowSize && NextChunkToSend < Chunks.Num();
}

bool FEasyDataTransferState::AreAllChunksReceived() const
{
	if (!bIsReceiver)
	{
		return false; // Only valid for receiver-side state
	}
	
	// Check if we have all chunks from 0 to TotalChunks-1
	if (ReceivedChunks.Num() == 0)
	{
		return false;
	}
	
	// Find the total number of chunks from the first received chunk
	int32 TotalChunks = 0;
	for (const auto& ChunkPair : ReceivedChunks)
	{
		TotalChunks = FMath::Max(TotalChunks, ChunkPair.Value.TotalChunks);
	}
	
	// Verify we have all chunks
	for (int32 i = 0; i < TotalChunks; ++i)
	{
		if (!ReceivedChunks.Contains(i))
		{
			return false;
		}
	}
	
	return true;
}

bool FEasyDataTransferState::ReassembleAndDecompressData()
{
	if (!bIsReceiver || !AreAllChunksReceived())
	{
		UE_LOG(LogTemp, Warning, TEXT("ReassembleAndDecompressData: Invalid state - IsReceiver:%d, AllChunksReceived:%d"), 
		       bIsReceiver, AreAllChunksReceived());
		return false;
	}
	
	// Find total chunks
	int32 TotalChunks = 0;
	for (const auto& ChunkPair : ReceivedChunks)
	{
		const FEasyDataChunk& Chunk = ChunkPair.Value;
		TotalChunks = FMath::Max(TotalChunks, Chunk.TotalChunks);
	}
	
	if (TotalChunks == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ReassembleAndDecompressData: No chunks found"));
		return false;
	}
	
	// Reassemble data in order
	TArray<uint8> ReassembledChunks;
	int32 TotalReassembledSize = 0;
	
	for (int32 i = 0; i < TotalChunks; ++i)
	{
		if (const FEasyDataChunk* Chunk = ReceivedChunks.Find(i))
		{
			ReassembledChunks.Append(Chunk->Data);
			TotalReassembledSize += Chunk->Data.Num();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ReassembleAndDecompressData: Missing chunk %d"), i);
			return false;
		}
	}
	
	// Validate reassembled size if we have expected size information
	if (bExpectedCompressed && ExpectedOriginalSize > 0)
	{
		// For compressed data, we can't easily validate size before decompression
		// But we can check if it's reasonable
		if (TotalReassembledSize > ExpectedOriginalSize * 10) // Allow up to 10x size for compressed data
		{
			UE_LOG(LogTemp, Warning, TEXT("ReassembleAndDecompressData: Reassembled size %d seems too large for expected size %d"), 
			       TotalReassembledSize, ExpectedOriginalSize);
		}
	}
	
	// Decompress if the data was compressed by sender
	if (bExpectedCompressed && ExpectedOriginalSize > 0)
	{
		TArray<uint8> DecompressedData;
		bool bDecompressionSuccessful = FEasyDataTransferCompression::DecompressData(
			ReassembledChunks, 
			DecompressedData, 
			ExpectedOriginalSize
		);
		
		if (bDecompressionSuccessful)
		{
			// Validate decompressed size
			if (DecompressedData.Num() == ExpectedOriginalSize)
			{
				ReassembledData = DecompressedData;
				UE_LOG(LogTemp, Log, TEXT("ReassembleAndDecompressData: Successfully decompressed %d bytes to %d bytes"), 
				       ReassembledChunks.Num(), DecompressedData.Num());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ReassembleAndDecompressData: Decompressed size %d doesn't match expected %d"), 
				       DecompressedData.Num(), ExpectedOriginalSize);
				ReassembledData = DecompressedData; // Use anyway, but log warning
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ReassembleAndDecompressData: Decompression failed, using compressed data as-is"));
			ReassembledData = ReassembledChunks;
		}
	}
	else
	{
		// Data was not compressed
		ReassembledData = ReassembledChunks;
		UE_LOG(LogTemp, Log, TEXT("ReassembleAndDecompressData: Data was not compressed, using as-is (%d bytes)"), 
		       ReassembledChunks.Num());
	}
	
	return true;
}

std::atomic<int32> FEasyDataTransferHandleGenerator::Counter{1};

int32 FEasyDataTransferHandleGenerator::GenerateHandle()
{
	int32 Handle = Counter.fetch_add(1);
	
	// Handle overflow by wrapping (skip 0 as it's invalid)
	if (Handle == 0)
	{
		Counter.store(1);
		Handle = 1;
	}
	
	return Handle;
}

bool FEasyDataTransferHandleGenerator::IsValidHandle(int32 Handle)
{
	return Handle != 0;
}

bool FEasyDataTransferCompression::CompressData(const TArray<uint8>& Input, TArray<uint8>& Output)
{
	if (Input.Num() == 0)
	{
		Output.Empty();
		return false;
	}
	
	// Estimate compressed buffer size
	const int32 EstimatedCompressedSize = FCompression::CompressMemoryBound(NAME_Zlib, Input.Num());
	Output.SetNum(EstimatedCompressedSize);
	
	int32 CompressedSize = EstimatedCompressedSize;
	
	// The compression level is handled internally by the Zlib format
	const bool bSuccess = FCompression::CompressMemory(
		NAME_Zlib,
		Output.GetData(),
		CompressedSize,
		Input.GetData(),
		Input.Num(),
		ECompressionFlags::COMPRESS_BiasMemory
	);
	
	if (bSuccess)
	{
		// Check if compression actually reduced the size
		if (CompressedSize < Input.Num())
		{
			// Use centralized validation for compression parameters
			if (!FEasyDataTransferValidation::ValidateCompressionParameters(Input, CompressedSize, TEXT("CompressData")))
			{
				Output = Input;
				return false;
			}
			
			// Compression was successful and beneficial
			Output.SetNum(CompressedSize);
			const float CompressionRatio = static_cast<float>(Input.Num()) / static_cast<float>(CompressedSize);
			UE_LOG(LogTemp, Verbose, TEXT("%hs: Successfully compressed %d bytes to %d bytes (ratio: %.2f)"),
				__FUNCTION__, Input.Num(), CompressedSize, CompressionRatio);
			return true;
		}
		else
		{
			// Compression succeeded but didn't reduce size
			UE_LOG(LogTemp, Verbose, TEXT("%hs: Compression succeeded but didn't reduce size (%d -> %d), using uncompressed"),
				__FUNCTION__, Input.Num(), CompressedSize);
			Output = Input;
			return false;
		}
	}
	else
	{
		// Compression failed
		UE_LOG(LogTemp, Warning, TEXT("%hs: Compression failed for %d bytes"), __FUNCTION__, Input.Num());
		Output = Input;
		return false;
	}
}

bool FEasyDataTransferCompression::DecompressData(const TArray<uint8>& Input, TArray<uint8>& Output, int32 UncompressedSize)
{
	// Use centralized validation for decompression parameters
	if (!FEasyDataTransferValidation::ValidateDecompressionParameters(Input, UncompressedSize, TEXT("DecompressData")))
	{
		Output.Empty();
		return false;
	}
	
	Output.SetNum(UncompressedSize);
	
	return FCompression::UncompressMemory(
		NAME_Zlib,
		Output.GetData(),
		UncompressedSize,
		Input.GetData(),
		Input.Num()
	);
}

int32 FEasyDataTransferCompression::CalculateChecksum(const TArray<uint8>& Data)
{
	if (Data.Num() == 0)
	{
		return 0;
	}
	
	return FCrc::MemCrc32(Data.GetData(), Data.Num());
}

void FEasyDataCompressionTask::DoWork()
{
	// Check for early cancellation
	if (bCancelled.load())
	{
		bSuccess = false;
		return;
	}
	
	if (bCompress)
	{
		// Perform compression
		bSuccess = FEasyDataTransferCompression::CompressData(InputData, OutputData);
		
		// Check if compression was beneficial (output smaller than input)
		bBeneficial = bSuccess && (OutputData.Num() < InputData.Num());
		
		// If compression wasn't beneficial, use original data
		if (bSuccess && !bBeneficial)
		{
			OutputData = InputData;
		}
	}
	else
	{
		// Perform decompression
		bSuccess = FEasyDataTransferCompression::DecompressData(InputData, OutputData, UncompressedSize);
		bBeneficial = true; // Decompression is always considered beneficial if successful
	}
	
	// Final cancellation check
	if (bCancelled.load())
	{
		bSuccess = false;
		OutputData.Empty();
	}
}