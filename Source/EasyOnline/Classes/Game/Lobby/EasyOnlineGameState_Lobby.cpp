// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineGameState_Lobby.h"

#include "Component/EasyOnlineBotCreationComponent.h"
#include "EasyOnlineBotController_Lobby.h"
#include "EasyOnlinePlayerState_Lobby.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Settings/EasyOnlineSettings.h"
#include "TimerManager.h"
#include "Component/EasyOnlineLobbyModeComponent.h"
#include "Game/EasyOnlineTypes.h"
#include "Utility/EasyOnlineFunctionLibrary.h"

AEasyOnlineGameState_Lobby::AEasyOnlineGameState_Lobby(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BotCreationComponent = ObjectInitializer.CreateDefaultSubobject<UEasyOnlineBotCreationComponent>(this, TEXT("BotCreationComponent"));
	if (BotCreationComponent)
	{
		BotCreationComponent->SetBotControllerClass(AEasyOnlineBotController_Lobby::StaticClass());
		BotCreationComponent->SetNumBotsToCreate(0);
	}
	
	LobbyModeComponent = ObjectInitializer.CreateDefaultSubobject<UEasyOnlineLobbyModeComponent>(this, TEXT("LobbyModeComponent"));
}

void AEasyOnlineGameState_Lobby::BeginPlay()
{
	Super::BeginPlay();

	if (LobbyModeComponent)
	{
		LobbyModeComponent->AddEasyOnlineOnGameModeChangedDelegate_Handle(
			FEasyOnlineOnGameModeChangedDelegate::CreateUObject(this, &ThisClass::OnGameModeChanged));
	}
}

void AEasyOnlineGameState_Lobby::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)  const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEasyOnlineGameState_Lobby, TimeRemaining);
}

bool AEasyOnlineGameState_Lobby::IsGameStartCountdown() const
{
	return CountdownTimerHandle.IsValid();
}

void AEasyOnlineGameState_Lobby::StartGameCountdown()
{
#if WITH_SERVER_CODE
	if (!CanLobbyStartGame())
	{
		return;
	}
	
	GetWorldTimerManager().SetTimer(CountdownTimerHandle,
		this, &AEasyOnlineGameState_Lobby::OnTimerCountdownDecreased, 1.0f, true);
#endif // WITH_SERVER_CODE
}

bool AEasyOnlineGameState_Lobby::CanAddPlayingPlayer() const
{
	const int32 NumPlayerJoinedGame = GetLobbyNumPlayers(true);
	const int32 MaxPlayingPlayers = GetLobbyMaxPlayingPlayers();
	return NumPlayerJoinedGame + 1 <= MaxPlayingPlayers;
}

void AEasyOnlineGameState_Lobby::OnNewPlayerJoined()
{
#if WITH_SERVER_CODE
	checkf(HasAuthority(), TEXT("%hs Should only be called on the Host"), __FUNCTION__)

	EnforceGameModeConstrains();
	
	if (CanAutoStartGameCountdown())
	{
		StartGameCountdown();
	}
#endif // WITH_SERVER_CODE
}

bool AEasyOnlineGameState_Lobby::CanLobbyStartGame() const
{
	if (!HasAuthority() || IsGameStartCountdown())
	{
		return false;
	}

	const int32 PlayerJoinedGameCount = GetLobbyNumPlayers();
	if (PlayerJoinedGameCount < GetLobbyMinPlayingPlayers())
	{
		return false;
	}

	if (!ensureAlwaysMsgf(GetLobbyNumPlayers(true) <= GetLobbyMaxPlayingPlayers(),
		TEXT("%hs Too many players are wanting to play"), __FUNCTION__))
	{
		return false;
	}

	if (!GetLobbyModeComponentChecked()->IsModeReady())
	{
		return false;
	}
	
	return true;
}

int32 AEasyOnlineGameState_Lobby::GetLobbyNumPlayers(bool bPlayingOnly) const
{
	int32 PlayerJoinedGameCount = 0;
	
	for (const APlayerState* PlayerState : PlayerArray)
	{
		if (const AEasyOnlinePlayerState_Lobby* LobbyPlayerState = Cast<AEasyOnlinePlayerState_Lobby>(PlayerState))
		{
			if (LobbyPlayerState->IsInactive() || LobbyPlayerState->IsABot() || (bPlayingOnly && LobbyPlayerState->IsSpectator()))
			{
				continue;
			}

			PlayerJoinedGameCount++;
		}
	}

	return PlayerJoinedGameCount;
}

