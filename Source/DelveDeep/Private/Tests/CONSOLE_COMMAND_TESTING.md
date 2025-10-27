# Console Command Testing Guide

This guide explains how to use the console command testing utilities in the DelveDeep automated testing framework.

## Overview

The console command testing utilities provide comprehensive tools for:
- Executing console commands programmatically
- Capturing and verifying console output
- Testing parameter handling
- Verifying command side effects
- Validating command registration

## Core Utilities

### FConsoleOutputCapture

RAII-style helper for capturing console output during command execution.

```cpp
// Create output capture
DelveDeepTestUtils::FConsoleOutputCapture Capture;

// Execute command (output is captured automatically)
DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.ShowValidationMetrics"));

// Verify captured output
EXPECT_TRUE(Capture.ContainsOutput(TEXT("Total Validations")));
EXPECT_GT(Capture.GetOutputLineCount(), 0);

// Check for multiple strings
TArray<FString> ExpectedStrings = {
    TEXT("Total Validations"),
    TEXT("Passed Validations"),
    TEXT("Failed Validations")
};
EXPECT_TRUE(Capture.ContainsAllOutput(ExpectedStrings));
```

### ExecuteConsoleCommand

Executes a console command programmatically.

```cpp
// Execute simple command
bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.TestValidationSeverity"));
EXPECT_TRUE(bExecuted);

// Execute command with arguments
TArray<FString> Args;
Args.Add(TEXT("DelveDeep.Event.Test"));
bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommandWithArgs(
    TEXT("DelveDeep.Events.BroadcastTestEvent"),
    Args);
```

### ExecuteConsoleCommandWithCapture

Executes a command and captures its output in one call.

```cpp
TArray<FString> CapturedOutput;
bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommandWithCapture(
    TEXT("DelveDeep.ShowValidationMetrics"),
    CapturedOutput);

EXPECT_TRUE(bExecuted);
EXPECT_GT(CapturedOutput.Num(), 0);

// Verify output contains expected strings
for (const FString& Line : CapturedOutput)
{
    if (Line.Contains(TEXT("Total Validations")))
    {
        // Found expected output
    }
}
```

## Command Registration Testing

### IsConsoleCommandRegistered

Checks if a specific command is registered.

```cpp
bool bRegistered = DelveDeepTestUtils::IsConsoleCommandRegistered(TEXT("DelveDeep.ValidateAllData"));
EXPECT_TRUE(bRegistered);
```

### GetRegisteredConsoleCommands

Gets all commands matching a prefix.

```cpp
TArray<FString> DelveDeepCommands = DelveDeepTestUtils::GetRegisteredConsoleCommands(TEXT("DelveDeep."));

EXPECT_GT(DelveDeepCommands.Num(), 0);

for (const FString& Command : DelveDeepCommands)
{
    UE_LOG(LogTemp, Display, TEXT("Found command: %s"), *Command);
}
```

### VerifySubsystemCommandsRegistered

Verifies that all expected commands for a subsystem are registered.

```cpp
TArray<FString> ExpectedCommands = {
    TEXT("DelveDeep.ValidateObject"),
    TEXT("DelveDeep.ListValidationRules"),
    TEXT("DelveDeep.ShowValidationMetrics")
};

TArray<FString> MissingCommands;
bool bAllRegistered = DelveDeepTestUtils::VerifySubsystemCommandsRegistered(
    TEXT("Validation"),
    ExpectedCommands,
    MissingCommands);

EXPECT_TRUE(bAllRegistered);
EXPECT_EQ(MissingCommands.Num(), 0);
```

## Parameter Handling Testing

### TestConsoleCommandInvalidParameters

Tests that a command handles invalid parameters gracefully without crashing.

```cpp
TArray<FString> CapturedOutput;
TArray<FString> EmptyArgs;  // Missing required parameter

bool bHandledGracefully = DelveDeepTestUtils::TestConsoleCommandInvalidParameters(
    TEXT("DelveDeep.ValidateObject"),
    EmptyArgs,
    CapturedOutput);

EXPECT_TRUE(bHandledGracefully);

// Verify error message was logged
bool bFoundErrorMessage = false;
for (const FString& Line : CapturedOutput)
{
    if (Line.Contains(TEXT("Usage:")) || Line.Contains(TEXT("Error")))
    {
        bFoundErrorMessage = true;
        break;
    }
}

EXPECT_TRUE(bFoundErrorMessage);
```

