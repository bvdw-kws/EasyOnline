#include "TestStates/EasyQuickJoinState.h"
#include "EasyGauntletTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Utility/EasyOnlineFunctionLibrary.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

FEasyQuickJoinState::FEasyQuickJoinState(UEasyGauntletController* InController, float InTimeout, int32 InMaxRetries, float InRetryInterval)
	: FEasyGauntletState(InController)
	, Timeout(InTimeout)
	, ElapsedTime(0.0f)
	, MaxRetries(InMaxRetries)
	, CurrentRetry(0)
	, RetryInterval(InRetryInterval)
	, TimeSinceLastAttempt(0.0f)
	, bWaitingForJoinResult(false)
	, bJoinSuccessful(false)
{
}

void FEasyQuickJoinState::OnEnterState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Entering EasyQuickJoinState"));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Timeout: %.1f seconds"), Timeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Max Retries: %d"), MaxRetries);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Retry Interval: %.1f seconds"), RetryInterval);
	
	ElapsedTime = 0.0f;
	CurrentRetry = 0;
	TimeSinceLastAttempt = RetryInterval; // Start immediately
	bWaitingForJoinResult = false;
	bJoinSuccessful = false;
}

void FEasyQuickJoinState::OnTick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	TimeSinceLastAttempt += DeltaTime;
	
	// Check timeout
	if (ElapsedTime >= Timeout)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] Timeout reached (%.1f seconds) after %d attempts - Quick join failed"), 
			Timeout, CurrentRetry);
		LogSessionState();
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] Quick join failed after %d attempts in %.1f seconds"), CurrentRetry, Timeout);
		FinishState(false);
		return;
	}
	
	// Check if we successfully joined
	if (IsJoinedToSession())
	{
		if (!bJoinSuccessful) // First time detecting success
		{
			bJoinSuccessful = true;
			UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Successfully joined session! Attempt: %d, Elapsed time: %.1f seconds"), 
				CurrentRetry, ElapsedTime);
			LogSessionState();
			UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Quick join succeeded on attempt %d"), CurrentRetry);
			FinishState(true);
			return;
		}
	}
	
	// Attempt QuickJoin if it's time for a retry
	if (!bWaitingForJoinResult && TimeSinceLastAttempt >= RetryInterval)
	{
		if (CanRetry())
		{
			AttemptQuickJoin();
		}
		else
		{
			UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] Max retries (%d) reached - Quick join failed"), MaxRetries);
			LogSessionState();
			UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] Quick join failed after maximum %d attempts"), MaxRetries);
			FinishState(false);
			return;
		}
	}
	
	// Reset waiting flag after some time if we're still waiting
	if (bWaitingForJoinResult && TimeSinceLastAttempt > RetryInterval)
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("[QuickJoin] Join attempt timed out, will retry"));
		bWaitingForJoinResult = false;
	}
	
	// Log progress every 10 seconds
	if (FMath::Fmod(ElapsedTime, 10.0f) < DeltaTime)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Still attempting to join... Attempt: %d/%d, Elapsed: %.1f/%.1f seconds"), 
			CurrentRetry, MaxRetries, ElapsedTime, Timeout);
		LogSessionState();
	}
}

void FEasyQuickJoinState::OnExitState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Exiting EasyQuickJoinState - Final state:"));
	LogSessionState();
}

void FEasyQuickJoinState::AttemptQuickJoin()
{
	CurrentRetry++;
	TimeSinceLastAttempt = 0.0f;
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Attempting QuickJoin (attempt %d/%d)..."), CurrentRetry, MaxRetries);
	
	if (!Controller)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] Controller is null"));
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] Controller is null"));
		FinishState(false);
		return;
	}
	
	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] World is null"));
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] World is null"));
		FinishState(false);
		return;
	}
	
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] PlayerController is null"));
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickJoin] PlayerController is null"));
		FinishState(false);
		return;
	}
	
	// Log current state before attempting to join
	LogSessionState();
	
	// Use EasyOnline's QuickJoin function - same as the UI button
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Calling UEasyOnlineFunctionLibrary::QuickJoin"));
	UEasyOnlineFunctionLibrary::QuickJoin(World, PlayerController);
	
	bWaitingForJoinResult = true;
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] QuickJoin function called successfully"));
}

bool FEasyQuickJoinState::IsJoinedToSession() const
{
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
			if (Session)
			{
				bool bIsJoined = (Session->SessionState == EOnlineSessionState::InProgress);
				
				if (bIsJoined)
				{
					UE_LOG(LogEasyGauntletTest, VeryVerbose, TEXT("[QuickJoin] Joined session - State: %s, NumOpenPublicConnections: %d"), 
						EOnlineSessionState::ToString(Session->SessionState), Session->NumOpenPublicConnections);
				}
				
				return bIsJoined;
			}
		}
	}
	
	return false;
}

bool FEasyQuickJoinState::CanRetry() const
{
	return CurrentRetry < MaxRetries;
}

void FEasyQuickJoinState::LogSessionState() const
{
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Online Subsystem: %s"), 
			*OnlineSubsystem->GetSubsystemName().ToString());
		
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
			if (Session)
			{
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Session State: %s"), EOnlineSessionState::ToString(Session->SessionState));
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Session NumOpenPublicConnections: %d"), Session->NumOpenPublicConnections);
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] Session MaxPlayers: %d"), Session->SessionSettings.NumPublicConnections);
			}
			else
			{
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickJoin] No active session found"));
			}
		}
		else
		{
			UE_LOG(LogEasyGauntletTest, Warning, TEXT("[QuickJoin] Session interface is null"));
		}
	}
	else
	{
		UE_LOG(LogEasyGauntletTest, Warning, TEXT("[QuickJoin] Online subsystem is null"));
	}
}