// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "EasyOnlinePlayerState.h"

#include "DataTransfer/Components/EasyDataTransferPlayerComponent.h"
#include "Net/UnrealNetwork.h"

AEasyOnlinePlayerState::AEasyOnlinePlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create the data transfer component
	DataTransferComponent = CreateDefaultSubobject<UEasyDataTransferPlayerComponent>(TEXT("DataTransferComponent"));
}

void AEasyOnlinePlayerState::BeginPlay()
{
	Super::BeginPlay();
}

UEasyDataTransferPlayerComponent* AEasyOnlinePlayerState::GetDataTransferComponent_Implementation() const
{
	return DataTransferComponent;
}