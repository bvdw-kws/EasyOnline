// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "EasyOnlineFunctionLibrary.generated.h"

class AEasyOnlineGameMode_InGame;
class UEasyOnlineGameModeAsset;
class UEasyOnlineMapAsset;

UCLASS()
class EASYONLINE_API UEasyOnlineFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static FName GetDefaultMapID(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static TArray<UEasyOnlineMapAsset*> GetSortedMapAssets(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static TArray<UEasyOnlineMapAsset*> GetSortedMapAssetsWithTag(const UObject* WorldContextObject, FGameplayTag Tag);

	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static UEasyOnlineMapAsset* GetMapAsset(const UObject* WorldContextObject, FName MapID);
		
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static FName GetDefaultGameModeID(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static TArray<UEasyOnlineGameModeAsset*> GetSortedGameModeAssets(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static UEasyOnlineGameModeAsset* GetGameModeAsset(const UObject* WorldContextObject, FName GameModeID);

	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static TSoftClassPtr<AEasyOnlineGameMode_InGame> GetGameModeSoftClass(const UObject* WorldContextObject, FName GameModeID);
	
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static FString GetMapURL(const UObject* WorldContextObject,
		const UEasyOnlineMapAsset* MapAsset, const FName& GameModeID, bool bListenServer = true, int32 NumBots = 0);

	// Directly opens MapAsset via UGameplayStatics::OpenLevel, using the same URL/options building as
	// GetMapURL. Intended for local/single-player travel (e.g. Puzzle mode) that doesn't need a hosted
	// online session; use UEasyOnlineHost::HostGameMap (via CreateLobby/QuickHost) for that instead.
	UFUNCTION(BlueprintCallable, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void OpenMap(const UObject* WorldContextObject,
		const UEasyOnlineMapAsset* MapAsset, const FName& GameModeID = NAME_None, bool bListenServer = false, int32 NumBots = 0);

	UFUNCTION(BlueprintCallable, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void CreateLobby(const UObject* WorldContextObject, APlayerController* HostingPlayer, bool bPrivateSession);
	
	UFUNCTION(BlueprintCallable, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void QuickJoin(const UObject* WorldContextObject, APlayerController* JoiningPlayer);
	
	UFUNCTION(BlueprintCallable, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void QuickHost(const UObject* WorldContextObject, APlayerController* HostingPlayer, bool bPrivateSession);

	// Session validation utilities
	UFUNCTION(BlueprintPure, Category="EasyOnline")
	static bool IsSessionActiveOrPending();
};