## Side Effect Verification

### VerifyConsoleCommandSideEffects

Executes a command and verifies its side effects using a custom verification function.

```cpp
// Get subsystem
UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();

// Broadcast some events to generate metrics
FDelveDeepEventPayload TestPayload;
TestPayload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Test"));
EventSubsystem->BroadcastEvent(TestPayload);

// Verify reset command clears metrics
bool bVerified = DelveDeepTestUtils::VerifyConsoleCommandSideEffects(
    TEXT("DelveDeep.Events.ResetMetrics"),
    [EventSubsystem]() -> bool
    {
        const FEventSystemMetrics& Metrics = EventSubsystem->GetPerformanceMetrics();
        return Metrics.TotalEventsBroadcast == 0;
    });

EXPECT_TRUE(bVerified);
```

## Complete Test Examples

### Basic Command Execution Test

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBasicCommandExecutionTest,
    "DelveDeep.ConsoleCommands.BasicExecution",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FBasicCommandExecutionTest::RunTest(const FString& Parameters)
{
    // Test executing a valid command
    bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.TestValidationSeverity"));
    EXPECT_TRUE(bExecuted);

    // Test executing a non-existent command
    bool bFailedExecution = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.NonExistentCommand"));
    EXPECT_FALSE(bFailedExecution);

    return true;
}
```

### Command Registration Test

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FValidationCommandRegistrationTest,
    "DelveDeep.ConsoleCommands.ValidationCommandRegistration",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FValidationCommandRegistrationTest::RunTest(const FString& Parameters)
{
    TArray<FString> ExpectedCommands = {
        TEXT("DelveDeep.ValidateObject"),
        TEXT("DelveDeep.ListValidationRules"),
        TEXT("DelveDeep.ShowValidationMetrics")
    };

    TArray<FString> MissingCommands;
    bool bAllRegistered = DelveDeepTestUtils::VerifySubsystemCommandsRegistered(
        TEXT("Validation"),
        ExpectedCommands,
        MissingCommands);

    EXPECT_TRUE(bAllRegistered);

    return true;
}
```

### Output Capture Test

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCommandOutputCaptureTest,
    "DelveDeep.ConsoleCommands.OutputCapture",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCommandOutputCaptureTest::RunTest(const FString& Parameters)
{
    DelveDeepTestUtils::FConsoleOutputCapture Capture;

    bool bExecuted = DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.TestValidationSeverity"));
    EXPECT_TRUE(bExecuted);

    EXPECT_GT(Capture.GetOutputLineCount(), 0);
    EXPECT_TRUE(Capture.ContainsOutput(TEXT("Testing Validation Severity")));

    return true;
}
```

### Side Effects Test

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCommandSideEffectsTest,
    "DelveDeep.ConsoleCommands.SideEffects",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCommandSideEffectsTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    UDelveDeepValidationSubsystem* ValidationSubsystem = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepValidationSubsystem>(GameInstance);

    bool bVerified = DelveDeepTestUtils::VerifyConsoleCommandSideEffects(
        TEXT("DelveDeep.ResetValidationMetrics"),
        [ValidationSubsystem]() -> bool
        {
            FValidationMetricsData Metrics = ValidationSubsystem->GetValidationMetrics();
            return Metrics.TotalValidations == 0;
        });

    EXPECT_TRUE(bVerified);

    return true;
}
```

## Best Practices

### 1. Always Verify Command Registration

Test that commands are registered on subsystem initialization:

```cpp
TArray<FString> ExpectedCommands = { /* ... */ };
TArray<FString> MissingCommands;
bool bAllRegistered = DelveDeepTestUtils::VerifySubsystemCommandsRegistered(
    TEXT("MySubsystem"),
    ExpectedCommands,
    MissingCommands);
```

### 2. Test Invalid Parameter Handling

Ensure commands handle invalid inputs gracefully:

```cpp
TArray<FString> CapturedOutput;
TArray<FString> InvalidArgs;  // Empty or invalid
bool bHandledGracefully = DelveDeepTestUtils::TestConsoleCommandInvalidParameters(
    TEXT("MyCommand"),
    InvalidArgs,
    CapturedOutput);
```

