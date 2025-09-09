# EasyOnline Plugin - Complete Multiplayer Solution

## EasyDataTransfer - Reliable Data Transfer System for Unreal Engine

## Overview

The EasyOnline plugin provides a complete multiplayer solution for Unreal Engine, featuring the EasyDataTransfer system for reliable, efficient transmission of data between players. Built with a focus on performance, reliability, and ease of use, it provides a robust solution for transferring various types of data including game state, custom messages, and large files.

### Plugin Modules
- **EasyOnline**: Main module with lobby systems, player management, and online game modes
- **EasyDataTransferModule**: Dedicated data transfer system with chunking, compression, and flow control
- **EasyOnlineTest**: Comprehensive test suite ensuring system reliability

## Key Features

### 🚀 High Performance
- **Sliding Window Protocol**: Efficient flow control to prevent network congestion
- **Adaptive Chunking**: Dynamic chunk sizing based on network conditions
- **Compression Support**: Built-in data compression using Unreal's FCompression API
- **Thread-Safe Operations**: Atomic handle generation and concurrent transfer support

### 🛡️ Reliability
- **Data Integrity**: CRC32 checksums for all data chunks
- **Automatic Retry**: Configurable retry mechanisms for failed transfers
- **Timeout Handling**: Robust timeout detection and cleanup
- **Error Recovery**: Comprehensive error handling and reporting

### 🔧 Developer Friendly
- **Blueprint Integration**: Full Blueprint support for all core functionality
- **Interface-Based Design**: Decoupled architecture avoiding circular dependencies
- **Extensive Testing**: Comprehensive unit and integration test suite
- **Detailed Documentation**: Thorough API documentation and examples

### 🌐 Network Optimized
- **Unreliable RPCs**: Uses unreliable RPCs for data chunks (efficiency)
- **Reliable RPCs**: Uses reliable RPCs for control messages (guaranteed delivery)
- **Bandwidth Management**: Global and per-transfer bandwidth limiting
- **Network Adaptation**: Automatic adjustment based on latency and packet loss

## Architecture

### Core Components

#### EasyDataTransferModule
The main runtime module containing all core functionality:

- **EasyDataTransferSubsystem**: Central management system for all transfers
- **EasyDataTransferPlayerComponent**: Player-specific component handling networking
- **EasyDataTransferSettings**: Configuration system with per-project settings
- **IEasyDataTransferPlayerInterface**: Interface for PlayerState integration

#### EasyOnlineTest
Comprehensive test suite ensuring system reliability:

- **Core Tests**: Basic functionality, data structures, utilities
- **Settings Tests**: Configuration validation and defaults
- **Component Tests**: PlayerComponent functionality and networking
- **Subsystem Tests**: GameInstanceSubsystem behavior and lifecycle
- **Integration Tests**: End-to-end transfer scenarios
- **Performance Tests**: Memory usage, throughput, stress testing

### Data Flow

```
Player A → PlayerComponent → Subsystem → Network → Subsystem → PlayerComponent → Player B
          ↓                   ↓                     ↓                    ↓
      Chunking          Compression           Validation          Reassembly
```

## API Reference

### Core Subsystem Methods

#### Opening Data Channels
```cpp
/**
 * Open a data channel to transfer data to another player.
 * @param ChannelName Unique identifier for this transfer
 * @param TargetPlayer Player to receive the data
 * @param Data Raw data to transfer
 * @param Options Transfer configuration options
 * @return Transfer handle for tracking progress (0 if failed)
 */
int32 OpenDataChannel(
    const FString& ChannelName,
    APlayerState* TargetPlayer,
    const TArray<uint8>& Data,
    const FEasyDataTransferOptions& Options = FEasyDataTransferOptions()
);
```

#### Transfer Management
```cpp
// Get transfer status
EDataTransferStatus GetTransferStatus(int32 Handle);

// Get transfer progress (0.0 to 1.0)
float GetTransferProgress(int32 Handle);

// Close specific transfer
void CloseDataChannel(int32 Handle, const FString& Reason);

// Close all transfers for a player
void CloseAllTransfersForPlayer(APlayerState* Player, const FString& Reason);

// Get active transfers for a player
TArray<int32> GetActiveTransfersForPlayer(APlayerState* Player);
```

### Event Delegates

