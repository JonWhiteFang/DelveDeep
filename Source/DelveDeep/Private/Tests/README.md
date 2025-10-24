# DelveDeep Automated Testing Framework

This directory contains the automated testing infrastructure for DelveDeep, built on Unreal Engine's Automation System with Google Test-style assertion macros.

## Directory Structure

```
Tests/
├── README.md                      # This file
├── DelveDeepTestMacros.h         # Google Test-style assertion macros
├── DelveDeepTestUtilities.h      # Test helper functions
├── DelveDeepTestUtilities.cpp    # Test utilities implementation
├── DelveDeepAsyncTestCommands.h  # Async test support
├── DelveDeepTestFixtures.h       # Test fixtures
├── ConfigurationTests/           # Configuration system tests
├── EventTests/                   # Event system tests
├── TelemetryTests/               # Telemetry system tests
├── CombatTests/                  # Combat system tests (placeholder)
├── AITests/                      # AI system tests (placeholder)
└── IntegrationTests/             # Cross-system integration tests
```

## Test Organization

### ConfigurationTests/
Tests for the Data-Driven Configuration System:
- Configuration manager caching and performance
- Data asset validation
- Hot-reload functionality
- Query performance benchmarks

### EventTests/
Tests for the Centralized Event System:
- Event broadcasting and filtering
- GameplayTag hierarchy integration
- Priority-based listener execution
- Deferred event processing
- Performance benchmarks

### TelemetryTests/
Tests for the Performance Telemetry System:
- Frame performance tracking
- Memory tracking
- Budget validation
- Performance baseline comparison

### CombatTests/
Tests for the Combat System (placeholder for future implementation):
- Damage calculations
- Targeting system
- Combat performance

### AITests/
Tests for the AI System (placeholder for future implementation):
- Behavior tree execution
- AI perception
- Pathfinding

### IntegrationTests/
Cross-system integration tests:
- Configuration + Events integration
- Events + Telemetry integration
- Full system integration scenarios

## Writing Tests

### Simple Test Example

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMySimpleTest,
    "DelveDeep.Configuration.SimpleTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMySimpleTest::RunTest(const FString& Parameters)
{
    // Arrange
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    ASSERT_NOT_NULL(GameInstance);

    // Act
    UDelveDeepConfigurationManager* ConfigManager = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);

    // Assert
    EXPECT_NOT_NULL(ConfigManager);
    
    return true;
}
```

### Complex Test with Fixture

```cpp
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FMyComplexTest,
    "DelveDeep.Configuration.ComplexTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

class FMyTestFixture : public DelveDeepTestUtils::FSubsystemTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FSubsystemTestFixture::BeforeEach();
        ConfigManager = GetSubsystem<UDelveDeepConfigurationManager>();
    }

    UDelveDeepConfigurationManager* ConfigManager = nullptr;
};

bool FMyComplexTest::RunTest(const FString& Parameters)
{
    FMyTestFixture Fixture;
    Fixture.BeforeEach();

    // Test code here
    EXPECT_NOT_NULL(Fixture.ConfigManager);

    Fixture.AfterEach();
    return true;
}
```

### Performance Test Example

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyPerformanceTest,
    "DelveDeep.Performance.QueryTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FMyPerformanceTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    UDelveDeepConfigurationManager* ConfigManager = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);

    // Measure performance with multiple iterations
    DelveDeepTestUtils::FScopedTestTimer Timer("Query Performance");
    
    for (int32 i = 0; i < 1000; ++i)
    {
        ConfigManager->GetCharacterData("Warrior");
        Timer.RecordSample();
    }

    // Validate performance target
    EXPECT_TRUE(Timer.IsWithinBudget(1.0));  // <1ms average
    
    return true;
}
```

### Memory Test Example

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyMemoryTest,
    "DelveDeep.Memory.LeakTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyMemoryTest::RunTest(const FString& Parameters)
{
    DelveDeepTestUtils::FScopedMemoryTracker Tracker;
    
    {
        // Code that should not leak memory
        UDelveDeepCharacterData* Data = DelveDeepTestUtils::CreateTestCharacterData();
        // Use data...
    }
    
    // Validate no memory leak
    EXPECT_FALSE(Tracker.HasMemoryLeak());
    EXPECT_TRUE(Tracker.IsWithinBudget(1024 * 1024));  // <1MB
    
    return true;
}
```

### Validation Test Example

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyValidationTest,
    "DelveDeep.Validation.InvalidDataTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyValidationTest::RunTest(const FString& Parameters)
{
    // Create invalid test data
    UDelveDeepCharacterData* InvalidData = DelveDeepTestUtils::CreateInvalidCharacterData();
    
    // Test validation
    FValidationContext Context;
    bool bIsValid = DelveDeepTestUtils::TestExplicitValidation(InvalidData, Context);
    
    // Verify validation failed
    EXPECT_FALSE(bIsValid);
    EXPECT_HAS_ERRORS(Context);
    
    // Verify specific errors
    TArray<FString> ExpectedErrors = { TEXT("BaseHealth"), TEXT("negative") };
    EXPECT_TRUE(DelveDeepTestUtils::VerifyValidationErrors(Context, ExpectedErrors));
    
    return true;
}
```

