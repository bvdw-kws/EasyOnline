// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "EasyOnlineBotCreationComponent.h"

#include "AIController.h"
#include "Engine/World.h"
#include "Game/Base/EasyOnlineGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/EasyOnlineSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EasyOnlineBotCreationComponent)

UEasyOnlineBotCreationComponent::UEasyOnlineBotCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BotControllerClass = AAIController::StaticClass();
}

void UEasyOnlineBotCreationComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateBots();
	}
#endif // WITH_SERVER_CODE
}

void UEasyOnlineBotCreationComponent::SetBotControllerClass(const TSubclassOf<AAIController>& ControllerClass)
{
	BotControllerClass = ControllerClass;
}

void UEasyOnlineBotCreationComponent::SetNumBotsToCreate(int32 NumBots)
{
	NumBotsToCreate = NumBots;
}

#if WITH_SERVER_CODE

void UEasyOnlineBotCreationComponent::ServerCreateBots()
{
	if (BotControllerClass == nullptr)
	{
		return;
	}

	RemainingBotNames = RandomBotNames;

	// Determine how many bots to spawn
	int32 EffectiveBotCount = NumBotsToCreate;

	const UEasyOnlineSettings* EasyOnlineSettings = GetDefault<UEasyOnlineSettings>();
	
	if (EasyOnlineSettings->bOverrideBotCount)
	{
		EffectiveBotCount = EasyOnlineSettings->OverrideNumPlayerBotsToSpawn;
	}

	// Give the URL a chance to override it
	if (AGameModeBase* GameModeBase = GetGameMode<AGameModeBase>())
	{
		EffectiveBotCount = UGameplayStatics::GetIntOption(GameModeBase->OptionsString, TEXT("NumBots"), EffectiveBotCount);
	}

	// Create them
	for (int32 Count = 0; Count < EffectiveBotCount; ++Count)
	{
		SpawnOneBot();
	}
}

FString UEasyOnlineBotCreationComponent::CreateBotName(int32 PlayerIndex)
{
	FString Result;
	if (RemainingBotNames.Num() > 0)
	{
		const int32 NameIndex = FMath::RandRange(0, RemainingBotNames.Num() - 1);
		Result = RemainingBotNames[NameIndex];
		RemainingBotNames.RemoveAtSwap(NameIndex);
	}
	else
	{
		//@TODO: PlayerId is only being initialized for players right now
		PlayerIndex = FMath::RandRange(260, 260+100);
		Result = FString::Printf(TEXT("Tinplate %d"), PlayerIndex);
	}
	return Result;
}

void UEasyOnlineBotCreationComponent::SpawnOneBot()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;
	AAIController* NewController = GetWorld()->SpawnActor<AAIController>(BotControllerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnInfo);

	if (NewController != nullptr)
	{
		AEasyOnlineGameModeBase* GameMode = GetGameMode<AEasyOnlineGameModeBase>();
		check(GameMode);

		if (NewController->PlayerState != nullptr)
		{
			NewController->PlayerState->SetPlayerName(CreateBotName(NewController->PlayerState->GetPlayerId()));
		}

		GameMode->GenericPlayerInitialization(NewController);
		GameMode->RestartPlayer(NewController);

		/*
		if (NewController->GetPawn() != nullptr)
		{
			if (UEasyOnlinePawnExtensionComponent* PawnExtComponent = NewController->GetPawn()->FindComponentByClass<UEasyOnlinePawnExtensionComponent>())
			{
				PawnExtComponent->CheckDefaultInitialization();
			}
		}
		*/

		SpawnedBotList.Add(NewController);
	}
}

void UEasyOnlineBotCreationComponent::RemoveOneBot()
{
	if (SpawnedBotList.Num() > 0)
	{
		// Right now this removes a random bot as they're all the same; could prefer to remove one
		// that's high skill or low skill or etc... depending on why you are removing one
		const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);

		AAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];
		SpawnedBotList.RemoveAtSwap(BotToRemoveIndex);

		if (BotToRemove)
		{
			// If we can find a health component, self-destruct it, otherwise just destroy the actor
			if (APawn* ControlledPawn = BotToRemove->GetPawn())
			{
				ControlledPawn->Destroy();
			}

			// Destroy the controller (will cause it to Logout, etc...)
			BotToRemove->Destroy();
		}
	}
}

#endif // WITH_SERVER_CODE