```cpp
// Called when data is successfully received
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataReceived, 
    int32 /*Handle*/, const FString& /*ChannelName*/, const TArray<uint8>& /*Data*/);

// Called when data is successfully sent
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDataSent, 
    int32 /*Handle*/, const FString& /*ChannelName*/);

// Called periodically during transfer with progress updates
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataProgress, 
    int32 /*Handle*/, const FString& /*ChannelName*/, float /*Progress*/);

// Called when transfer errors occur
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataError, 
    int32 /*Handle*/, const FString& /*ChannelName*/, EDataTransferError /*Error*/);

// Called when transfers are closed or cancelled
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnDataTransferClosed, 
    int32 /*Handle*/, const FString& /*ChannelName*/, const FString& /*Reason*/);
```

### Configuration Options

#### Per-Transfer Settings
```cpp
USTRUCT(BlueprintType)
struct FEasyDataTransferOptions
{
    // Transfer priority (higher = more bandwidth allocation)
    int32 Priority = 0;
    
    // Chunk size in bytes (0 = use global default)
    int32 ChunkSize = 0;
    
    // Enable compression
    bool bEnableCompression = true;
    
    // Compression level (1-9)
    int32 CompressionLevel = 6;
    
    // Timeout in seconds (0 = use global default)
    float TimeoutSeconds = 0.0f;
    
    // Maximum retry attempts
    int32 MaxRetries = 0;
    
    // Bandwidth limit in bytes/sec
    int32 BandwidthLimit = 0;
    
    // Enable adaptive chunk sizing
    bool bAdaptiveChunking = true;
    
    // Sliding window size for flow control
    int32 SlidingWindowSize = 0;
};
```

#### Global Settings
Configure global defaults in Project Settings → Game → Easy Data Transfer:

- **Transfer Defaults**: Chunk size, compression, timeouts
- **Limits**: Max transfer size, concurrent transfers
- **Bandwidth Management**: Global bandwidth limits
- **Network Adaptation**: Latency and packet loss thresholds
- **Security**: Channel name whitelisting

## Usage Examples

### Basic Data Transfer
```cpp
// Get the subsystem
UEasyDataTransferSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UEasyDataTransferSubsystem>();

// Prepare data
TArray<uint8> GameStateData;
// ... populate with your data

// Set up transfer options
FEasyDataTransferOptions Options;
Options.bEnableCompression = true;
Options.Priority = 100;

// Start transfer
int32 Handle = Subsystem->OpenDataChannel(TEXT("GameState"), TargetPlayerState, GameStateData, Options);

if (Handle != 0)
{
    // Set up progress callback
    Subsystem->OnDataProgress.AddUObject(this, &AMyGameMode::OnTransferProgress);
    
    // Set up completion callback
    Subsystem->OnDataReceived.AddUObject(this, &AMyGameMode::OnDataReceived);
}
```

### Type-Safe Struct Transfer (C++)
```cpp
// Define your data structure
USTRUCT()
struct FPlayerStats
{
    GENERATED_BODY()
    
    int32 Level = 1;
    float Health = 100.0f;
    int32 Score = 0;
    FVector Position = FVector::ZeroVector;
};

// Send typed data
FPlayerStats MyStats;
MyStats.Level = 42;
MyStats.Health = 85.5f;
MyStats.Score = 12345;
MyStats.Position = FVector(100, 200, 300);

int32 Handle = Subsystem->SendData(TEXT("PlayerStats"), TargetPlayerState, MyStats);

// Receive typed data
void AMyPlayerController::OnDataReceived(int32 Handle, const FString& ChannelName, const TArray<uint8>& Data)
{
    if (ChannelName == TEXT("PlayerStats"))
    {
        FPlayerStats ReceivedStats;
        if (Subsystem->GetTransferData(Handle, ReceivedStats))
        {
            UE_LOG(LogGame, Log, TEXT("Received player stats: Level=%d, Health=%.1f, Score=%d"), 
                   ReceivedStats.Level, ReceivedStats.Health, ReceivedStats.Score);
        }
    }
}
```

