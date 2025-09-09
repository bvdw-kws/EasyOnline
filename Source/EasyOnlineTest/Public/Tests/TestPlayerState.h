// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

/**
 * EasyDataTransfer Comprehensive Test Suite
 * 
 * This file contains all unit and integration tests for the EasyDataTransfer system.
 * Tests are organized by functionality and thoroughly document their purpose, goals,
 * and validation criteria.
 * 
 * Test Categories:
 * 1. CORE TESTS - Basic functionality, data structures, utilities
 * 2. COMPONENT TESTS - PlayerComponent functionality and networking
 * 3. SUBSYSTEM TESTS - GameInstanceSubsystem behavior and lifecycle
 * 4. SETTINGS TESTS - Configuration validation and defaults
 * 5. INTEGRATION TESTS - End-to-end transfer scenarios
 * 6. PERFORMANCE TESTS - Memory usage, throughput, stress testing
 * 7. NETWORK SIMULATION TESTS - Multi-client scenarios and error handling
 */

#pragma once

#include "GameFramework/PlayerState.h"
#include "DataTransfer/IEasyDataTransferPlayerInterface.h"

#include "TestPlayerState.generated.h"

// Forward declare mock classes
UCLASS()
class ATestPlayerState : public APlayerState, public IEasyDataTransferPlayerInterface
{
	GENERATED_BODY()

public:
	ATestPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin IEasyDataTransferPlayerInterface Interface
	virtual UEasyDataTransferPlayerComponent* GetDataTransferComponent_Implementation() const override
	{
		return DataTransferComponent;
	}
	//~ End IEasyDataTransferPlayerInterface Interface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEasyDataTransferPlayerComponent> DataTransferComponent;
};