## Available Assertion Macros

### Boolean Assertions
- `EXPECT_TRUE(Expression)` - Non-fatal true assertion
- `EXPECT_FALSE(Expression)` - Non-fatal false assertion
- `ASSERT_TRUE(Expression)` - Fatal true assertion (stops test)
- `ASSERT_FALSE(Expression)` - Fatal false assertion (stops test)

### Equality Assertions
- `EXPECT_EQ(Actual, Expected)` - Non-fatal equality
- `EXPECT_NE(Actual, Expected)` - Non-fatal inequality
- `ASSERT_EQ(Actual, Expected)` - Fatal equality
- `ASSERT_NE(Actual, Expected)` - Fatal inequality

### Comparison Assertions
- `EXPECT_LT(Actual, Expected)` - Less than
- `EXPECT_LE(Actual, Expected)` - Less than or equal
- `EXPECT_GT(Actual, Expected)` - Greater than
- `EXPECT_GE(Actual, Expected)` - Greater than or equal

### Null Pointer Assertions
- `EXPECT_NULL(Pointer)` - Non-fatal null check
- `EXPECT_NOT_NULL(Pointer)` - Non-fatal not-null check
- `ASSERT_NOT_NULL(Pointer)` - Fatal not-null check

### Floating Point Assertions
- `EXPECT_NEAR(Actual, Expected, Tolerance)` - Floating point comparison with tolerance

### String Assertions
- `EXPECT_STR_EQ(Actual, Expected)` - String equality
- `EXPECT_STR_CONTAINS(Haystack, Needle)` - String contains substring

### Array/Container Assertions
- `EXPECT_ARRAY_SIZE(Array, ExpectedSize)` - Array size check
- `EXPECT_ARRAY_CONTAINS(Array, Element)` - Array contains element

### Validation Context Assertions
- `EXPECT_VALID(Context)` - Validation context is valid
- `EXPECT_HAS_ERRORS(Context)` - Validation context has errors
- `EXPECT_NO_ERRORS(Context)` - Validation context has no errors
- `EXPECT_VALIDATION_SUCCESS(Context)` - Validation succeeded with detailed report

## Test Utilities

### UObject Creation
- `CreateTestObject<T>()` - Create test UObject
- `CreateTestObjectWithOuter<T>(Outer)` - Create with specific outer
- `CreateTestDataAsset<T>()` - Create test data asset

### Game Instance and Subsystems
- `CreateTestGameInstance()` - Create test game instance
- `GetTestSubsystem<T>(GameInstance)` - Get subsystem from game instance

### Mock Objects
- `FMockWorld::Create()` - Create mock world
- `FMockWorld::Destroy(World)` - Destroy mock world
- `FMockGameInstance::Create()` - Create mock game instance
- `FMockGameInstance::Destroy(GameInstance)` - Destroy mock game instance

### Test Data Generators
- `CreateTestCharacterData(Name, Health, Damage)` - Create test character data
- `CreateTestMonsterConfig(Name, Health, Damage)` - Create test monster config
- `CreateTestWeaponData(Name, Damage, AttackSpeed)` - Create test weapon data
- `CreateTestAbilityData(Name, Cooldown, ResourceCost)` - Create test ability data
- `CreateInvalidCharacterData()` - Create invalid character data for validation testing
- `CreateInvalidMonsterConfig()` - Create invalid monster config for validation testing

### Validation Helpers
- `ValidateTestObject(Object, Context)` - Validate test object
- `TestPostLoadValidation(DataAsset)` - Test PostLoad validation
- `TestExplicitValidation(Object, Context)` - Test explicit Validate() call
- `VerifyValidationErrors(Context, ExpectedErrors)` - Verify expected errors
- `VerifyValidationWarnings(Context, ExpectedWarnings)` - Verify expected warnings

