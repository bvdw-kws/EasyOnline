@echo off
:: Generic EasyOnline QuickJoin Gauntlet Test Runner
:: Can be customized via command line parameters

setlocal

:: Default configuration
set UNREAL_ENGINE_PATH=D:\Epic Games\UE_5.6\Engine
set PROJECT_PATH=%~dp0..\..\..\..\..\..\PG.uproject
set TEST_PROFILE=Quick
set HOST_TIMEOUT=60
set JOIN_TIMEOUT=120
set MAX_RETRIES=10
set RETRY_INTERVAL=5
set VALIDATION_TIMEOUT=30

:: Test profile configurations
if "%1"=="Quick" (
    set HOST_TIMEOUT=30
    set JOIN_TIMEOUT=60
    set MAX_RETRIES=5
    set RETRY_INTERVAL=3
    set VALIDATION_TIMEOUT=15
    echo [INFO] Using Quick test profile (faster, less thorough^)
) else if "%1"=="Standard" (
    set HOST_TIMEOUT=60
    set JOIN_TIMEOUT=120
    set MAX_RETRIES=10
    set RETRY_INTERVAL=5
    set VALIDATION_TIMEOUT=30
    echo [INFO] Using Standard test profile (balanced^)
) else if "%1"=="Extended" (
    set HOST_TIMEOUT=120
    set JOIN_TIMEOUT=300
    set MAX_RETRIES=20
    set RETRY_INTERVAL=10
    set VALIDATION_TIMEOUT=60
    echo [INFO] Using Extended test profile (thorough, slower^)
) else if "%1"=="Custom" (
    echo [INFO] Using Custom test profile - configure via environment variables
    echo [INFO] Set EASY_HOST_TIMEOUT, EASY_JOIN_TIMEOUT, EASY_MAX_RETRIES, etc.
    if defined EASY_HOST_TIMEOUT set HOST_TIMEOUT=%EASY_HOST_TIMEOUT%
    if defined EASY_JOIN_TIMEOUT set JOIN_TIMEOUT=%EASY_JOIN_TIMEOUT%
    if defined EASY_MAX_RETRIES set MAX_RETRIES=%EASY_MAX_RETRIES%
    if defined EASY_RETRY_INTERVAL set RETRY_INTERVAL=%EASY_RETRY_INTERVAL%
    if defined EASY_VALIDATION_TIMEOUT set VALIDATION_TIMEOUT=%EASY_VALIDATION_TIMEOUT%
) else if "%1"=="" (
    echo [INFO] No profile specified, using Standard profile
    echo [INFO] Available profiles: Quick, Standard, Extended, Custom
) else (
    echo [ERROR] Unknown profile: %1
    echo [INFO] Available profiles: Quick, Standard, Extended, Custom
    pause
    exit /b 1
)

echo.
echo ========================================
echo EasyOnline QuickJoin Gauntlet Test
echo ========================================
echo Profile: %TEST_PROFILE%
echo Host Timeout: %HOST_TIMEOUT%s
echo Join Timeout: %JOIN_TIMEOUT%s  
echo Max Retries: %MAX_RETRIES%
echo Retry Interval: %RETRY_INTERVAL%s
echo Validation Timeout: %VALIDATION_TIMEOUT%s
echo ========================================
echo.

:: Confirmation prompt
echo This will launch 2 game instances to test QuickHost/QuickJoin functionality
echo One instance will host, the other will attempt to join with retry logic
set /p CONFIRM="Continue? (Y/N): "
if /i not "%CONFIRM%"=="Y" (
    echo Test cancelled.
    pause
    exit /b 0
)

echo.
echo [INFO] Starting QuickJoin test...
echo [INFO] Check the logs for detailed test output and debugging information
echo.

:: Build test parameters
set TEST_PARAMS=-gauntlet.controller=EasyQuickJoinGauntletController
set TEST_PARAMS=%TEST_PARAMS% -easy.hostTimeout=%HOST_TIMEOUT%
set TEST_PARAMS=%TEST_PARAMS% -easy.joinTimeout=%JOIN_TIMEOUT%
set TEST_PARAMS=%TEST_PARAMS% -easy.maxRetries=%MAX_RETRIES%
set TEST_PARAMS=%TEST_PARAMS% -easy.retryInterval=%RETRY_INTERVAL%
set TEST_PARAMS=%TEST_PARAMS% -easy.validationTimeout=%VALIDATION_TIMEOUT%

:: Run the Gauntlet test
"%UNREAL_ENGINE_PATH%\Binaries\DotNET\AutomationTool\Bin\AutomationTool.exe" ^
    RunUnreal ^
    -project="%PROJECT_PATH%" ^
    -platform=Win64 ^
    -configuration=Development ^
    -test=EasyOnlineQuickJoin ^
    -nullrhi ^
    -unattended ^
    %TEST_PARAMS%

echo.
if %ERRORLEVEL%==0 (
    echo [SUCCESS] QuickJoin test completed successfully!
    echo [INFO] Both instances were able to connect via QuickHost/QuickJoin
) else (
    echo [FAILURE] QuickJoin test failed with error code: %ERRORLEVEL%
    echo [INFO] Check the detailed logs above for debugging information
    echo [INFO] Common issues:
    echo [INFO] - Host session creation failed
    echo [INFO] - Client unable to find/join session
    echo [INFO] - Network connectivity issues
    echo [INFO] - Timeout reached before successful connection
)

echo.
pause