// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "DataTransfer/EasyDataTransferTypes.h"

/**
 * Centralized validation utilities for Easy Data Transfer system.
 * Consolidates validation logic to prevent duplication and ensure consistency.
 */
class EASYDATATRANSFERMODULE_API FEasyDataTransferValidation
{
public:
	// Constants for validation limits
	static constexpr int32 MaxChunksPerTransfer = 10000;
	static constexpr int32 MaxChunkSize = 64 * 1024; // 64KB
	static constexpr int32 MaxTransferSize = 100 * 1024 * 1024; // 100MB
	static constexpr float MinChunkSendInterval = 0.001f; // 1ms
	static constexpr float MaxExpansionRatio = 1000.0f;
	static constexpr float MinCompressionRatio = 0.1f;

	/**
	 * Validate a data chunk comprehensively.
	 * @param Chunk The chunk to validate
	 * @param Context Optional context for logging
	 * @return True if chunk is valid
	 */
	static bool ValidateChunk(const FEasyDataChunk& Chunk, const TCHAR* Context = TEXT(""));

	/**
	 * Validate transfer parameters.
	 * @param Handle Transfer handle
	 * @param TotalChunks Total number of chunks
	 * @param TransferSize Total transfer size
	 * @param Context Optional context for logging
	 * @return True if parameters are valid
	 */
	static bool ValidateTransferParameters(int32 Handle, int32 TotalChunks, int32 TransferSize, const TCHAR* Context = TEXT(""));

	/**
	 * Validate compression parameters.
	 * @param Input Input data
	 * @param CompressedSize Compressed size
	 * @param Context Optional context for logging
	 * @return True if compression parameters are valid
	 */
	static bool ValidateCompressionParameters(const TArray<uint8>& Input, int32 CompressedSize, const TCHAR* Context = TEXT(""));

	/**
	 * Validate decompression parameters.
	 * @param CompressedData Compressed data
	 * @param UncompressedSize Expected uncompressed size
	 * @param Context Optional context for logging
	 * @return True if decompression parameters are valid
	 */
	static bool ValidateDecompressionParameters(const TArray<uint8>& CompressedData, int32 UncompressedSize, const TCHAR* Context = TEXT(""));
};