### 3. Capture Output for Verification

Use output capture to verify command behavior:

```cpp
DelveDeepTestUtils::FConsoleOutputCapture Capture;
DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("MyCommand"));
EXPECT_TRUE(Capture.ContainsOutput(TEXT("Expected output")));
```

### 4. Verify Side Effects

Test that commands produce expected changes:

```cpp
bool bVerified = DelveDeepTestUtils::VerifyConsoleCommandSideEffects(
    TEXT("MyCommand"),
    []() -> bool
    {
        // Verify expected state changes
        return true;
    });
```

### 5. Test Command Discovery

Verify commands can be discovered by prefix:

```cpp
TArray<FString> Commands = DelveDeepTestUtils::GetRegisteredConsoleCommands(TEXT("MyPrefix."));
EXPECT_GT(Commands.Num(), 0);
```

## Common Patterns

### Testing Command Output Format

```cpp
DelveDeepTestUtils::FConsoleOutputCapture Capture;
DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.ShowMetrics"));

// Verify output structure
EXPECT_TRUE(Capture.ContainsOutput(TEXT("=== Metrics ===")));
EXPECT_TRUE(Capture.ContainsOutput(TEXT("Total:")));
EXPECT_TRUE(Capture.ContainsOutput(TEXT("Average:")));
```

### Testing Command Chains

```cpp
// Execute multiple commands in sequence
DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.ResetMetrics"));
DelveDeepTestUtils::ExecuteConsoleCommand(TEXT("DelveDeep.RunOperation"));

// Verify final state
bool bVerified = DelveDeepTestUtils::VerifyConsoleCommandSideEffects(
    TEXT("DelveDeep.ShowMetrics"),
    []() -> bool
    {
        // Verify metrics reflect the operation
        return true;
    });
```

### Testing Command Error Handling

```cpp
TArray<FString> CapturedOutput;
TArray<FString> InvalidArgs = { TEXT("InvalidValue") };

bool bHandled = DelveDeepTestUtils::TestConsoleCommandInvalidParameters(
    TEXT("DelveDeep.MyCommand"),
    InvalidArgs,
    CapturedOutput);

EXPECT_TRUE(bHandled);

// Verify error message format
bool bFoundError = false;
for (const FString& Line : CapturedOutput)
{
    if (Line.Contains(TEXT("Error:")) || Line.Contains(TEXT("Invalid")))
    {
        bFoundError = true;
        break;
    }
}
EXPECT_TRUE(bFoundError);
```

## Requirements Coverage

This implementation satisfies the following requirements from the automated testing framework specification:

### Requirement 15.1
✅ **Utilities for executing console commands programmatically**
- `ExecuteConsoleCommand()` - Basic command execution
- `ExecuteConsoleCommandWithArgs()` - Command execution with arguments
- `ExecuteConsoleCommandWithCapture()` - Command execution with output capture

### Requirement 15.2
✅ **Console output capture for verification**
- `FConsoleOutputCapture` - RAII-style output capture
- `ContainsOutput()` - Search for specific strings
- `ContainsAllOutput()` - Verify multiple expected strings
- `GetCapturedOutput()` - Access all captured lines

### Requirement 15.3
✅ **Helpers for testing command parameters**
- `TestConsoleCommandInvalidParameters()` - Test invalid parameter handling
- `ExecuteConsoleCommandWithArgs()` - Test with specific arguments
- Graceful error handling verification

### Requirement 15.4
✅ **Utilities for verifying command side effects**
- `VerifyConsoleCommandSideEffects()` - Custom verification function support
- Integration with subsystem testing
- State change verification

### Requirement 15.5
✅ **Test command registration on subsystem initialization**
- `IsConsoleCommandRegistered()` - Check individual command registration
- `GetRegisteredConsoleCommands()` - Discover commands by prefix
- `VerifySubsystemCommandsRegistered()` - Verify all expected commands

## See Also

- [DelveDeepTestUtilities.h](DelveDeepTestUtilities.h) - Full API reference
- [ConsoleCommandTests.cpp](ConsoleCommandTests.cpp) - Complete test examples
- [DelveDeepValidationCommands.cpp](../../DelveDeepValidationCommands.cpp) - Example command implementation
- [DelveDeepEventCommands.cpp](../../DelveDeepEventCommands.cpp) - Example command implementation
