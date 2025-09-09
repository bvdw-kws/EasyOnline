// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Components/GameStateComponent.h"
#include "OnlineDelegateMacros.h"

#include "EasyOnlineLobbyModeComponent.generated.h"

class AAIController;
class AEasyOnlineGameMode_InGame;

DECLARE_MULTICAST_DELEGATE_OneParam(FEasyOnlineOnMapChanged, const FName& MapID);
typedef FEasyOnlineOnMapChanged::FDelegate FEasyOnlineOnMapChangedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FEasyOnlineOnGameModeChanged, const FName& GameModeID);
typedef FEasyOnlineOnGameModeChanged::FDelegate FEasyOnlineOnGameModeChangedDelegate;

/**
 * Component to select the map, the game mode and so on while inside the lobby.
 */
UCLASS()
class EASYONLINE_API UEasyOnlineLobbyModeComponent : public UGameStateComponent
{
	GENERATED_UCLASS_BODY()

public:
	void SetMapID(const FName& NewMapID);
	const FName& GetMapID() const { return MapID; }
	bool IsModeReady() const;

	void SetGameModeID(const FName& NewGameModeID);
	const FName& GetGameModeID() const { return GameModeID; }
	
	int32 GetModeMaxPlayingPlayers() const;

	DEFINE_ONLINE_DELEGATE_ONE_PARAM(EasyOnlineOnMapChanged, const FName&);
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(EasyOnlineOnGameModeChanged, const FName&);

	//~UActorComponent interface
protected:
	virtual void BeginPlay() override;
	//~End of UActorComponent interface
	
	//~ Begin UObject Interface.
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface.

protected:
	UPROPERTY(ReplicatedUsing=OnRep_MapID)
	FName MapID = NAME_None;
	
	UPROPERTY(ReplicatedUsing=OnRep_GameModeID)
	FName GameModeID = NAME_None;

private:
	UPROPERTY(Transient)
	TSoftClassPtr<AEasyOnlineGameMode_InGame> GameModeSoftClass;
	UPROPERTY(Transient)
	TSubclassOf<AEasyOnlineGameMode_InGame> GameModeClass;
	UPROPERTY(Transient)
	bool bIsGameModeReady = false;
	
	UFUNCTION()
	void OnRep_MapID();
	
	UFUNCTION()
	void OnRep_GameModeID();

	UFUNCTION()
	void OnGameModeLoaded();

	FDelegateHandle OnMapAssetsLoadedDelegateHandle;
	void ResetDefaultMapID();

	FDelegateHandle OnGameModeAssetsLoadedDelegateHandle;
	void ResetDefaultGameModeID();

	void ApplyGameMode(const TSubclassOf<AEasyOnlineGameMode_InGame>& NewGameModeClass);
};
