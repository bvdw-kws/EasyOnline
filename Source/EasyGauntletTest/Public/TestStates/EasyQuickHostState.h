#pragma once

#include "TestStates/EasyGauntletState.h"

class UEasyGauntletController;

/**
 * Test state that uses EasyOnline's QuickHost functionality
 * This tests the same code path that players use when clicking "Quick Host" in the UI
 */
class EASYGAUNTLETTEST_API FEasyQuickHostState : public FEasyGauntletState
{
public:
	FEasyQuickHostState(UEasyGauntletController* InController, float InTimeout = 60.0f, const FString& InMapName = TEXT(""));
	virtual ~FEasyQuickHostState() = default;

	// FEasyGauntletState interface
	virtual void OnEnterState() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnExitState() override;
	virtual FString GetStateName() const override { return TEXT("EasyQuickHostState"); }

private:
	/** Timeout for the quick host operation */
	float Timeout;
	
	/** Time elapsed since entering this state */
	float ElapsedTime;
	
	/** Map to host (empty = use default from settings) */
	FString MapName;
	
	/** Whether quick host has been initiated */
	bool bQuickHostInitiated;
	
	/** Whether we're waiting for the session to be created */
	bool bWaitingForSession;
	
	/** Attempts to start quick hosting */
	void InitiateQuickHost();
	
	/** Checks if the session has been successfully created */
	bool IsSessionActive() const;
	
	/** Called when session creation completes */
	void OnSessionCreated(bool bWasSuccessful);
};