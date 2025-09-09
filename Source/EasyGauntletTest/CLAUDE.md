# EasyGauntletTest Framework

<!-- Tokens: ~800 -->

## Overview
Generic, reusable testing framework built on Unreal Engine's Gauntlet Automation Framework. Provides standardized patterns for automated testing across multiple projects with special focus on multiplayer scenarios, performance testing, and network validation.

## Core Architecture

### Design Principles
- **Generic & Reusable**: Framework components work across different projects
- **Extensible**: Easy to add project-specific test logic while maintaining common patterns
- **Multiplayer-First**: Built-in support for multi-client test orchestration
- **Performance-Aware**: Integrated profiling and metrics collection
- **Network-Safe**: Desync detection and rollback validation for deterministic games

### Key Components
- **Controllers**: `EasyGauntletController`, `EasyOnlineGauntletController`
- **States**: `EasyGauntletState`, `EasyDelayState`, `EasyWaitForGameState`
- **Multiplayer**: `EasyOnlineHostState`, `EasyOnlineJoinState`

## Core Components

### Controllers
- **UEasyGauntletController**: Base with parameter parsing, state machine, timeout handling
- **UEasyOnlineGauntletController**: Multiplayer orchestration, session management, network validation

### Test States
- **EasyGauntletState**: Base with logging and timing
- **EasyDelayState**: Configurable delays
- **EasyWaitForGameState**: Template GameState waiting
- **EasyOnlineHostState/JoinState**: Session hosting/joining

## Key Features
- **Parameter System**: Auto-parse easy.timeout, easy.skipCinematic, easy.profiler
- **State Machine**: Automatic progression, timeout handling, error propagation
- **Multiplayer**: Session management, client sync, network validation
- **Performance**: Frame time, memory, CPU profiling, custom metrics

## Quick Start

### Basic Usage
1. Inherit from `UEasyGauntletController`
2. Define states from `EasyGauntletState`
3. Configure transitions

### Example
```cpp
class UMyTestController : public UEasyGauntletController
{
    virtual void OnInit() override
    {
        AddState<EasyDelayState>(2.0f);
        AddState<EasyWaitForGameState<AMyGameState>>();
    }
};
```

## Integration
Add to Build.cs: `"EasyGauntletTest"`, `"Gauntlet"`, `"AutomationController"`

**Usage**: Direct Gauntlet integration, automated test suites, CI/CD pipelines

## Related Documentation
- **Technical Specification**: [EasyGauntletTest_TechnicalSpecification.md](../../../../Docs/Spec/EasyGauntletTest_TechnicalSpecification.md)
- **UE Gauntlet Documentation**: https://dev.epicgames.com/documentation/en-us/unreal-engine/gauntlet-automation-framework-in-unreal-engine
- **EasyOnline Integration**: Check EasyOnline module documentation for session management patterns