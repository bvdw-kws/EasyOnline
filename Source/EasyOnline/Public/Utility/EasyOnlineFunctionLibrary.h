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
	
	// Builds the travel URL for MapAsset: its own MapData.Options plus ExtraOptions
	// (already-formed URL options, e.g. built via AddOption/AddNumBotsOption below).
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static FString GetMapURLWithExtraOptions(const UObject* WorldContextObject,
		const UEasyOnlineMapAsset* MapAsset, const FString& ExtraOptions = FString());

	// Convenience overload of GetMapURLWithExtraOptions that also builds the listen/NumBots/game
	// options for you.
	UFUNCTION(BlueprintPure, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static FString GetMapURL(const UObject* WorldContextObject,
		const UEasyOnlineMapAsset* MapAsset, const FName& GameModeID, bool bListenServer = true, int32 NumBots = 0,
		const FString& ExtraOptions = FString());

	// Directly opens MapAsset via UGameplayStatics::OpenLevel, using the same URL/options building as
	// GetMapURLWithExtraOptions. Intended for local/single-player travel (e.g. Puzzle mode) that doesn't
	// need a hosted online session; use UEasyOnlineHost::HostGameMap (via CreateLobby/QuickHost) for that
	// instead. ExtraOptions is appended verbatim to the travel URL (build it with AddOption/
	// AddNumBotsOption/etc, e.g. AddOption(Options, TEXT("GameEditor"), TEXT("false"))).
	UFUNCTION(BlueprintCallable, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void OpenMapWithExtraOptions(const UObject* WorldContextObject,
		const UEasyOnlineMapAsset* MapAsset, const FString& ExtraOptions = FString());

	// Convenience overload of OpenMapWithExtraOptions that also builds the listen/NumBots/game options
	// for you.
	UFUNCTION(BlueprintCallable, Category="EasyOnline", meta=(WorldContext="WorldContextObject", UnsafeDuringActorConstruction="true"))
	static void OpenMap(const UObject* WorldContextObject,
		const UEasyOnlineMapAsset* MapAsset, const FName& GameModeID = NAME_None, bool bListenServer = false, int32 NumBots = 0,
		const FString& ExtraOptions = FString());

	// URL-options helpers, for building an ExtraOptions string to pass to GetMapURLWithExtraOptions/
	// OpenMapWithExtraOptions piece by piece.
	UFUNCTION(BlueprintCallable, Category="EasyOnline|Options")
	static void AddOption(UPARAM(ref) FString& Options, const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category="EasyOnline|Options")
	static void AddBoolOption(UPARAM(ref) FString& Options, const FString& Key, bool bValue);

	UFUNCTION(BlueprintCallable, Category="EasyOnline|Options")
	static void AddFlagOption(UPARAM(ref) FString& Options, const FString& Flag);

	UFUNCTION(BlueprintCallable, Category="EasyOnline|Options")
	static void AddListenServerOption(UPARAM(ref) FString& Options);

	UFUNCTION(BlueprintCallable, Category="EasyOnline|Options")
	static void AddNumBotsOption(UPARAM(ref) FString& Options, int32 NumBots);

	UFUNCTION(BlueprintCallable, Category="EasyOnline|Options", meta=(WorldContext="WorldContextObject"))
	static void AddGameModeOption(const UObject* WorldContextObject, UPARAM(ref) FString& Options, FName GameModeID);

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
