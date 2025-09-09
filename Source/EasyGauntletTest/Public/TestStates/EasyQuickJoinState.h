#pragma once

#include "TestStates/EasyGauntletState.h"

class UEasyGauntletController;

/**
 * Test state that uses EasyOnline's QuickJoin functionality with retry logic
 * This tests the same code path that players use when clicking "Quick Join" in the UI
 * Will retry QuickJoin attempts until successful or timeout
 */
class EASYGAUNTLETTEST_API FEasyQuickJoinState : public FEasyGauntletState
{
public:
	FEasyQuickJoinState(UEasyGauntletController* InController, float InTimeout = 120.0f, int32 InMaxRetries = 10, float InRetryInterval = 5.0f);
	virtual ~FEasyQuickJoinState() = default;

	// FEasyGauntletState interface
	virtual void OnEnterState() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnExitState() override;
	virtual FString GetStateName() const override { return TEXT("EasyQuickJoinState"); }

private:
	/** Total timeout for all join attempts */
	float Timeout;
	
	/** Time elapsed since entering this state */
	float ElapsedTime;
	
	/** Maximum number of retry attempts */
	int32 MaxRetries;
	
	/** Current retry attempt number */
	int32 CurrentRetry;
	
	/** Time between retry attempts */
	float RetryInterval;
	
	/** Time since last retry attempt */
	float TimeSinceLastAttempt;
	
	/** Whether we're currently waiting for a join attempt to complete */
	bool bWaitingForJoinResult;
	
	/** Whether we successfully joined a session */
	bool bJoinSuccessful;
	
	/** Attempts to join via QuickJoin */
	void AttemptQuickJoin();
	
	/** Checks if we've successfully joined a session */
	bool IsJoinedToSession() const;
	
	/** Checks if we can attempt another retry */
	bool CanRetry() const;
	
	/** Logs current session search/join state */
	void LogSessionState() const;
};