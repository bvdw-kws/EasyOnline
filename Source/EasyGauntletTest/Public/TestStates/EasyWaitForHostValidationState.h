#pragma once

#include "CoreMinimal.h"
#include "TestStates/EasyGauntletState.h"
#include "Interfaces/OnlineSessionInterface.h"

class UEasyGauntletController;

/**
 * Test state that validates successful connection to host from client perspective
 * Verifies that client is properly connected to host
 */
class EASYGAUNTLETTEST_API FEasyWaitForHostValidationState : public FEasyGauntletState
{
public:
	FEasyWaitForHostValidationState(UEasyGauntletController* InController, float InTimeout = 30.0f);
	virtual ~FEasyWaitForHostValidationState() = default;

	// FEasyGauntletState interface
	virtual void OnEnterState() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnExitState() override;
	virtual FString GetStateName() const override { return TEXT("EasyWaitForHostValidationState"); }

private:
	/** Timeout for validation */
	float Timeout;
	
	/** Time elapsed since entering this state */
	float ElapsedTime;
	
	/** Whether we've completed validation successfully */
	bool bValidationComplete;
	
	/** Validates the current multiplayer connection state */
	bool ValidateConnection() const;
	
	/** Gets the number of connected players */
	int32 GetConnectedPlayerCount() const;
	
	/** Logs detailed connection information */
	void LogConnectionState() const;
};