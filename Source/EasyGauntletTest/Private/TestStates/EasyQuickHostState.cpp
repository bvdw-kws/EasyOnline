#include "TestStates/EasyQuickHostState.h"
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
#include "Gauntlet/EasyGauntletController.h"

FEasyQuickHostState::FEasyQuickHostState(UEasyGauntletController* InController, float InTimeout, const FString& InMapName)
	: FEasyGauntletState(InController)
	, Timeout(InTimeout)
	, ElapsedTime(0.0f)
	, MapName(InMapName)
	, bQuickHostInitiated(false)
	, bWaitingForSession(false)
{
}

void FEasyQuickHostState::OnEnterState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Entering EasyQuickHostState"));
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Timeout: %.1f seconds"), Timeout);
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Map: %s"), MapName.IsEmpty() ? TEXT("Default") : *MapName);
	
	ElapsedTime = 0.0f;
	bQuickHostInitiated = false;
	bWaitingForSession = false;
	
	// Start quick host on next tick to ensure everything is properly initialized
}

void FEasyQuickHostState::OnTick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	
	// Check timeout
	if (ElapsedTime >= Timeout)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] Timeout reached (%.1f seconds) - Quick host failed"), Timeout);
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] Quick host operation timed out"));
		FinishState(false);
		return;
	}
	
	// Initiate quick host if not already done
	if (!bQuickHostInitiated)
	{
		InitiateQuickHost();
		return;
	}
	
	// Check if session is now active
	if (bWaitingForSession && IsSessionActive())
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Session successfully created! Elapsed time: %.1f seconds"), ElapsedTime);
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Quick host completed successfully"));
		FinishState(true);
		return;
	}
	
	// Log progress every 5 seconds
	if (FMath::Fmod(ElapsedTime, 5.0f) < DeltaTime)
	{
		UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Still waiting for session creation... (%.1f/%.1f seconds)"), ElapsedTime, Timeout);
	}
}

void FEasyQuickHostState::OnExitState()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Exiting EasyQuickHostState"));
}

void FEasyQuickHostState::InitiateQuickHost()
{
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Initiating QuickHost operation..."));
	
	if (!Controller)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] Controller is null"));
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] Controller is null"));
		FinishState(false);
		return;
	}
	
	UWorld* World = Controller->GetWorld();
	if (!World)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] World is null"));
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] World is null"));
		FinishState(false);
		return;
	}
	
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] PlayerController is null"));
		UE_LOG(LogEasyGauntletTest, Error, TEXT("[QuickHost] PlayerController is null"));
		FinishState(false);
		return;
	}
	
	// Log current session state before attempting to host
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
			if (ExistingSession)
			{
				UE_LOG(LogEasyGauntletTest, Warning, TEXT("[QuickHost] Existing session found, state: %s"), 
					EOnlineSessionState::ToString(ExistingSession->SessionState));
			}
			else
			{
				UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] No existing session found"));
			}
		}
	}
	
	// Use EasyOnline's QuickHost function - same as the UI button
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] Calling UEasyOnlineFunctionLibrary::QuickHost"));
	UEasyOnlineFunctionLibrary::QuickHost(World, PlayerController, false); // false = public session
	
	bQuickHostInitiated = true;
	bWaitingForSession = true;
	
	UE_LOG(LogEasyGauntletTest, Display, TEXT("[QuickHost] QuickHost function called successfully"));
}

bool FEasyQuickHostState::IsSessionActive() const
{
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface())
		{
			FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
			if (Session)
			{
				bool bIsActive = (Session->SessionState == EOnlineSessionState::InProgress || 
								Session->SessionState == EOnlineSessionState::Pending);
				
				if (bIsActive)
				{
					UE_LOG(LogEasyGauntletTest, VeryVerbose, TEXT("[QuickHost] Session active - State: %s, NumOpenPublicConnections: %d"), 
						EOnlineSessionState::ToString(Session->SessionState), Session->NumOpenPublicConnections);
				}
				
				return bIsActive;
			}
		}
	}
	
	return false;
}