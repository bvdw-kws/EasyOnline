// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "Gauntlet/EasyGauntletController.h"
#include "EasyGauntletTest.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "Async/Async.h"

void UEasyGauntletController::OnInit()
{
	Super::OnInit();
	
	ParseCommandLineParameters();
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("EasyGauntletController initialized"));
}

void UEasyGauntletController::ParseCommandLineParameters()
{
	// Parse timeout
	if (int32 InTimeout; FParse::Value(FCommandLine::Get(), TEXT("easy.timeout="), InTimeout))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Global timeout set to: %d seconds"), InTimeout);
		TimeoutTime = InTimeout;
	}
	
	// Parse skip cinematic
	if (bool InSkipCinematic; FParse::Bool(FCommandLine::Get(), TEXT("easy.skipCinematic="), InSkipCinematic))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Skip cinematic: %s"), InSkipCinematic ? TEXT("Enabled") : TEXT("Disabled"));
		bSkipCinematic = InSkipCinematic;
	}
	
	// Parse profiler
	if (bool InProfiler; FParse::Bool(FCommandLine::Get(), TEXT("easy.profiler="), InProfiler))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("CSV Profiler: %s"), InProfiler ? TEXT("Enabled") : TEXT("Disabled"));
		bUseProfiler = InProfiler;
	}
	
	// Parse verbose logging
	if (bool InVerbose; FParse::Bool(FCommandLine::Get(), TEXT("easy.verbose="), InVerbose))
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Verbose logging: %s"), InVerbose ? TEXT("Enabled") : TEXT("Disabled"));
		bVerboseLogging = InVerbose;
	}
	
	// Parse map override
	if (FString InMapOverride; FParse::Value(FCommandLine::Get(), TEXT("easy.mapOverride="), InMapOverride) && InMapOverride.Len() > 0)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Map override: %s"), *InMapOverride);
		MapOverride = InMapOverride;
	}
}

void UEasyGauntletController::OnPostMapChange(UWorld* World)
{
	if (!IsValid(World))
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("EasyGauntletController failed to load map"));
		EndTest(1);
		return;
	}

	if (bTestStarted)
	{
		// Map changed during test - could be intentional state transition
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Map changed during test execution"));
	}
	else
	{
		// Handle map override if specified
		if (!MapOverride.IsEmpty())
		{
			UE_LOG(LogEasyGauntletTest, Display, TEXT("Opening override map: %s"), *MapOverride);
			UGameplayStatics::OpenLevel(World, *MapOverride);
		}
		
		bTestStarted = true;
		
		// Start testing after spin-up delay
		FTimerHandle DelayTimer;
		GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &ThisClass::StartTesting, SpinUpTime, false);
	}
}

void UEasyGauntletController::OnTick(float DeltaTime)
{
	// Update current state
	if (CurrentState.IsValid())
	{
		CurrentState->OnTick(DeltaTime);
		if (CurrentState->IsFinishState())
		{
			if (!CurrentState->HasSucceeded())
			{
				UE_LOG(LogEasyGauntletTest, Error, TEXT("State failed: %s"), *CurrentState->GetStateName());
				StopTesting(1);
				return;
			}
			MoveNextState();
		}
	}

	// Check global timeout
	const double CurrentTimer = GetTimeInCurrentState();
	if (TimeoutTime.IsSet() && CurrentTimer >= TimeoutTime.GetValue())
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("Global timeout reached: %.2f seconds"), CurrentTimer);
		StopTesting(1);
		return;
	}

#if CSV_PROFILER
	// Handle CSV profiler state transitions
	if (bUseProfiler)
	{
		if (const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
		{
			const bool bBlackScreen = FMath::IsNearlyEqual(CameraManager->FadeAmount, 1.0f);
			if (bBlackScreen != bIsProfilerWaiting)
			{
				bIsProfilerWaiting = bBlackScreen;
				if (bIsProfilerWaiting)
				{
					FCsvProfiler::Get()->BeginWait();
				}
				else
				{
					FCsvProfiler::Get()->EndWait();
				}
			}
		}
	}
#endif // CSV_PROFILER
}

void UEasyGauntletController::StartTesting()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Starting test execution"));
	
	// Setup test states
	OnSetupTestStates(States);
	
#if CSV_PROFILER
	// Start profiler if enabled
	if (bUseProfiler)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Starting CSV profiler"));
		FCsvProfiler::Get()->BeginCapture();
	}
#endif // CSV_PROFILER

	// Start state chain
	if (!States.IsEmpty())
	{
		MoveNextState();
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("No test states to run"));
		StopTesting(1);
	}
}

void UEasyGauntletController::MoveNextState()
{
	// Exit current state
	if (CurrentState.IsValid())
	{
		CurrentState->OnExitState();
	}
	
	// Move to next state
	if (States.Dequeue(CurrentState))
	{
		if (CurrentState.IsValid())
		{
			CurrentState->OnEnterState();
		}
	}
	else
	{
		// No more states - test completed successfully
		UE_LOG(LogEasyGauntletTest, Display, TEXT("All test states completed successfully"));
		StopTesting(0);
	}
}

void UEasyGauntletController::OnSetupTestStates(TQueue<TUniquePtr<FEasyGauntletState>>& OutStates)
{
	// Override in derived classes to setup test states
	UE_LOG(LogEasyGauntletTest, Warning, TEXT("OnSetupTestStates not implemented - no test states will run"));
}

void UEasyGauntletController::StopTesting(int32 ExitCode)
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("Stopping test with exit code: %d"), ExitCode);

#if CSV_PROFILER
	// Stop profiler if running
	if (bUseProfiler && FCsvProfiler::Get()->IsCapturing())
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("Stopping CSV profiler"));
		TSharedFuture<FString> Future = FCsvProfiler::Get()->EndCapture();
		
		// Wait for profiler to finish before ending test
		AsyncTask(ENamedThreads::AnyThread, [this, Future, ExitCode]()
		{
			while (!Future.IsReady())
			{
				FPlatformProcess::SleepNoStats(0);
			}

			AsyncTask(ENamedThreads::GameThread, [this, ExitCode]()
			{
				EndTest(ExitCode);
			});
		});
		return;
	}
#endif // CSV_PROFILER

	EndTest(ExitCode);
}