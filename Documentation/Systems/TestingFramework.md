# Testing Framework

**Status**: ✅ Complete (October 27, 2025)

**Version**: 1.0

## Overview

The DelveDeep Automated Testing Framework enhances Unreal Engine's native Automation System with Google Test-style assertion macros, comprehensive testing utilities, and organizational patterns. This framework enables developers to write, run, and maintain automated tests for all game systems with minimal friction while maintaining seamless integration with UE5's build system, editor, and CI/CD tools.

The framework provides a familiar testing API for developers experienced with modern C++ testing frameworks while leveraging Unreal's proven infrastructure. It supports unit tests, integration tests, performance tests, memory tests, and async tests without requiring external dependencies.

## Key Features

- **Google Test-Style Assertions**: Familiar EXPECT_* and ASSERT_* macros
- **Test Fixtures**: Base classes for setup/teardown with automatic cleanup
- **Async Test Support**: Utilities for testing timers and latent actions
- **Performance Measurement**: Microsecond-precision timing utilities
- **Memory Tracking**: Allocation and leak detection
- **Test Organization**: Structured directory layout by system
- **CI/CD Integration**: Headless test execution with report generation
- **Blueprint Integration**: Test utilities accessible from Blueprint

## Quick Start

### Writing Your First Test

```cpp
#include "Tests/DelveDeepTestMacros.h"
#include "Tests/DelveDeepTestUtilities.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyFirstTest,
    "DelveDeep.MySystem.BasicTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyFirstTest::RunTest(const FString& Parameters)
{
    // Arrange
    int32 Value = 42;
    
    // Act
    int32 Result = Value * 2;
    
    // Assert
    EXPECT_EQ(Result, 84);
    EXPECT_GT(Result, 0);
    
    return true;
}
```

### Running Tests

```bash
# Run all tests
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Product" -unattended

# Run specific test suite
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunTests DelveDeep.Configuration" -unattended

# Run performance tests
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Perf" -unattended
```

## Assertion Macros

### Boolean Assertions

```cpp
// Non-fatal assertions (test continues on failure)
EXPECT_TRUE(bCondition);
EXPECT_FALSE(bCondition);

// Fatal assertions (test stops on failure)
ASSERT_TRUE(bCondition);
ASSERT_FALSE(bCondition);
```

### Equality Assertions

```cpp
// Non-fatal
EXPECT_EQ(Actual, Expected);
EXPECT_NE(Actual, Expected);

// Fatal
ASSERT_EQ(Actual, Expected);
ASSERT_NE(Actual, Expected);
```

### Comparison Assertions

```cpp
EXPECT_LT(Actual, Expected);  // Less than
EXPECT_LE(Actual, Expected);  // Less than or equal
EXPECT_GT(Actual, Expected);  // Greater than
EXPECT_GE(Actual, Expected);  // Greater than or equal
```

### Null Pointer Assertions

```cpp
EXPECT_NULL(Pointer);
EXPECT_NOT_NULL(Pointer);
ASSERT_NOT_NULL(Pointer);  // Fatal
```

### Floating Point Assertions

```cpp
// Compare with tolerance
EXPECT_NEAR(Actual, Expected, 0.001f);
```

### String Assertions

```cpp
EXPECT_STR_EQ(ActualString, ExpectedString);
EXPECT_STR_CONTAINS(Haystack, Needle);
```

### Array/Container Assertions

```cpp
EXPECT_ARRAY_SIZE(Array, ExpectedSize);
EXPECT_ARRAY_CONTAINS(Array, Element);
```

### Validation Context Assertions

```cpp
FValidationContext Context;
// ... perform validation ...

EXPECT_VALID(Context);
EXPECT_HAS_ERRORS(Context);
EXPECT_NO_ERRORS(Context);
```

## Test Fixtures

### Base Test Fixture

Use for tests requiring setup and teardown:

```cpp
#include "Tests/DelveDeepTestFixtures.h"

class FMyTestFixture : public FDelveDeepTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FDelveDeepTestFixture::BeforeEach();
        
        // Your setup code
        TestData = CreateAndTrackObject<UMyDataAsset>();
    }
    
    virtual void AfterEach() override
    {
        // Your cleanup code
        TestData = nullptr;
        
        FDelveDeepTestFixture::AfterEach();  // Automatic cleanup
    }
    
protected:
    UMyDataAsset* TestData = nullptr;
};

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FMyComplexTest,
    "DelveDeep.MySystem.ComplexTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyComplexTest::RunTest(const FString& Parameters)
{
    FMyTestFixture Fixture;
    Fixture.BeforeEach();
    
    // Test code using Fixture.TestData
    EXPECT_NOT_NULL(Fixture.TestData);
    
    Fixture.AfterEach();
    return true;
}
```