### Data Request/Response Pattern
```cpp
// Player A requests data from Player B
int32 RequestHandle = Subsystem->RequestData(TEXT("PlayerInventory"), PlayerBState);

// Player B responds to data requests
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Listen for data requests
    if (UEasyDataTransferSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UEasyDataTransferSubsystem>())
    {
        Subsystem->OnDataRequested.AddUObject(this, &AMyGameMode::HandleDataRequest);
    }
}

void AMyGameMode::HandleDataRequest(const FString& ChannelName, APlayerState* FromPlayer, const FEasyDataTransferOptions& Settings)
{
    if (ChannelName == TEXT("PlayerInventory"))
    {
        // Prepare inventory data
        FPlayerInventory InventoryData = GetPlayerInventory(FromPlayer);
        
        // Send response
        Subsystem->SendData(TEXT("PlayerInventory"), FromPlayer, InventoryData, Settings);
    }
}
```

### Implementing Player Interface
```cpp
// In your PlayerState class header
UCLASS()
class AMyPlayerState : public APlayerState, public IEasyDataTransferPlayerInterface
{
    GENERATED_BODY()

public:
    AMyPlayerState();
    
    // IEasyDataTransferPlayerInterface
    virtual UEasyDataTransferPlayerComponent* GetDataTransferComponent_Implementation() const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEasyDataTransferPlayerComponent> DataTransferComponent;
};

// In your PlayerState class implementation
AMyPlayerState::AMyPlayerState()
{
    DataTransferComponent = CreateDefaultSubobject<UEasyDataTransferPlayerComponent>(TEXT("DataTransferComponent"));
}

UEasyDataTransferPlayerComponent* AMyPlayerState::GetDataTransferComponent_Implementation() const
{
    return DataTransferComponent;
}
```

### Error Handling
```cpp
void AMyGameMode::OnTransferError(int32 Handle, const FString& ChannelName, EDataTransferError Error)
{
    switch (Error)
    {
        case EDataTransferError::Timeout:
            UE_LOG(LogGame, Warning, TEXT("Transfer %s timed out"), *ChannelName);
            // Implement retry logic
            break;
            
        case EDataTransferError::NetworkError:
            UE_LOG(LogGame, Error, TEXT("Network error in transfer %s"), *ChannelName);
            // Handle network issues
            break;
            
        case EDataTransferError::SizeLimitExceeded:
            UE_LOG(LogGame, Error, TEXT("Transfer %s exceeded size limit"), *ChannelName);
            // Handle oversized data
            break;
    }
}
```

## Testing

### Running Tests
The system includes comprehensive automated tests covering all functionality:

```bash
# Run all EasyDataTransfer tests
UE_INSTALL_PATH/Engine/Binaries/Win64/UnrealEditor-Cmd.exe \
    ProjectPath/Project.uproject \
    -ExecCmds="Automation RunTests EasyOnline.DataTransfer"

# Run specific test categories
-ExecCmds="Automation RunTests EasyOnline.DataTransfer.Core"
-ExecCmds="Automation RunTests EasyOnline.DataTransfer.Integration"
-ExecCmds="Automation RunTests EasyOnline.DataTransfer.Network"
-ExecCmds="Automation RunTests EasyOnline.DataTransfer.Performance"
-ExecCmds="Automation RunTests EasyOnline.DataTransfer.Protocol"
-ExecCmds="Automation RunTests EasyOnline.DataTransfer.Security"
```

### Test Implementation Highlights

#### NetSimulation & Networking in Unit Tests
The system fully supports **NetSimulation** and **Unreal networking replication** in unit tests:

**Advanced Network Simulation** using UE5.6's NetEmulation commands:
```cpp
// Simulate realistic network conditions
ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("NetEmulation.PktLoss 20")));
ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("NetEmulation.PktLag 100")));
ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("NetEmulation.PktLagVariance 50")));

// Test with extended timeouts for adverse conditions
Options.TimeoutSeconds = 30.0f;
Options.MaxRetries = 5;
Options.SlidingWindowSize = 2; // Conservative window for packet loss
```

**Networking Integration**: Tests use actual UE networking features:
- **RPC Communication**: Tests real Client/Server RPCs for data transfer
- **Component Replication**: PlayerComponent uses proper replication setup
- **Network Validation**: All chunks validated with checksums and size limits
- **Multiplayer Simulation**: Tests simulate multiple players with realistic network conditions

#### Critical Test Setup Patterns

**Test Actor Creation** - Essential for PlayerState testing:
```cpp
// Create test player with proper component setup
ATestPlayerState* Player = NewObject<ATestPlayerState>(TestWorld);
Player->PostInitProperties();

// Get and initialize the component for testing
UEasyDataTransferPlayerComponent* Component = 
    IEasyDataTransferPlayerInterface::Execute_GetDataTransferComponent(Player);
if (Component) {
    Component->BeginPlay(); // CRITICAL: Initialize component for tests
    Component->AddToRoot(); // Prevent GC during test
}
```