### Performance Measurement
- `FScopedTestTimer(TestName)` - RAII timer for performance measurement
  - `GetElapsedMs()` - Get elapsed time in milliseconds
  - `GetElapsedUs()` - Get elapsed time in microseconds
  - `RecordSample()` - Record timing sample
  - `GetMinMs()` - Get minimum time
  - `GetMaxMs()` - Get maximum time
  - `GetAverageMs()` - Get average time
  - `GetMedianMs()` - Get median time
  - `IsWithinBudget(BudgetMs)` - Check if within performance budget
  - `GetSampleCount()` - Get number of samples

### Memory Tracking
- `FScopedMemoryTracker()` - RAII memory tracker
  - `GetAllocatedBytes()` - Get bytes allocated
  - `GetAllocationCount()` - Get allocation count
  - `HasMemoryLeak()` - Check for memory leak
  - `IsWithinBudget(BudgetBytes)` - Check if within memory budget
  - `GetPeakBytes()` - Get peak memory usage

### Resource Cleanup
- `FScopedTestCleanup(CleanupFunc)` - RAII cleanup helper

### Timeout Protection
- `FTestTimeoutGuard(TimeoutSeconds)` - Timeout guard for async operations
  - `IsTimedOut()` - Check if timed out

## Test Fixtures

### FDelveDeepTestFixture
Base test fixture with setup/teardown:
- `BeforeEach()` - Setup before each test
- `AfterEach()` - Cleanup after each test
- `CreateAndTrackObject<T>()` - Create and track object for automatic cleanup

### FSubsystemTestFixture
Fixture for subsystem testing:
- Inherits from `FDelveDeepTestFixture`
- Creates test game instance
- `GetSubsystem<T>()` - Get subsystem from game instance

### FIntegrationTestFixture
Fixture for integration testing:
- Inherits from `FSubsystemTestFixture`
- Initializes all major subsystems
- Provides references to ConfigManager, EventSubsystem, TelemetrySubsystem

## Running Tests

### From Command Line

```bash
# Run all tests
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Product" -unattended

# Run specific test suite
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunTests DelveDeep.Configuration" -unattended

# Run performance tests
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Perf" -unattended

# Run with report generation
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Product" -ReportOutputPath="TestResults" -unattended
```

### From Unreal Editor

1. Open Session Frontend (Window > Developer Tools > Session Frontend)
2. Go to Automation tab
3. Select tests to run
4. Click "Start Tests"

## Test Naming Conventions

Tests should follow this naming pattern:
- **Class Name**: `F[System][Feature]Test`
- **Test Path**: `DelveDeep.[System].[Feature]`
- **File Name**: `[System][Feature]Tests.cpp`

Examples:
- `FConfigurationCacheTest` in `DelveDeep.Configuration.Cache`
- `FEventFilteringTest` in `DelveDeep.Events.Filtering`
- `FTelemetryFrameTrackingTest` in `DelveDeep.Telemetry.FrameTracking`

## Test Flags

- `EAutomationTestFlags::ApplicationContextMask` - Required for all tests
- `EAutomationTestFlags::ProductFilter` - Unit tests (default)
- `EAutomationTestFlags::PerfFilter` - Performance tests
- `EAutomationTestFlags::EditorContext` - Editor-only tests
- `EAutomationTestFlags::ClientContext` - Runtime-only tests

## Best Practices

1. **Use fixtures for setup/teardown** - Avoid code duplication
2. **Test one thing per test** - Keep tests focused and simple
3. **Use descriptive test names** - Make failures easy to understand
4. **Clean up resources** - Use fixtures or RAII helpers
5. **Validate performance targets** - Use FScopedTestTimer
6. **Check for memory leaks** - Use FScopedMemoryTracker
7. **Test validation logic** - Use validation helpers
8. **Document complex tests** - Add comments explaining test logic
9. **Keep tests fast** - Target <30 seconds for full suite
10. **Run tests frequently** - Catch regressions early

## Performance Targets

- **Configuration System**: <1ms per query, >95% cache hit rate
- **Event System**: <1ms broadcast for 50 listeners, <0.1ms per listener
- **Telemetry System**: <0.1ms per frame tracking update
- **Full Test Suite**: <30 seconds total execution time

## Contributing

When adding new tests:
1. Place tests in the appropriate subdirectory
2. Follow naming conventions
3. Use existing test utilities and fixtures
4. Add performance and memory tests for critical paths
5. Document complex test scenarios
6. Update this README if adding new utilities or patterns

## References

- [Unreal Engine Automation System](https://docs.unrealengine.com/en-US/TestingAndOptimization/Automation/)
- [DelveDeep Configuration System](../../../Documentation/Systems/DataDrivenConfiguration.md)
- [DelveDeep Event System](../../../Documentation/Systems/CentralizedEventSystem.md)
- [DelveDeep Validation System](../../../Documentation/Systems/ValidationSystem.md)
