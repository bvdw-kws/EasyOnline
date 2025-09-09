// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#include "EasyOnlineTest.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogEasyOnlineTest, Log, All);

void FEasyOnlineTestModule::StartupModule()
{
	UE_LOG(LogEasyOnlineTest, Log, TEXT("EasyOnlineTest module started"));
}

void FEasyOnlineTestModule::ShutdownModule()
{
	UE_LOG(LogEasyOnlineTest, Log, TEXT("EasyOnlineTest module shutdown"));
}

IMPLEMENT_MODULE(FEasyOnlineTestModule, EasyOnlineTest)