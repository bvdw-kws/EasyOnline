#include "Gauntlet/EasyQuickJoinGauntletController.h"
#include "TestStates/EasyQuickHostState.h"
#include "TestStates/EasyQuickJoinState.h"
#include "TestStates/EasyWaitForHostValidationState.h"
#include "TestStates/EasyWaitForClientsState.h"
#include "TestStates/EasyDelayState.h"
#include "EasyGauntletTest.h"

UEasyQuickJoinGauntletController::UEasyQuickJoinGauntletController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UEasyQuickJoinGauntletController::OnInit()
{
	Super::OnInit();
	
	ParseEasyQuickJoinParameters();
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("=== EasyQuickJoinGauntletController Initialized ==="));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Host Timeout: %.1f seconds"), HostTimeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Join Timeout: %.1f seconds"), JoinTimeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Max Join Retries: %d"), MaxJoinRetries);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Join Retry Interval: %.1f seconds"), JoinRetryInterval);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Validation Timeout: %.1f seconds"), ValidationTimeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Expected Client Count: %d"), ExpectedClientCount);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Test Map: %s"), TestMap.IsEmpty() ? TEXT("Default") : *TestMap);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Extended Validation: %s"), bExtendedValidation ? TEXT("Enabled") : TEXT("Disabled"));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Client Mode: %s"), IsHost() ? TEXT("Host") : TEXT("Client"));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("==================================================="));
}

void UEasyQuickJoinGauntletController::ParseEasyQuickJoinParameters()
{
	// Parse host timeout
	if (float InHostTimeout; FParse::Value(FCommandLine::Get(), TEXT("easy.hostTimeout="), InHostTimeout))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Host timeout set to: %.1f seconds"), InHostTimeout);
		HostTimeout = InHostTimeout;
	}
	
	// Parse join timeout
	if (float InJoinTimeout; FParse::Value(FCommandLine::Get(), TEXT("easy.joinTimeout="), InJoinTimeout))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Join timeout set to: %.1f seconds"), InJoinTimeout);
		JoinTimeout = InJoinTimeout;
	}
	
	// Parse max join retries
	if (int32 InMaxRetries; FParse::Value(FCommandLine::Get(), TEXT("easy.maxRetries="), InMaxRetries))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Max join retries set to: %d"), InMaxRetries);
		MaxJoinRetries = InMaxRetries;
	}
	
	// Parse join retry interval
	if (float InRetryInterval; FParse::Value(FCommandLine::Get(), TEXT("easy.retryInterval="), InRetryInterval))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Join retry interval set to: %.1f seconds"), InRetryInterval);
		JoinRetryInterval = InRetryInterval;
	}
	
	// Parse validation timeout
	if (float InValidationTimeout; FParse::Value(FCommandLine::Get(), TEXT("easy.validationTimeout="), InValidationTimeout))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Validation timeout set to: %.1f seconds"), InValidationTimeout);
		ValidationTimeout = InValidationTimeout;
	}
	
	// Parse test map
	if (FString InTestMap; FParse::Value(FCommandLine::Get(), TEXT("easy.testMap="), InTestMap) && InTestMap.Len() > 0)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Test map override: %s"), *InTestMap);
		TestMap = InTestMap;
	}
	
	// Parse extended validation flag
	if (bool bInExtendedValidation; FParse::Bool(FCommandLine::Get(), TEXT("easy.extendedValidation="), bInExtendedValidation))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Extended validation: %s"), bInExtendedValidation ? TEXT("Enabled") : TEXT("Disabled"));
		bExtendedValidation = bInExtendedValidation;
	}
	
	// Parse expected client count
	if (int32 InExpectedClientCount; FParse::Value(FCommandLine::Get(), TEXT("easy.expectedClients="), InExpectedClientCount))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Expected client count set to: %d"), InExpectedClientCount);
		ExpectedClientCount = InExpectedClientCount;
	}
}

void UEasyQuickJoinGauntletController::OnSetupTestStates(TQueue<TUniquePtr<FEasyGauntletState>>& OutStates)
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("=== Setting up QuickJoin Test States ==="));
	
	if (IsHost())
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Configuring HOST test sequence:"));
		UE_LOG(LogEasyGauntletTest, Display, TEXT("1. QuickHost (%.1fs timeout)"), HostTimeout);
		UE_LOG(LogEasyGauntletTest, Display, TEXT("2. Brief setup delay (2s)"));
		UE_LOG(LogEasyGauntletTest, Display, TEXT("3. Wait for %d client(s) (%.1fs timeout)"), ExpectedClientCount, JoinTimeout);
		UE_LOG(LogEasyGauntletTest, Display, TEXT("4. Keep connection alive (%.1fs)"), ValidationTimeout);
		
		// Phase 1: Host creates session using QuickHost
		OutStates.Enqueue(MakeUnique<FEasyQuickHostState>(this, HostTimeout, TestMap));
		
		// Phase 2: Brief setup delay to allow session to be ready
		OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, 2.0f)); // Brief setup delay
		
		// Phase 3: Wait for actual client connections - this handles validation
		OutStates.Enqueue(MakeUnique<FEasyWaitForClientsState>(this, ExpectedClientCount, JoinTimeout));
		
		// Phase 4: Keep connection alive for client validation
		OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, ValidationTimeout)); // Allow client to complete validation
		
		// Phase 5: Extended stability test if requested
		if (bExtendedValidation)
		{
			UE_LOG(LogEasyGauntletTest, Display, TEXT("5. Extended stability test (additional 10s)"));
			OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, 10.0f)); // Additional stability test
		}
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Configuring CLIENT test sequence:"));
		UE_LOG(LogEasyGauntletTest, Display, TEXT("1. Wait for host setup (10s)"));
		UE_LOG(LogEasyGauntletTest, Display, TEXT("2. QuickJoin with retries (%.1fs timeout, %d max retries, %.1fs interval)"), 
			JoinTimeout, MaxJoinRetries, JoinRetryInterval);
		UE_LOG(LogEasyGauntletTest, Display, TEXT("3. Validate connection to host (%.1fs timeout)"), ValidationTimeout);
		UE_LOG(LogEasyGauntletTest, Display, TEXT("4. Connection stability test (2s)"));
		
		// Phase 1: Wait for host to be ready
		OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, 10.0f)); // Give host time to start
		
		// Phase 2: Attempt to join using QuickJoin with retry logic
		OutStates.Enqueue(MakeUnique<FEasyQuickJoinState>(this, JoinTimeout, MaxJoinRetries, JoinRetryInterval));
		
		// Phase 3: Validate successful connection to host
		OutStates.Enqueue(MakeUnique<FEasyWaitForHostValidationState>(this, ValidationTimeout));
		
		// Phase 4: Brief connection stability test
		OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, 2.0f)); // Brief stability test
		
		// Phase 5: Extended validation if requested
		if (bExtendedValidation)
		{
			UE_LOG(LogEasyGauntletTest, Display, TEXT("5. Extended validation (additional %.1fs)"), ValidationTimeout);
			OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, 10.0f)); // Additional stability test
			OutStates.Enqueue(MakeUnique<FEasyWaitForHostValidationState>(this, ValidationTimeout));
		}
	}
	
	// Final cleanup delay
	OutStates.Enqueue(MakeUnique<FEasyDelayState>(this, 3.0f));
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("=== Test States Setup Complete ==="));
}