// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Game/EasyOnlineManagerSubsystem.h"
#include "UObject/Object.h"
#include "EasyOnlineHost.generated.h"

/**
 * 
 */
UCLASS()
class EASYONLINE_API UEasyOnlineHost : public UObject
{
	GENERATED_BODY()

public:
	/** Host game and travel to lobby map */
	bool HostLobby(const FUniqueNetId& HostPlayerId, bool bPrivateSession, int32 NumPublicConnections);
	
	/** Host game and travel to specific map */
	bool HostGameMap(const FUniqueNetId& HostPlayerId, const FString& MapName, bool bPrivateSession, int32 NumPublicConnections);
	
	/** Host game in-background. Stay in same map and start listen server mode in-background */
	bool HostGameInBackground(const FUniqueNetId& HostPlayerId, bool bPrivateSession, int32 NumPublicConnections);
	
	/** Stop host and session */
	UFUNCTION(BlueprintCallable, Category=Online)
	void StopHost();

	/** Return true if instance is listen server / hosting mode */
	bool IsHosting() const;
	
	FOnlineSessionSettings GenerateOnlineSessionSettings(bool bPrivateSession, int32 NumPublicConnections);
	
private:
	UEasyOnlineManagerSubsystem* GetOnlineManager() const;
	void OnCreateSessionToHostGameMapComplete(const FName& SessionName, bool bWasSuccessful);
	void OnCreateSessionToHostGameInBackgroundComplete(const FName& SessionName, bool bWasSuccessful);
	
	TOptional<FString> PendingHostMap;
};