**Explicit Sender Pattern** - Required for unit test environments:
```cpp
// Use explicit sender version for reliable testing
int32 Handle = Subsystem->OpenDataChannel(
    ChannelName, 
    SenderPlayer,    // Explicit sender
    ReceiverPlayer,  // Target player
    Data, 
    Options
);
// Auto-detection version works in runtime but not in test environments
```

**GameInstance Lifecycle** - Essential for subsystem testing:
```cpp
UGameInstance* GameInstance = NewObject<UGameInstance>(TestWorld);
TestWorld->SetGameInstance(GameInstance);
GameInstance->Init(); // CRITICAL: Triggers subsystem initialization

UEasyDataTransferSubsystem* Subsystem = 
    GameInstance->GetSubsystem<UEasyDataTransferSubsystem>();
```

**Rate Limiting Validation** - Per-player concurrent transfer limits:
```cpp
// System enforces MaxConcurrentTransfersPerPlayer (default: 5)
for (int32 i = 0; i < 10; ++i) {
    int32 Handle = Subsystem->OpenDataChannel(...);
    // First 5 succeed, remaining 5 are rejected
}
```

#### Simple World Creation
Streamlined pattern avoiding complex initialization:
```cpp
UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
TestWorld->AddToRoot(); // Prevent GC
FURL URL;
TestWorld->InitializeActorsForPlay(URL); // Minimal actor support
```

### Test Categories (19+ Automated Tests)

#### Core Tests (3 tests)
- Handle generation uniqueness and performance (10,000 handles)
- Data compression/decompression integrity with round-trip validation
- Transfer state management and lifecycle with progress tracking

#### Settings Tests (2 tests)
- Default configuration values validation
- Per-transfer options and default application logic

#### Component Tests (2 tests)
- PlayerComponent initialization and proper world registration
- Comprehensive chunk validation against malicious data

#### Subsystem Tests (2 tests)  
- GameInstanceSubsystem initialization with proper GameInstance lifecycle
- Complete transfer lifecycle management with handle tracking

#### Integration Tests (4 tests)
- Small data transfers (single chunk scenarios)
- Large data transfers (multi-chunk with progress callbacks)
- Compression validation with compressible and incompressible data
- Error handling and recovery with timeout/cancellation scenarios

#### Performance Tests (3 tests)
- Handle generation performance (100,000 handles benchmark)
- Memory usage validation under load with garbage collection
- Concurrent transfer stress testing (20 players, multiple transfers)

#### Network Tests (4 tests)
- **Packet Loss Simulation**: 20% packet loss with extended timeouts and retry logic
- **High Latency Testing**: 200ms+ latency simulation with adaptive timeouts
- **Jitter Testing**: Variable network delays with sliding window adaptation
- **Combined Conditions**: Multiple network issues with comprehensive error handling

#### Protocol Tests (2 tests)
- **Sliding Window Flow Control**: Multi-chunk transfers with window size validation
- **Timeout and Retry Logic**: Comprehensive timeout handling with backoff strategies

#### Security Tests (1 test)
- **Data Integrity**: CRC32 validation and corruption detection with malicious data

## Configuration

### Project Settings
Navigate to **Edit → Project Settings → Game → Easy Data Transfer** to configure:

#### Transfer Defaults
- **Default Chunk Size**: 1024 bytes (recommended: 512-4096)
- **Default Enable Compression**: true
- **Default Compression Level**: 6 (balance of speed/ratio)
- **Default Timeout**: 30 seconds
- **Default Max Retries**: 3
- **Default Adaptive Chunking**: true

#### System Limits  
- **Max Transfer Size**: 100MB (safety limit)
- **Max Concurrent Transfers Per Player**: 5
- **Max Concurrent Transfers Total**: 50

#### Network Settings
- **Global Bandwidth Limit**: 0 (unlimited)
- **Default Sliding Window Size**: 5
- **High Latency Threshold**: 200ms
- **Medium Latency Threshold**: 100ms
- **Packet Loss Threshold**: 5%

#### Security
- **Require Channel Whitelist**: false
- **Allowed Channel Names**: (empty by default)

