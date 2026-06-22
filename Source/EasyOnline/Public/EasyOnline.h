// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Modules/ModuleManager.h"

/**
 * EasyOnline Plugin Module
 * 
 * Advanced multiplayer networking framework designed for competitive gaming with deterministic rollback netcode.
 * Provides comprehensive online functionality optimized for frame-perfect synchronization and low-latency gameplay.
 * 
 * Core Features:
 * - Deterministic session management for competitive multiplayer integrity
 * - Recall ECS integration for rollback netcode compatibility  
 * - Intelligent matchmaking with latency-aware session discovery
 * - Enhanced friend system with competitive priority algorithms
 * - Advanced spectator mode and reconnection handling
 * - Platform-agnostic networking with LAN fallback support
 * 
 * Architecture Benefits:
 * - Centralized online state management for consistent multiplayer experience
 * - Optimized networking protocols for competitive gaming requirements
 * - Real-time connection quality monitoring and automatic optimization
 * - Seamless integration with Unreal Engine's online subsystem infrastructure
 */
class EASYONLINE_API FEasyOnline : public IModuleInterface
{
};