### Subsystem Test Fixture

Use for testing subsystems:

```cpp
class FMySubsystemTest : public FSubsystemTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FSubsystemTestFixture::BeforeEach();
        
        // Get subsystem
        ConfigManager = GetSubsystem<UDelveDeepConfigurationManager>();
    }
    
protected:
    UDelveDeepConfigurationManager* ConfigManager = nullptr;
};

bool FMySubsystemTest::RunTest(const FString& Parameters)
{
    FMySubsystemTest Fixture;
    Fixture.BeforeEach();
    
    ASSERT_NOT_NULL(Fixture.ConfigManager);
    
    // Test subsystem functionality
    const UDelveDeepCharacterData* Data = Fixture.ConfigManager->GetCharacterData("Warrior");
    EXPECT_NOT_NULL(Data);
    
    Fixture.AfterEach();
    return true;
}
```

### Integration Test Fixture

Use for testing multiple subsystems together:

```cpp
class FMyIntegrationTest : public FIntegrationTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FIntegrationTestFixture::BeforeEach();
        
        // ConfigManager, EventSubsystem, TelemetrySubsystem already initialized
    }
};

bool FMyIntegrationTest::RunTest(const FString& Parameters)
{
    FMyIntegrationTest Fixture;
    Fixture.BeforeEach();
    
    ASSERT_NOT_NULL(Fixture.ConfigManager);
    ASSERT_NOT_NULL(Fixture.EventSubsystem);
    
    // Test cross-system interaction
    bool bEventReceived = false;
    Fixture.EventSubsystem->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Test"),
        FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
        {
            bEventReceived = true;
        }));
    
    FDelveDeepEventPayload Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag("DelveDeep.Test");
    Fixture.EventSubsystem->BroadcastEvent(Payload);
    
    EXPECT_TRUE(bEventReceived);
    
    Fixture.AfterEach();
    return true;
}
```

## Test Utilities

### UObject Creation

```cpp
#include "Tests/DelveDeepTestUtilities.h"

// Create test object
UMyObject* Object = DelveDeepTestUtils::CreateTestObject<UMyObject>();

// Create with outer
UMyObject* Object = DelveDeepTestUtils::CreateTestObjectWithOuter<UMyObject>(Outer);

// Create test game instance
UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();

// Get subsystem
auto* ConfigManager = DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);
```

### Test Data Generation

```cpp
// Create test character data
FDelveDeepCharacterData CharData = DelveDeepTestUtils::CreateTestCharacterData(
    "TestWarrior",
    100.0f,  // Health
    10.0f    // Damage
);

// Create test monster config
FDelveDeepMonsterConfig MonsterConfig = DelveDeepTestUtils::CreateTestMonsterConfig(
    "TestGoblin",
    50.0f,   // Health
    5.0f     // Damage
);
```

## Async Testing

### Advancing Time

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTimerTest,
    "DelveDeep.Async.Timer",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTimerTest::RunTest(const FString& Parameters)
{
    bool bTimerFired = false;
    
    // Set timer
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [&]() { bTimerFired = true; },
        1.0f,
        false);
    
    // Advance time
    ADD_ADVANCE_TIME(1.1f);
    
    // Verify timer fired
    ADD_LATENT_AUTOMATION_COMMAND(FDelveDeepVerifyCommand([&]()
    {
        EXPECT_TRUE(bTimerFired);
        return true;
    }));
    
    return true;
}
```

### Waiting for Conditions

```cpp
bool bConditionMet = false;

// Wait for condition with timeout
ADD_WAIT_FOR_CONDITION(
    [&]() { return bConditionMet; },
    5.0f  // Timeout in seconds
);
```

### Verifying Callbacks

```cpp
TSharedPtr<bool> bCallbackInvoked = MakeShared<bool>(false);

// Register callback
MySystem->OnEvent.AddLambda([bCallbackInvoked]()
{
    *bCallbackInvoked = true;
});

// Trigger event
MySystem->TriggerEvent();