### Console Variables
Runtime configuration via console:

```
# Enable debug logging
easydatatransfer.debug.logging 1

# Show transfer statistics
easydatatransfer.debug.stats 1

# Override global bandwidth limit (bytes/sec)
easydatatransfer.bandwidth.global 1048576

# Override chunk size for new transfers
easydatatransfer.chunk.size 2048
```

## Performance Guidelines

### Optimal Usage Patterns

#### Chunk Size Selection
- **Small data (< 1KB)**: Use default chunk size
- **Medium data (1-100KB)**: 1024-2048 byte chunks
- **Large data (> 100KB)**: 2048-4096 byte chunks
- **Very large data**: Enable adaptive chunking

#### Compression Guidelines
- **Enable for**: Repetitive data, text, JSON, XML
- **Disable for**: Already compressed data (images, audio, video)
- **Compression levels**: 1-3 (fast), 4-6 (balanced), 7-9 (maximum)

#### Bandwidth Management
```cpp
// High priority transfers (critical game state)
Options.Priority = 100;
Options.BandwidthLimit = 0; // No specific limit

// Low priority transfers (cosmetic data)
Options.Priority = 10;
Options.BandwidthLimit = 10240; // 10KB/s limit
```

#### Concurrent Transfer Limits
- **Recommended per player**: 2-3 active transfers
- **Recommended total**: 20-30 for 10+ players
- **Monitor performance**: Use performance tests to validate

### Memory Considerations
- Each active transfer holds complete data in memory
- Compression reduces network usage but increases CPU/memory usage
- Large transfers (> 10MB) should be streamed or split into smaller logical chunks

## Troubleshooting

### Common Issues

#### "Module not found" errors
```
LogInit: Warning: Incompatible or missing module: EasyDataTransferModule
```
**Solution**: Ensure the plugin is enabled in .uproject file and project has been compiled

#### Transfer timeouts
**Common causes**: Network congestion, oversized chunks, insufficient sliding window
**Solutions**: 
- Increase timeout duration
- Reduce chunk size
- Increase sliding window size
- Enable adaptive chunking

#### Memory usage spikes
**Causes**: Too many concurrent large transfers
**Solutions**:
- Reduce concurrent transfer limits
- Implement transfer queuing
- Split large data into smaller logical chunks

#### Poor compression ratios
**Causes**: Already compressed data, random data, very small chunks
**Solutions**:
- Disable compression for media files
- Increase chunk size for better compression
- Use appropriate compression levels

### Debug Features

#### Logging
```cpp
// Enable detailed logging
LogEasyDataTransfer.VerbosityLevel = Verbose

// Log categories
LogEasyDataTransfer: General transfer operations
LogEasyDataTransferNetwork: Network traffic details  
LogEasyDataTransferPerformance: Performance metrics
```

#### Statistics
Access transfer statistics via subsystem:
```cpp
// Get bandwidth usage
float CurrentBandwidthUsage = Subsystem->GetCurrentBandwidthUsage();

// Get active transfer count
int32 ActiveCount = Subsystem->GetActiveTransferCount();

// Get player-specific statistics
FEasyDataTransferPlayerStats Stats = Subsystem->GetPlayerStats(PlayerState);
```

## Version History

### Version 1.0.0
- Initial release with core data transfer functionality
- Comprehensive test suite with 50+ automated tests
- Full Blueprint integration
- Performance optimizations for large-scale multiplayer

### Planned Features
- **Streaming Support**: Direct file streaming without loading into memory
- **Peer-to-Peer Transfers**: Direct player-to-player transfers bypassing server
- **Advanced Compression**: Support for additional compression algorithms
- **Analytics Integration**: Built-in analytics for transfer performance monitoring

## Support

### Documentation
- API Reference: See header files in `Source/EasyDataTransferModule/Public/`
- Examples: Check `Source/EasyOnlineTest/Private/Tests/` for usage patterns
- Design Document: See `Docs/EasyDataTransfer_Design.md` for architecture details

### Contributing
1. Fork the repository
2. Create feature branch
3. Add comprehensive tests for new functionality
4. Follow existing code style and documentation standards
5. Submit pull request with detailed description

### License
Licensed under the Apache License, Version 2.0
See LICENSE file for full license text.

---

*EasyDataTransform - Making multiplayer data transfer simple and reliable.*