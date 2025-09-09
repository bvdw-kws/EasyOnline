// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Misc/AutomationTest.h"
#include "DataTransfer/EasyDataTransferTypes.h"
#include "DataTransfer/Settings/EasyDataTransferSettings.h"
#include "DataTransfer/Utils/EasyDataTransferChunkProcessor.h"
#include "DataTransfer/Utils/EasyDataTransferStateManager.h"
#include "DataTransfer/Utils/EasyDataTransferValidation.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

/**
 * EasyDataTransfer Unit Tests
 * 
 * This file contains pure unit tests that test individual validation components
 * using the actual public APIs. Note: Chunk processor and state manager tests
 * require subsystem integration and are moved to integration tests.
 */

// NOTE: Chunk processor and state manager tests require subsystem integration
// and have been moved to integration tests. This file focuses on validation tests
// that can run independently.

// ============================================================================
// CATEGORY 1: VALIDATION TESTS
// Tests input validation and security checks
// ============================================================================

/**
 * Validation Test 1: Chunk Validation
 * 
 * PURPOSE: Tests chunk validation logic
 * GOAL: Verify malformed chunks are properly rejected
 * VALIDATION: All validation rules, edge cases
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEasyDataTransferValidationChunkTest, 
    "EasyOnline.DataTransfer.Unit.Validation.Chunk", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEasyDataTransferValidationChunkTest::RunTest(const FString& Parameters)
{
    // Test valid chunk
    FEasyDataChunk ValidChunk;
    ValidChunk.TransferHandle = 123;
    ValidChunk.ChunkIndex = 2;
    ValidChunk.TotalChunks = 5;
    ValidChunk.Data = {1, 2, 3, 4, 5};
    ValidChunk.Checksum = FEasyDataTransferCompression::CalculateChecksum(ValidChunk.Data);
    
    TestTrue(TEXT("Valid chunk should pass validation"), 
        FEasyDataTransferValidation::ValidateChunk(ValidChunk));
    
    // Test invalid handle
    FEasyDataChunk InvalidHandle = ValidChunk;
    InvalidHandle.TransferHandle = 0;
    TestFalse(TEXT("Chunk with handle 0 should fail"), 
        FEasyDataTransferValidation::ValidateChunk(InvalidHandle));
    
    // Test invalid index
    FEasyDataChunk InvalidIndex = ValidChunk;
    InvalidIndex.ChunkIndex = -1;
    TestFalse(TEXT("Chunk with negative index should fail"), 
        FEasyDataTransferValidation::ValidateChunk(InvalidIndex));
    
    InvalidIndex.ChunkIndex = 10;
    InvalidIndex.TotalChunks = 5;
    TestFalse(TEXT("Chunk with index >= total should fail"), 
        FEasyDataTransferValidation::ValidateChunk(InvalidIndex));
    
    // Test invalid total chunks
    FEasyDataChunk InvalidTotal = ValidChunk;
    InvalidTotal.TotalChunks = 0;
    TestFalse(TEXT("Chunk with 0 total should fail"), 
        FEasyDataTransferValidation::ValidateChunk(InvalidTotal));
    
    // Test empty data
    FEasyDataChunk EmptyData = ValidChunk;
    EmptyData.Data.Empty();
    TestFalse(TEXT("Chunk with empty data should fail"), 
        FEasyDataTransferValidation::ValidateChunk(EmptyData));
    
    return true;
}

/**
 * Validation Test 2: Transfer Parameters Validation
 * 
 * PURPOSE: Tests transfer parameter validation
 * GOAL: Verify invalid transfer parameters are rejected
 * VALIDATION: Handle, chunk count, size limits
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEasyDataTransferValidationParametersTest, 
    "EasyOnline.DataTransfer.Unit.Validation.Parameters", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEasyDataTransferValidationParametersTest::RunTest(const FString& Parameters)
{
    // Test valid parameters
    bool bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, 10, 10000, TEXT("ValidTest"));
    TestTrue(TEXT("Valid parameters should pass"), bValid);
    
    // Note: ValidateTransferParameters does NOT validate the handle parameter
    // It only validates chunks and transfer size
    
    // Test with handle 0 (should pass - handle not validated in this function)
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        0, 10, 10000, TEXT("HandleZero"));
    TestTrue(TEXT("Handle 0 should pass (not validated by this function)"), bValid);
    
    // Test with negative handle (should pass - handle not validated in this function)
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        -1, 10, 10000, TEXT("NegativeHandle"));
    TestTrue(TEXT("Negative handle should pass (not validated by this function)"), bValid);
    
    // Test zero chunks
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, 0, 10000, TEXT("ZeroChunks"));
    TestFalse(TEXT("Zero chunks should fail"), bValid);
    
    // Test negative chunks
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, -5, 10000, TEXT("NegativeChunks"));
    TestFalse(TEXT("Negative chunks should fail"), bValid);
    
    // Test zero size (should pass - zero is valid)
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, 10, 0, TEXT("ZeroSize"));
    TestTrue(TEXT("Zero size should pass"), bValid);
    
    // Test negative size
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, 10, -1000, TEXT("NegativeSize"));
    TestFalse(TEXT("Negative size should fail"), bValid);
    
    // Test oversized transfer (using validation constant)
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, 10, FEasyDataTransferValidation::MaxTransferSize + 1, TEXT("Oversized"));
    TestFalse(TEXT("Oversized transfer should fail"), bValid);
    
    // Test too many chunks (using validation constant)
    bValid = FEasyDataTransferValidation::ValidateTransferParameters(
        12345, FEasyDataTransferValidation::MaxChunksPerTransfer + 1, 10000, TEXT("TooManyChunks"));
    TestFalse(TEXT("Too many chunks should fail"), bValid);
    
    return true;
}

/**
 * Validation Test 3: Compression Parameters Validation
 * 
 * PURPOSE: Tests compression parameter validation
 * GOAL: Verify compression validation logic
 * VALIDATION: Input data, compression ratios, size limits
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEasyDataTransferValidationCompressionTest, 
    "EasyOnline.DataTransfer.Unit.Validation.Compression", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEasyDataTransferValidationCompressionTest::RunTest(const FString& Parameters)
{
    // Create test data
    TArray<uint8> TestData;
    for (int32 i = 0; i < 1000; ++i)
    {
        TestData.Add(static_cast<uint8>(i % 256));
    }
    
    // Test valid compression (reasonable compression ratio)
    bool bValid = FEasyDataTransferValidation::ValidateCompressionParameters(
        TestData, 800, TEXT("ValidCompression"));
    TestTrue(TEXT("Valid compression should pass"), bValid);
    
    // Test empty input data
    TArray<uint8> EmptyData;
    bValid = FEasyDataTransferValidation::ValidateCompressionParameters(
        EmptyData, 100, TEXT("EmptyInput"));
    TestFalse(TEXT("Empty input should fail"), bValid);
    
    // Test negative compressed size (should pass - validation allows <= 0)
    bValid = FEasyDataTransferValidation::ValidateCompressionParameters(
        TestData, -100, TEXT("NegativeSize"));
    TestTrue(TEXT("Negative compressed size should pass"), bValid);
    
    // Test zero compressed size (should pass - validation allows <= 0)
    bValid = FEasyDataTransferValidation::ValidateCompressionParameters(
        TestData, 0, TEXT("ZeroSize"));
    TestTrue(TEXT("Zero compressed size should pass"), bValid);
    
    // Test very high compression ratio that would be suspicious
    // MinCompressionRatio is 0.1, so input/compressed must be >= 0.1
    // If compressed = input * 20, ratio = input/(input*20) = 0.05 < 0.1 (should fail)
    bValid = FEasyDataTransferValidation::ValidateCompressionParameters(
        TestData, TestData.Num() * 20, TEXT("SuspiciousCompression"));
    TestFalse(TEXT("Suspicious compression ratio should fail"), bValid);
    
    return true;
}

/**
 * Validation Test 4: Decompression Parameters Validation
 * 
 * PURPOSE: Tests decompression parameter validation
 * GOAL: Verify decompression validation logic
 * VALIDATION: Expansion ratios, size limits, data integrity
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEasyDataTransferValidationDecompressionTest, 
    "EasyOnline.DataTransfer.Unit.Validation.Decompression", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEasyDataTransferValidationDecompressionTest::RunTest(const FString& Parameters)
{
    // Create test compressed data
    TArray<uint8> CompressedData = {1, 2, 3, 4, 5};
    
    // Test valid decompression
    bool bValid = FEasyDataTransferValidation::ValidateDecompressionParameters(
        CompressedData, 50, TEXT("ValidDecompression"));
    TestTrue(TEXT("Valid decompression should pass"), bValid);
    
    // Test empty compressed data
    TArray<uint8> EmptyData;
    bValid = FEasyDataTransferValidation::ValidateDecompressionParameters(
        EmptyData, 100, TEXT("EmptyCompressed"));
    TestFalse(TEXT("Empty compressed data should fail"), bValid);
    
    // Test zero uncompressed size
    bValid = FEasyDataTransferValidation::ValidateDecompressionParameters(
        CompressedData, 0, TEXT("ZeroUncompressed"));
    TestFalse(TEXT("Zero uncompressed size should fail"), bValid);
    
    // Test suspicious expansion ratio (> MaxExpansionRatio which is 1000.0f)
    // If compressed is 5 bytes and uncompressed is 6000 bytes, ratio = 6000/5 = 1200 > 1000
    bValid = FEasyDataTransferValidation::ValidateDecompressionParameters(
        CompressedData, 6000, TEXT("SuspiciousExpansion"));
    TestFalse(TEXT("Suspicious expansion ratio should fail"), bValid);
    
    // Test oversized uncompressed data (> MaxTransferSize)
    bValid = FEasyDataTransferValidation::ValidateDecompressionParameters(
        CompressedData, FEasyDataTransferValidation::MaxTransferSize + 1, TEXT("Oversized"));
    TestFalse(TEXT("Oversized uncompressed data should fail"), bValid);
    
    return true;
}