int32 AEasyOnlineGameState_Lobby::GetLobbyNumPublicConnections() const
{
	const IOnlineSessionPtr Sessions = Online::GetSessionInterface(GetWorld());
	if (!ensureAlwaysMsgf(Sessions != nullptr,
		TEXT("%hs Invalid session"), __FUNCTION__))
	{
		return 0;
	}
	
	const FOnlineSessionSettings* SessionSettings = Sessions->GetSessionSettings(NAME_GameSession);
	if (SessionSettings == nullptr)
	{
		UE_LOG(LogEasyOnline, Warning, TEXT("%hs No NAME_GameSession could be found"), __FUNCTION__);
		return 4;
	}

	return SessionSettings->NumPublicConnections;
}

int32 AEasyOnlineGameState_Lobby::GetLobbyMinPlayingPlayers() const
{
	return 1;
}

int32 AEasyOnlineGameState_Lobby::GetLobbyMaxPlayingPlayers() const
{
	if (LobbyModeComponent)
	{
		return LobbyModeComponent->GetModeMaxPlayingPlayers();
	}
	else
	{		
		return 1;
	}
}

UEasyOnlineLobbyModeComponent* AEasyOnlineGameState_Lobby::GetLobbyModeComponentChecked() const
{
	check(LobbyModeComponent);
	return LobbyModeComponent;
}

void AEasyOnlineGameState_Lobby::OnTimerCountdownDecreased()
{
	TimeRemaining--;
	OnRep_TimeRemaining();

#if WITH_SERVER_CODE
	if (TimeRemaining == 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);

		const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
		if (EasyOnlineSettings->bFillEmptySlotWithBot)
		{
			FillEmptyPlayerSlotWithBot();
		}
		
		if (UWorld* World = GetWorld())
		{
			const FName& MapID = GetLobbyModeComponentChecked()->GetMapID();
			const FName& GameModeID = GetLobbyModeComponentChecked()->GetGameModeID();
			
			const FString MapURL = UEasyOnlineFunctionLibrary::GetMapURL(this, MapID, GameModeID);
			if (ensureAlwaysMsgf(!MapURL.IsEmpty(),
				TEXT("%hs Invalid map: %s"), __FUNCTION__, *MapID.ToString()))
			{				
				World->ServerTravel(MapURL);
			}
		}
	}
#endif // WITH_SERVER_CODE
}

void AEasyOnlineGameState_Lobby::OnRep_TimeRemaining()
{
	UKismetSystemLibrary::PrintString(this,
		FString::Printf(TEXT("%d seconds remaining before traveling to level"), TimeRemaining));
}

void AEasyOnlineGameState_Lobby::OnGameModeChanged(const FName& GameModeID)
{
	if (HasAuthority())
	{
		EnforceGameModeConstrains();
	}
}

void AEasyOnlineGameState_Lobby::FillEmptyPlayerSlotWithBot() const
{
#if WITH_SERVER_CODE
	const int32 PlayerJoinedGameCount = GetLobbyNumPlayers(true);
	const int32 PlayerMaxCount = GetLobbyMaxPlayingPlayers();		
	const int32 BotCount = PlayerMaxCount - PlayerJoinedGameCount;

	if (BotCreationComponent)
	{
		for (int32 BotIndex = 0; BotIndex < BotCount; BotIndex++)
		{
			BotCreationComponent->SpawnOneBot();
		}
	}
#endif // WITH_SERVER_CODE
}

void AEasyOnlineGameState_Lobby::EnforceGameModeConstrains()
{
#if WITH_SERVER_CODE
	checkf(HasAuthority(), TEXT("%hs Should only be called on the Host"), __FUNCTION__)
	
	const int32 MaxPlayingPlayers = GetLobbyMaxPlayingPlayers();

	int32 NumPlayerPlayers = 0;
	
	for (APlayerState* PlayerState : PlayerArray)
	{
		AEasyOnlinePlayerState_Lobby* LobbyPlayerState = Cast<AEasyOnlinePlayerState_Lobby>(PlayerState);
		if (!IsValid(LobbyPlayerState) || LobbyPlayerState->IsInactive() || LobbyPlayerState->IsABot() || LobbyPlayerState->IsSpectator())
		{
			continue;
		}

		if (NumPlayerPlayers >= MaxPlayingPlayers)
		{
			LobbyPlayerState->SetIsSpectator(true);
			continue;
		}

		NumPlayerPlayers++;
	}
#endif // WITH_SERVER_CODE
}

bool AEasyOnlineGameState_Lobby::CanAutoStartGameCountdown() const
{
	const int32 PlayerJoinedGameCount = GetLobbyNumPlayers();
	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	
	return PlayerJoinedGameCount == GetLobbyNumPublicConnections() && EasyOnlineSettings->bAutoStartLobbyTimer;
}
