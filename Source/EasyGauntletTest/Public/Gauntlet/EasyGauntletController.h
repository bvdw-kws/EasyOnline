// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GauntletTestController.h"
#include "TestStates/EasyGauntletState.h"
#include "EasyGauntletController.generated.h"

UCLASS()
class EASYGAUNTLETTEST_API UEasyGauntletController : public UGauntletTestController
{
	GENERATED_BODY()

public:
	virtual void OnInit() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnPostMapChange(UWorld* World) override;
	
	void StopTesting(int32 ExitCode);

protected:
	virtual void OnSetupTestStates(TQueue<TUniquePtr<FEasyGauntletState>>& OutStates);
	
	// Configuration accessors
	FORCEINLINE bool IsSkipCinematicEnabled() const { return bSkipCinematic; }
	FORCEINLINE bool IsProfilerEnabled() const { return bUseProfiler; }
	FORCEINLINE bool IsVerboseLoggingEnabled() const { return bVerboseLogging; }

private:
	void StartTesting();
	void MoveNextState();
	void ParseCommandLineParameters();
	
	// State management
	TUniquePtr<FEasyGauntletState> CurrentState;
	TQueue<TUniquePtr<FEasyGauntletState>> States;
	
	// Test control
	const float SpinUpTime = 3.0f;
	bool bTestStarted = false;
	
	// Configuration parameters
	TOptional<float> TimeoutTime;
	bool bSkipCinematic = false;
	bool bUseProfiler = false;
	bool bVerboseLogging = false;
	FString MapOverride;

#if CSV_PROFILER
	bool bIsProfilerWaiting = false;
#endif // CSV_PROFILER
};