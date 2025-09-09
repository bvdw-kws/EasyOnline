// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IEasyDataTransferPlayerInterface.generated.h"

class UEasyDataTransferPlayerComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class UEasyDataTransferPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface to access data transfer functionality without circular dependency.
 * This allows EasyDataTransferModule to work with PlayerStates without directly
 * referencing the EasyOnline module classes.
 */
class EASYDATATRANSFERMODULE_API IEasyDataTransferPlayerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Get the data transfer component for this player.
	 * @return The data transfer component, or nullptr if not available.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "EasyDataTransfer")
	UEasyDataTransferPlayerComponent* GetDataTransferComponent() const;
};