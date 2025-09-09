// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Components/GameStateComponent.h"

#include "EasyOnlineBotCreationComponent.generated.h"

class AAIController;

/**
 * Helper component to create and remove bots.
 * It internally keep track of the spawned bots.
 */
UCLASS()
class EASYONLINE_API UEasyOnlineBotCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UEasyOnlineBotCreationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetBotControllerClass(const TSubclassOf<AAIController>& ControllerClass);
	void SetNumBotsToCreate(int32 NumBots);

#if WITH_SERVER_CODE
public:
	virtual void SpawnOneBot();
	virtual void RemoveOneBot();
#endif // WITH_SERVER_CODE

	//~UActorComponent interface
protected:
	virtual void BeginPlay() override;
	//~End of UActorComponent interface
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=Teams)
	int32 NumBotsToCreate = 1;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TSubclassOf<AAIController> BotControllerClass;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<FString> RandomBotNames;

	TArray<FString> RemainingBotNames;

protected:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AAIController>> SpawnedBotList;

#if WITH_SERVER_CODE
public:
	void Cheat_AddBot() { SpawnOneBot(); }
	void Cheat_RemoveBot() { RemoveOneBot(); }

protected:
	virtual void ServerCreateBots();

	FString CreateBotName(int32 PlayerIndex);
#endif
};