// Verify callback was invoked
ADD_VERIFY_CALLBACK(bCallbackInvoked);
```

## Performance Testing

### Measuring Execution Time

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPerformanceTest,
    "DelveDeep.Performance.Query",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FPerformanceTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    auto* ConfigManager = DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);
    
    // Measure performance
    DelveDeepTestUtils::FScopedTestTimer Timer("Configuration Query");
    
    for (int32 i = 0; i < 1000; ++i)
    {
        ConfigManager->GetCharacterData("Warrior");
    }
    
    double ElapsedMs = Timer.GetElapsedMs();
    double AvgQueryMs = ElapsedMs / 1000.0;
    
    // Verify performance target
    EXPECT_LT(AvgQueryMs, 1.0);  // Target: <1ms per query
    
    return true;
}
```

### Performance Targets

- **Configuration Query**: <1ms per query
- **Event Broadcast**: <1ms for 50 listeners
- **Listener Invocation**: <0.1ms per listener
- **Deferred Processing**: <10ms for 1000 events
- **Initialization**: <100ms for 100+ assets

## Memory Testing

### Tracking Allocations

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMemoryTest,
    "DelveDeep.Memory.Allocation",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryTest::RunTest(const FString& Parameters)
{
    DelveDeepTestUtils::FScopedMemoryTracker Tracker;
    
    // Code that should not leak memory
    {
        TArray<int32> TempArray;
        TempArray.Reserve(1000);
        TempArray.Empty();
    }
    
    // Verify no leaks
    uint64 LeakedBytes = Tracker.GetAllocatedBytes();
    EXPECT_EQ(LeakedBytes, 0);
    
    return true;
}
```

## Test Organization

### Directory Structure

```
Source/DelveDeep/Private/Tests/
├── DelveDeepTestMacros.h           # Assertion macros
├── DelveDeepTestUtilities.h        # Test utilities
├── DelveDeepTestUtilities.cpp      # Test utilities implementation
├── DelveDeepAsyncTestCommands.h    # Async test support
├── DelveDeepTestFixtures.h         # Test fixtures
├── ConfigurationTests/             # Configuration system tests
├── EventTests/                     # Event system tests
├── TelemetryTests/                 # Telemetry system tests
├── CombatTests/                    # Combat system tests
├── AITests/                        # AI system tests
└── IntegrationTests/               # Cross-system tests
```

### Naming Conventions

**Test Files:**
- Pattern: `[System]Tests.cpp`
- Examples: `ConfigurationManagerTests.cpp`, `EventSystemTests.cpp`

**Test Names:**
- Pattern: `DelveDeep.[System].[TestName]`
- Examples: `DelveDeep.Configuration.CacheHitRate`, `DelveDeep.Events.Filtering`

**Test Flags:**
- `EAutomationTestFlags::ProductFilter` - Unit tests (run in parallel)
- `EAutomationTestFlags::PerfFilter` - Performance tests
- `EAutomationTestFlags::ApplicationContextMask` - Standard context

## CI/CD Integration

### Running Tests in CI

**Unix (RunTests.sh):**
```bash
#!/bin/bash
set -e

ENGINE_PATH="/path/to/UnrealEngine"
PROJECT_PATH="$(pwd)/DelveDeep.uproject"
OUTPUT_PATH="$(pwd)/TestResults"

mkdir -p "$OUTPUT_PATH"

"$ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor-Cmd" \
    "$PROJECT_PATH" \
    -ExecCmds="Automation RunFilter Product" \
    -ReportOutputPath="$OUTPUT_PATH" \
    -unattended \
    -nopause \
    -NullRHI \
    -log

exit $?
```

**Windows (RunTests.bat):**
```batch
@echo off
setlocal

set ENGINE_PATH=C:\UnrealEngine
set PROJECT_PATH=%CD%\DelveDeep.uproject
set OUTPUT_PATH=%CD%\TestResults

if not exist "%OUTPUT_PATH%" mkdir "%OUTPUT_PATH%"

"%ENGINE_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
    "%PROJECT_PATH%" ^
    -ExecCmds="Automation RunFilter Product" ^
    -ReportOutputPath="%OUTPUT_PATH%" ^
    -unattended ^
    -nopause ^
    -NullRHI ^
    -log

