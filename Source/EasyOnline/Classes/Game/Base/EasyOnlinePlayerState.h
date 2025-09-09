// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DataTransfer/IEasyDataTransferPlayerInterface.h"
#include "EasyOnlinePlayerState.generated.h"

class UEasyDataTransferPlayerComponent;

/**
 * Base player state class that includes data transfer functionality.
 * All EasyOnline player states should inherit from this class.
 */
UCLASS(Blueprintable, BlueprintType)
class EASYONLINE_API AEasyOnlinePlayerState : public APlayerState, public IEasyDataTransferPlayerInterface
{
	GENERATED_BODY()

public:
	AEasyOnlinePlayerState(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface

	//~ Begin IEasyDataTransferPlayerInterface Interface
	/**
	 * Get the data transfer component for this player.
	 * @return The data transfer component, or nullptr if not available.
	 */
	virtual UEasyDataTransferPlayerComponent* GetDataTransferComponent_Implementation() const override;
	//~ End IEasyDataTransferPlayerInterface Interface

protected:
	/**
	 * Component that handles data transfers for this player.
	 * Automatically created and managed.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEasyDataTransferPlayerComponent> DataTransferComponent;
};