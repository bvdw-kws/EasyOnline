// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "GameFramework/GameState.h"

#include "EasyOnlineGameState_Lobby.generated.h"

class UEasyOnlineLobbyModeComponent;
class AEasyOnlinePlayerState_Lobby;

UCLASS(config=EasyOnline, defaultconfig)
class EASYONLINE_API AEasyOnlineGameState_Lobby : public AGameState
{
	GENERATED_UCLASS_BODY()

	//~ Begin AActor Interface
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface

public:
	bool IsGameStartCountdown() const;
	void StartGameCountdown();

	bool CanAddPlayingPlayer() const;
	void OnNewPlayerJoined();
	
	UFUNCTION(BlueprintPure, Category=Lobby)
	bool CanLobbyStartGame() const;
	
	UFUNCTION(BlueprintPure, Category=Lobby)
	int32 GetLobbyNumPlayers(bool bPlayingOnly = false) const;

	UFUNCTION(BlueprintPure, Category=Lobby)
	int32 GetLobbyNumPublicConnections() const;
	
	UFUNCTION(BlueprintPure, Category=Lobby)
	int32 GetLobbyMinPlayingPlayers() const;
		
	UFUNCTION(BlueprintPure, Category=Lobby)
	int32 GetLobbyMaxPlayingPlayers() const;

	UEasyOnlineLobbyModeComponent* GetLobbyModeComponentChecked() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UEasyOnlineBotCreationComponent> BotCreationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess="true"))
	TObjectPtr<class UEasyOnlineLobbyModeComponent> LobbyModeComponent;
	
private:
	UFUNCTION()
	void OnTimerCountdownDecreased();

private:
	UPROPERTY(ReplicatedUsing=OnRep_TimeRemaining)
	uint32 TimeRemaining = 5;

	UPROPERTY(Transient)
	FTimerHandle CountdownTimerHandle;
	
	UFUNCTION()
	void OnRep_TimeRemaining();

	UFUNCTION()
	void OnGameModeChanged(const FName& GameModeID);

	void FillEmptyPlayerSlotWithBot() const;
	void EnforceGameModeConstrains();
	bool CanAutoStartGameCountdown() const;
};

