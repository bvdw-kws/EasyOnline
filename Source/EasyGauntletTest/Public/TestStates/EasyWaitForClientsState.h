#pragma once

#include "CoreMinimal.h"
#include "TestStates/EasyGauntletState.h"

class UEasyOnlineGauntletController;

/**
 * Gauntlet test state that waits for a specific number of clients to connect
 * before proceeding to validation phase. This ensures validation only starts
 * after actual client connections, not just after arbitrary time delays.
 */
class EASYGAUNTLETTEST_API FEasyWaitForClientsState : public FEasyGauntletState
{
public:
	/**
	 * Constructor
	 * @param InController The online gauntlet controller
	 * @param InExpectedClientCount Number of clients to wait for (default 1)
	 * @param InTimeout Maximum time to wait for clients (typically JoinTimeout)
	 */
	FEasyWaitForClientsState(UEasyOnlineGauntletController* InController, int32 InExpectedClientCount = 1, float InTimeout = 60.0f);

	// FEasyGauntletState interface
	virtual void OnEnterState() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnExitState() override;
	virtual FString GetStateName() const override { return TEXT("WaitForClients"); }

protected:
	/** Online gauntlet controller for accessing IsHost() */
	UEasyOnlineGauntletController* OnlineController;
	
	/** Number of clients we're waiting for */
	int32 ExpectedClientCount;
	
	/** Maximum time to wait for clients */
	float Timeout;
	
	/** Time elapsed since entering this state */
	float ElapsedTime;
	
	/** Whether we've successfully detected the required client count */
	bool bClientsConnected;

	/**
	 * Check if the required number of clients have connected
	 * @return True if we have host + expected client count
	 */
	bool CheckClientCount() const;
	
	/**
	 * Get the current number of connected players
	 * @return Number of players in GameState PlayerArray
	 */
	int32 GetConnectedPlayerCount() const;
	
	/**
	 * Log current connection state for debugging
	 */
	void LogConnectionState() const;
};