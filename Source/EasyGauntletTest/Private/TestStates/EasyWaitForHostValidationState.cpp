#include "TestStates/EasyWaitForHostValidationState.h"
#include "EasyGauntletTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Gauntlet/EasyGauntletController.h"

FEasyWaitForHostValidationState::FEasyWaitForHostValidationState(UEasyGauntletController* InController, float InTimeout)
	: FEasyGauntletState(InController)
	, Timeout(InTimeout)
	, ElapsedTime(0.0f)
	, bValidationComplete(false)
{
}

void FEasyWaitForHostValidationState::OnEnterState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Entering EasyWaitForHostValidationState"));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Timeout: %.1f seconds"), Timeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Role: Client"));
	
	ElapsedTime = 0.0f;
	bValidationComplete = false;
	
	// Log initial state
	LogConnectionState();
}

void FEasyWaitForHostValidationState::OnTick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	
	// Check timeout
	if (ElapsedTime >= Timeout)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[Validation] Timeout reached (%.1f seconds) - Connection validation failed"), Timeout);
		LogConnectionState();
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[Validation] Connection validation timed out"));
		FinishState(false);
		return;
	}
	
	// Check if validation is complete
	if (!bValidationComplete && ValidateConnection())
	{
		bValidationComplete = true;
		int32 PlayerCount = GetConnectedPlayerCount();
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Connection validation successful! Players connected: %d, Elapsed time: %.1f seconds"), 
			PlayerCount, ElapsedTime);
		LogConnectionState();
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Connection validated with %d players"), PlayerCount);
		FinishState(true);
		return;
	}
	
	// Log progress every 5 seconds
	if (FMath::Fmod(ElapsedTime, 5.0f) < DeltaTime)
	{
		int32 PlayerCount = GetConnectedPlayerCount();
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Still validating connection... Players: %d, Elapsed: %.1f/%.1f seconds"), 
			PlayerCount, ElapsedTime, Timeout);
		LogConnectionState();
	}
}

void FEasyWaitForHostValidationState::OnExitState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Exiting EasyWaitForHostValidationState - Final state:"));
	LogConnectionState();
}

bool FEasyWaitForHostValidationState::ValidateConnection() const
{
	if (!Controller)
	{
		return false;
	}
	
	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		return false;
	}
	
	// Client validation: Should be connected to host and have valid net connection
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		return false;
	}
	
	// Check if we have a valid net connection to the server
	bool bHasNetConnection = PC->GetNetConnection() != nullptr;
	
	// Check if we have a valid player state (indicates proper connection)
	bool bHasPlayerState = PC->GetPlayerState<APlayerState>() != nullptr;
	
	// Check GameState for connected players
	int32 PlayerCount = GetConnectedPlayerCount();
	bool bValidPlayerCount = PlayerCount >= 2;
	
	bool bIsValidClient = bHasNetConnection && bHasPlayerState && bValidPlayerCount;
	
	UE_LOG(LogEasyGauntletTest, VeryVerbose, TEXT("[Validation] Client validation - NetConnection: %s, PlayerState: %s, PlayerCount: %d, Valid: %s"), 
		bHasNetConnection ? TEXT("Yes") : TEXT("No"),
		bHasPlayerState ? TEXT("Yes") : TEXT("No"),
		PlayerCount,
		bIsValidClient ? TEXT("Yes") : TEXT("No"));
	
	return bIsValidClient;
}

int32 FEasyWaitForHostValidationState::GetConnectedPlayerCount() const
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

void FEasyWaitForHostValidationState::LogConnectionState() const
{
	if (!Controller)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Controller is null"));
		return;
	}
	
	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] World is null"));
		return;
	}
	
	// Log basic world info
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] World Info: %s"), *GetDebugStringForWorld(World));
	
	// Log player controller info
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] PlayerController: %s"), *PC->GetName());
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] HasNetConnection: %s"), PC->GetNetConnection() ? TEXT("Yes") : TEXT("No"));
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] HasPlayerState: %s"), PC->GetPlayerState<APlayerState>() ? TEXT("Yes") : TEXT("No"));
	}
	
	// Log GameState info
	AGameStateBase* GameState = World->GetGameState();
	if (GameState)
	{
		int32 PlayerCount = GameState->PlayerArray.Num();
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] GameState PlayerArray count: %d"), PlayerCount);
		
		for (int32 i = 0; i < PlayerCount; i++)
		{
			if (APlayerState* PS = GameState->PlayerArray[i])
			{
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation]   Player %d: %s"), i, *PS->GetPlayerName());
			}
		}
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] GameState is null"));
	}
	
	// Log session info
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
			if (Session)
			{
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Session State: %s"), EOnlineSessionState::ToString(Session->SessionState));
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Session NumOpenPublicConnections: %d"), Session->NumOpenPublicConnections);
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] Session MaxPlayers: %d"), Session->SessionSettings.NumPublicConnections);
			}
			else
			{
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[Validation] No named session found"));
			}
		}
	}
}

