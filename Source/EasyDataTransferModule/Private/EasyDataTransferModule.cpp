// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "EasyDataTransferModule.h"

#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "FEasyDataTransferModule"

void FEasyDataTransferModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("EasyDataTransferModule: Module started"));
}

void FEasyDataTransferModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("EasyDataTransferModule: Module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FEasyDataTransferModule, EasyDataTransferModule)