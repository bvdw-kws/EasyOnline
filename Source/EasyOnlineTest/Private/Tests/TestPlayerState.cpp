// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

/**
 * EasyDataTransfer Integration Tests
 * 
 * These tests verify the end-to-end functionality of the EasyDataTransfer system,
 * including multiplayer scenarios, data integrity, compression, and error handling.
 * 
 * Test Categories:
 * 1. Basic Transfer Tests - Simple data transfers between mock clients
 * 2. Compression Tests - Verify data compression/decompression integrity  
 * 3. Large Data Tests - Multi-chunk transfers with progress tracking
 * 4. Error Handling Tests - Network failures, timeouts, cancellations
 * 5. Concurrent Transfer Tests - Multiple simultaneous transfers
 * 6. Performance Tests - Bandwidth usage, memory consumption
 * 7. Multiplayer Simulation Tests - Multi-client scenarios
 */


#include "Tests/TestPlayerState.h"
#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"

ATestPlayerState::ATestPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DataTransferComponent = ObjectInitializer.CreateDefaultSubobject<UEasyDataTransferPlayerComponent>(this, TEXT("DataTransferComponent"));
}