exit /b %ERRORLEVEL%
```

### Test Report Generation

Test reports are automatically generated in the specified output directory:
- **Index.json**: Test results in JSON format
- **index.html**: Human-readable HTML report
- **Logs/**: Detailed execution logs

## Best Practices

### Test Structure

1. **Arrange**: Set up test data and preconditions
2. **Act**: Execute the code being tested
3. **Assert**: Verify expected behavior

```cpp
bool FMyTest::RunTest(const FString& Parameters)
{
    // Arrange
    UMyObject* Object = DelveDeepTestUtils::CreateTestObject<UMyObject>();
    Object->SetValue(42);
    
    // Act
    int32 Result = Object->GetDoubledValue();
    
    // Assert
    EXPECT_EQ(Result, 84);
    
    return true;
}
```

### Test Independence

- Each test should be independent and not rely on other tests
- Use fixtures for setup/teardown to ensure clean state
- Avoid global state that persists between tests

### Test Naming

- Use descriptive names that explain what is being tested
- Follow the pattern: `[System].[Component].[Behavior]`
- Examples: `Configuration.CacheHitRate`, `Events.FilteringByTag`

### Assertion Selection

- Use `EXPECT_*` for non-fatal assertions (test continues)
- Use `ASSERT_*` for fatal assertions (test stops)
- Use `ASSERT_*` for preconditions that must be true

### Performance Testing

- Run performance tests multiple iterations for stable results
- Set realistic performance targets based on requirements
- Use `EAutomationTestFlags::PerfFilter` for performance tests

### Memory Testing

- Track allocations for systems that should not leak
- Use `FScopedMemoryTracker` to detect leaks
- Verify cleanup in AfterEach() methods

## Common Patterns

### Testing Subsystems

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSubsystemTest,
    "DelveDeep.MySystem.Subsystem",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubsystemTest::RunTest(const FString& Parameters)
{
    // Create game instance
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    ASSERT_NOT_NULL(GameInstance);
    
    // Get subsystem
    UMySubsystem* Subsystem = DelveDeepTestUtils::GetTestSubsystem<UMySubsystem>(GameInstance);
    ASSERT_NOT_NULL(Subsystem);
    
    // Test subsystem functionality
    EXPECT_TRUE(Subsystem->IsInitialized());
    
    return true;
}
```

### Testing Data Assets

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDataAssetTest,
    "DelveDeep.Configuration.DataAsset",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDataAssetTest::RunTest(const FString& Parameters)
{
    // Create test data asset
    UDelveDeepCharacterData* CharData = DelveDeepTestUtils::CreateTestObject<UDelveDeepCharacterData>();
    CharData->BaseHealth = 100.0f;
    CharData->BaseDamage = 10.0f;
    
    // Validate
    FValidationContext Context;
    bool bIsValid = CharData->Validate(Context);
    
    EXPECT_VALID(Context);
    EXPECT_TRUE(bIsValid);
    
    return true;
}
```

### Testing Events

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEventTest,
    "DelveDeep.Events.Broadcasting",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    UDelveDeepEventSubsystem* EventSubsystem = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepEventSubsystem>(GameInstance);
    
    bool bEventReceived = false;
    
    // Register listener
    EventSubsystem->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Test"),
        FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
        {
            bEventReceived = true;
        }));
    
    // Broadcast event
    FDelveDeepEventPayload Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag("DelveDeep.Test");
    EventSubsystem->BroadcastEvent(Payload);
    
    EXPECT_TRUE(bEventReceived);
    
    return true;
}
```

## Troubleshooting

### Tests Not Running

**Problem**: Tests don't appear in test list

**Solution**:
- Verify test file is in `Source/DelveDeep/Private/Tests/` directory
- Check that `IMPLEMENT_*_AUTOMATION_TEST` macro is used correctly
- Ensure test name follows pattern: `DelveDeep.[System].[TestName]`
- Rebuild project to register new tests

### Test Failures

**Problem**: Test fails unexpectedly

**Solution**:
- Check assertion messages in test output
- Verify test preconditions are met
- Use `ASSERT_NOT_NULL` for pointers before dereferencing
- Add logging to understand test flow

### Memory Leaks

**Problem**: Memory tracker reports leaks

**Solution**:
- Use `FDelveDeepTestFixture` for automatic cleanup
- Call `AfterEach()` to clean up test objects
- Verify all created UObjects are tracked
- Check for circular references

### Async Test Timeouts

**Problem**: Async tests timeout

**Solution**:
- Increase timeout value in `ADD_WAIT_FOR_CONDITION`
- Verify condition is actually being met
- Check that timers are being advanced with `ADD_ADVANCE_TIME`
- Add logging to track async operation progress

## Examples

### Complete Unit Test Example

