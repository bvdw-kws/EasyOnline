#include "TestStates/EasyWaitForClientsState.h"
#include "EasyGauntletTest.h"
#include "Gauntlet/EasyOnlineGauntletController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

FEasyWaitForClientsState::FEasyWaitForClientsState(UEasyOnlineGauntletController* InController, int32 InExpectedClientCount, float InTimeout)
	: FEasyGauntletState(InController)
	, OnlineController(InController)
	, ExpectedClientCount(InExpectedClientCount)
	, Timeout(InTimeout)
	, ElapsedTime(0.0f)
	, bClientsConnected(false)
{
}

void FEasyWaitForClientsState::OnEnterState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Entering EasyWaitForClientsState"));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Expected clients: %d"), ExpectedClientCount);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Timeout: %.1f seconds"), Timeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Role: %s"), OnlineController->IsHost() ? TEXT("Host") : TEXT("Client"));
	
	ElapsedTime = 0.0f;
	bClientsConnected = false;
	
	// Log initial state
	LogConnectionState();
}

void FEasyWaitForClientsState::OnTick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	
	// Check timeout
	if (ElapsedTime >= Timeout)
	{
		int32 CurrentPlayerCount = GetConnectedPlayerCount();
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[WaitForClients] Timeout reached (%.1f seconds) - Only %d/%d clients connected"), 
			Timeout, FMath::Max(0, CurrentPlayerCount - 1), ExpectedClientCount);
		LogConnectionState();
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[WaitForClients] Failed to wait for required client count"));
		FinishState(false);
		return;
	}
	
	// Check if required clients have connected
	if (!bClientsConnected && CheckClientCount())
	{
		bClientsConnected = true;
		int32 CurrentPlayerCount = GetConnectedPlayerCount();
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Success! Required clients connected: %d/%d, Elapsed time: %.1f seconds"), 
			CurrentPlayerCount - 1, ExpectedClientCount, ElapsedTime);
		LogConnectionState();
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Proceeding to validation phase"));
		FinishState(true);
		return;
	}
	
	// Log progress every 5 seconds
	if (FMath::Fmod(ElapsedTime, 5.0f) < DeltaTime)
	{
		int32 CurrentPlayerCount = GetConnectedPlayerCount();
		int32 CurrentClientCount = FMath::Max(0, CurrentPlayerCount - 1);
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Still waiting for clients... Current: %d/%d clients, Elapsed: %.1f/%.1f seconds"), 
			CurrentClientCount, ExpectedClientCount, ElapsedTime, Timeout);
		LogConnectionState();
	}
}

void FEasyWaitForClientsState::OnExitState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Exiting EasyWaitForClientsState - Final state:"));
	LogConnectionState();
}

bool FEasyWaitForClientsState::CheckClientCount() const
{
	int32 CurrentPlayerCount = GetConnectedPlayerCount();
	int32 RequiredPlayerCount = 1 + ExpectedClientCount; // Host + clients
	
	bool bHasRequiredClients = CurrentPlayerCount >= RequiredPlayerCount;
	
	UE_LOG(LogEasyGauntletTest, VeryVerbose, TEXT("[WaitForClients] CheckClientCount - Current: %d, Required: %d, Success: %s"), 
		CurrentPlayerCount, RequiredPlayerCount, bHasRequiredClients ? TEXT("Yes") : TEXT("No"));
	
	return bHasRequiredClients;
}

int32 FEasyWaitForClientsState::GetConnectedPlayerCount() const
{
	if (!Controller)
	{
		return 0;
	}
	
	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		return 0;
	}
	
	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return 0;
	}
	
	return GameState->PlayerArray.Num();
}

void FEasyWaitForClientsState::LogConnectionState() const
{
	if (!Controller)
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("[WaitForClients] No controller available for logging"));
		return;
	}
	
	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("[WaitForClients] No world available for logging"));
		return;
	}
	
	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("[WaitForClients] No GameState available"));
		return;
	}
	
	int32 PlayerCount = GameState->PlayerArray.Num();
	int32 ClientCount = FMath::Max(0, PlayerCount - 1);
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[WaitForClients] Connection State - Total Players: %d, Clients: %d/%d, Expected: %d"), 
		PlayerCount, ClientCount, ExpectedClientCount, ExpectedClientCount);
	
	// Log individual player states for debugging
	for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
	{
		if (APlayerState* PS = GameState->PlayerArray[i])
		{
			UE_LOG(LogEasyGauntletTest, VeryVerbose, TEXT("[WaitForClients] Player %d: %s"), 
				i, PS->GetPlayerName().IsEmpty() ? TEXT("(no name)") : *PS->GetPlayerName());
		}
	}
}