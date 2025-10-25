@echo off
REM DelveDeep Automated Test Runner (Windows)
REM Runs all unit tests and generates test reports for CI/CD integration

setlocal enabledelayedexpansion

REM Configuration
set "ENGINE_PATH=%UE5_ENGINE_PATH%"
if "%ENGINE_PATH%"=="" set "ENGINE_PATH=C:\UnrealEngine"

set "PROJECT_PATH=%~dp0DelveDeep.uproject"
set "OUTPUT_PATH=%~dp0TestResults"
set "TEST_FILTER=%TEST_FILTER%"
if "%TEST_FILTER%"=="" set "TEST_FILTER=Product"

REM Print banner
echo ========================================
echo DelveDeep Automated Test Runner
echo ========================================
echo.

REM Validate engine path
if not exist "%ENGINE_PATH%" (
    echo ERROR: Unreal Engine not found at: %ENGINE_PATH%
    echo Set UE5_ENGINE_PATH environment variable or edit this script
    exit /b 1
)

REM Validate project file
if not exist "%PROJECT_PATH%" (
    echo ERROR: Project file not found: %PROJECT_PATH%
    exit /b 1
)

REM Create output directory
if not exist "%OUTPUT_PATH%" mkdir "%OUTPUT_PATH%"
echo Output directory: %OUTPUT_PATH%
echo.

REM Set editor executable path
set "EDITOR_CMD=%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

REM Validate editor executable
if not exist "%EDITOR_CMD%" (
    echo ERROR: Editor executable not found: %EDITOR_CMD%
    exit /b 1
)

REM Print configuration
echo Configuration:
echo   Engine: %ENGINE_PATH%
echo   Project: %PROJECT_PATH%
echo   Test Filter: %TEST_FILTER%
echo   Editor: %EDITOR_CMD%
echo.

REM Run tests
echo Running tests...
echo.

REM Record start time
set START_TIME=%TIME%

REM Execute tests with proper flags
"%EDITOR_CMD%" ^
    "%PROJECT_PATH%" ^
    -ExecCmds="Automation RunFilter %TEST_FILTER%" ^
    -ReportOutputPath="%OUTPUT_PATH%" ^
    -unattended ^
    -nopause ^
    -NullRHI ^
    -log ^
    -stdout ^
    -FullStdOutLogOutput

set TEST_EXIT_CODE=%ERRORLEVEL%

REM Record end time
set END_TIME=%TIME%

echo.
echo ========================================

REM Check test results
if %TEST_EXIT_CODE% EQU 0 (
    echo [32m✓ All tests passed![0m
    echo   Reports: %OUTPUT_PATH%
    echo ========================================
    exit /b 0
) else (
    echo [31m✗ Tests failed![0m
    echo   Exit code: %TEST_EXIT_CODE%
    echo   Reports: %OUTPUT_PATH%
    echo ========================================
    exit /b %TEST_EXIT_CODE%
)
