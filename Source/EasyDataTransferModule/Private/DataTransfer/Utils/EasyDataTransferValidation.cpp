// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "DataTransfer/Utils/EasyDataTransferValidation.h"

DEFINE_LOG_CATEGORY_STATIC(LogEasyDataTransferValidation, Log, All);

bool FEasyDataTransferValidation::ValidateChunk(const FEasyDataChunk& Chunk, const TCHAR* Context)
{
	// Validate transfer handle
	if (!FEasyDataTransferHandleGenerator::IsValidHandle(Chunk.TransferHandle))
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Invalid transfer handle %d"), Context, Chunk.TransferHandle);
		return false;
	}

	// Validate chunk indices
	if (Chunk.ChunkIndex < 0 || Chunk.TotalChunks <= 0 || Chunk.ChunkIndex >= Chunk.TotalChunks)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Invalid chunk indices - Handle:%d, Index:%d, Total:%d"),
			Context, Chunk.TransferHandle, Chunk.ChunkIndex, Chunk.TotalChunks);
		return false;
	}

	// Prevent unreasonable chunk counts (DoS protection)
	if (Chunk.TotalChunks > MaxChunksPerTransfer)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Too many chunks %d for handle %d (max %d)"),
			Context, Chunk.TotalChunks, Chunk.TransferHandle, MaxChunksPerTransfer);
		return false;
	}

	// Validate data size (prevent memory bombs)
	if (Chunk.Data.Num() < 0)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Negative data size %d for handle %d"),
			Context, Chunk.Data.Num(), Chunk.TransferHandle);
		return false;
	}

	if (Chunk.Data.Num() > MaxChunkSize)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Data size %d exceeds maximum %d for handle %d"),
			Context, Chunk.Data.Num(), MaxChunkSize, Chunk.TransferHandle);
		return false;
	}

	// Validate checksum for data integrity
	const int32 CalculatedChecksum = FEasyDataTransferCompression::CalculateChecksum(Chunk.Data);
	if (CalculatedChecksum != Chunk.Checksum)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Checksum mismatch for handle %d - Expected:%d, Calculated:%d"),
			Context, Chunk.TransferHandle, Chunk.Checksum, CalculatedChecksum);
		return false;
	}

	// Validate empty data checksum consistency
	if (Chunk.Data.Num() == 0 && Chunk.Checksum != 0)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Empty data with non-zero checksum %d for handle %d"),
			Context, Chunk.Checksum, Chunk.TransferHandle);
		return false;
	}

	return true;
}

bool FEasyDataTransferValidation::ValidateTransferParameters(int32 Handle, int32 TotalChunks, int32 TransferSize, const TCHAR* Context)
{
	if (TotalChunks <= 0 || TotalChunks > MaxChunksPerTransfer)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Invalid total chunks %d for handle %u (max %d)"),
			Context, TotalChunks, Handle, MaxChunksPerTransfer);
		return false;
	}

	if (TransferSize < 0 || TransferSize > MaxTransferSize)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Invalid transfer size %d for handle %u (max %d)"),
			Context, TransferSize, Handle, MaxTransferSize);
		return false;
	}

	return true;
}

bool FEasyDataTransferValidation::ValidateCompressionParameters(const TArray<uint8>& Input, int32 CompressedSize, const TCHAR* Context)
{
	if (Input.Num() <= 0)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Invalid input size %d for compression"), Context, Input.Num());
		return false;
	}

	if (CompressedSize > 0)
	{
		// Check compression ratio for suspicious data
		const float CompressionRatio = static_cast<float>(Input.Num()) / static_cast<float>(CompressedSize);
		if (CompressionRatio < MinCompressionRatio)
		{
			UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Suspicious compression ratio %.2f, rejecting"), Context, CompressionRatio);
			return false;
		}
	}

	return true;
}

bool FEasyDataTransferValidation::ValidateDecompressionParameters(const TArray<uint8>& CompressedData, int32 UncompressedSize, const TCHAR* Context)
{
	if (CompressedData.Num() == 0 || UncompressedSize <= 0)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Invalid decompression parameters - Compressed:%d, Uncompressed:%d"),
			Context, CompressedData.Num(), UncompressedSize);
		return false;
	}

	// Compression bomb protection: reject unreasonable expansion ratios
	const float ExpansionRatio = static_cast<float>(UncompressedSize) / static_cast<float>(CompressedData.Num());
	if (ExpansionRatio > MaxExpansionRatio)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Suspicious expansion ratio %.2f (compressed:%d -> uncompressed:%d), rejecting"),
			Context, ExpansionRatio, CompressedData.Num(), UncompressedSize);
		return false;
	}

	// Additional safety check: limit absolute uncompressed size
	if (UncompressedSize > MaxTransferSize)
	{
		UE_LOG(LogEasyDataTransferValidation, Warning, TEXT("%s: Uncompressed size %d exceeds maximum %d"),
			Context, UncompressedSize, MaxTransferSize);
		return false;
	}

	return true;
}