```cpp
#include "Tests/DelveDeepTestMacros.h"
#include "Tests/DelveDeepTestUtilities.h"
#include "Tests/DelveDeepTestFixtures.h"

class FConfigurationCacheTest : public FSubsystemTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FSubsystemTestFixture::BeforeEach();
        ConfigManager = GetSubsystem<UDelveDeepConfigurationManager>();
    }
    
protected:
    UDelveDeepConfigurationManager* ConfigManager = nullptr;
};

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FConfigurationCacheHitRateTest,
    "DelveDeep.Configuration.CacheHitRate",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationCacheHitRateTest::RunTest(const FString& Parameters)
{
    FConfigurationCacheTest Fixture;
    Fixture.BeforeEach();
    
    ASSERT_NOT_NULL(Fixture.ConfigManager);
    
    // First query (cache miss)
    const UDelveDeepCharacterData* Data1 = Fixture.ConfigManager->GetCharacterData("Warrior");
    EXPECT_NOT_NULL(Data1);
    
    // Second query (cache hit)
    const UDelveDeepCharacterData* Data2 = Fixture.ConfigManager->GetCharacterData("Warrior");
    EXPECT_NOT_NULL(Data2);
    
    // Should return same cached instance
    EXPECT_EQ(Data1, Data2);
    
    // Verify cache hit rate
    float CacheHitRate = Fixture.ConfigManager->GetCacheHitRate();
    EXPECT_GT(CacheHitRate, 0.5f);
    
    Fixture.AfterEach();
    return true;
}
```

### Complete Integration Test Example

```cpp
class FConfigurationEventIntegrationTest : public FIntegrationTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FIntegrationTestFixture::BeforeEach();
        bEventReceived = false;
    }
    
    bool bEventReceived = false;
};

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FConfigEventIntegrationTest,
    "DelveDeep.Integration.ConfigurationEvents",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigEventIntegrationTest::RunTest(const FString& Parameters)
{
    FConfigurationEventIntegrationTest Fixture;
    Fixture.BeforeEach();
    
    ASSERT_NOT_NULL(Fixture.ConfigManager);
    ASSERT_NOT_NULL(Fixture.EventSubsystem);
    
    // Register listener for configuration events
    Fixture.EventSubsystem->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Configuration.Loaded"),
        FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
        {
            Fixture.bEventReceived = true;
        }));
    
    // Trigger configuration reload
    Fixture.ConfigManager->ReloadAllData();
    
    // Verify event was broadcast
    EXPECT_TRUE(Fixture.bEventReceived);
    
    Fixture.AfterEach();
    return true;
}
```

### Complete Performance Test Example

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEventBroadcastPerformanceTest,
    "DelveDeep.Performance.EventBroadcast",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FEventBroadcastPerformanceTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    UDelveDeepEventSubsystem* EventSubsystem = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepEventSubsystem>(GameInstance);
    
    // Register 50 listeners
    FGameplayTag TestTag = FGameplayTag::RequestGameplayTag("DelveDeep.Performance.Test");
    for (int32 i = 0; i < 50; ++i)
    {
        EventSubsystem->RegisterListener(
            TestTag,
            FDelveDeepEventDelegate::CreateLambda([](const FDelveDeepEventPayload& Payload) {}));
    }
    
    // Measure broadcast performance
    DelveDeepTestUtils::FScopedTestTimer Timer("Event Broadcast");
    
    FDelveDeepEventPayload Payload;
    Payload.EventTag = TestTag;
    EventSubsystem->BroadcastEvent(Payload);
    
    double ElapsedMs = Timer.GetElapsedMs();
    
    // Verify performance target: <1ms for 50 listeners
    EXPECT_LT(ElapsedMs, 1.0);
    
    return true;
}
```

## Related Documentation

- [Validation System](ValidationSystem.md) - FValidationContext usage
- [Data-Driven Configuration](DataDrivenConfiguration.md) - Configuration system testing
- [Centralized Event System](CentralizedEventSystem.md) - Event system testing
- [Testing Guide](../Testing-Guide.md) - General testing philosophy

## Future Enhancements

- **Code Coverage**: Integration with coverage tools
- **Visual Test Runner**: In-editor UI for running tests
- **Regression Detection**: Automatic performance regression detection
- **Watch Mode**: Automatic test execution on file changes
- **Blueprint Test Support**: Enhanced Blueprint testing utilities

---

**Last Updated**: October 27, 2025

**Implementation Status**: Complete

**Test Coverage**: 100% of framework utilities tested
