# Design Document

## Overview

The Automated Testing Framework enhances Unreal Engine's native Automation System with Google Test-style assertion macros, testing utilities, and organizational patterns. This design leverages UE5's proven testing infrastructure while providing a familiar API for developers, enabling comprehensive unit and integration testing without external dependencies.

The framework follows a layered architecture: low-level assertion macros wrapping Unreal's test functions, mid-level test utilities for common operations (UObject creation, subsystem testing, async operations), and high-level organizational patterns for test suites. It's designed for minimal overhead while providing maximum developer productivity.

## Architecture

### System Components

```
Automated Testing Framework
├── Assertion Macros (DelveDeepTestMacros.h)
│   ├── EXPECT_* macros (non-fatal assertions)
│   ├── ASSERT_* macros (fatal assertions)
│   └── Custom matchers (EXPECT_NEAR, EXPECT_CONTAINS)
├── Test Utilities (DelveDeepTestUtilities.h)
│   ├── UObject Creation Helpers
│   ├── Subsystem Test Helpers
│   ├── Mock Object Factories
│   └── Test Data Generators
├── Async Test Support (DelveDeepAsyncTestCommands.h)
│   ├── Latent Command Wrappers
│   ├── Timer Fast-Forward Utilities
│   └── Callback Verification Helpers
├── Test Fixtures (DelveDeepTestFixtures.h)
│   ├── Base Test Fixture
│   ├── Subsystem Test Fixture
│   └── Integration Test Fixture
├── Test Organization
│   ├── Source/DelveDeep/Private/Tests/
│   │   ├── ConfigurationTests/
│   │   ├── EventTests/
│   │   ├── TelemetryTests/
│   │   ├── CombatTests/
│   │   ├── AITests/
│   │   └── IntegrationTests/
└── CI/CD Integration
    ├── RunTests.sh (Unix)
    ├── RunTests.bat (Windows)
    └── Test Report Generation
```

### Test Execution Flow

```
1. Test Discovery
   ↓
2. Test Registration (IMPLEMENT_*_AUTOMATION_TEST)
   ↓
3. Test Execution
   ├── BeforeEach() [Complex tests only]
   ├── RunTest() [Test body]
   └── AfterEach() [Complex tests only]
   ↓
4. Result Collection
   ↓
5. Report Generation
```

## Components and Interfaces

### Assertion Macros

Google Test-style macros wrapping Unreal's test functions:

```cpp
// DelveDeepTestMacros.h

// Boolean assertions
#define EXPECT_TRUE(Expression) \
    TestTrue(TEXT(#Expression), Expression)

#define EXPECT_FALSE(Expression) \
    TestFalse(TEXT(#Expression), Expression)

#define ASSERT_TRUE(Expression) \
    if (!TestTrue(TEXT(#Expression), Expression)) { return false; }

#define ASSERT_FALSE(Expression) \
    if (!TestFalse(TEXT(#Expression), Expression)) { return false; }

// Equality assertions
#define EXPECT_EQ(Actual, Expected) \
    TestEqual(TEXT(#Actual " == " #Expected), Actual, Expected)

#define EXPECT_NE(Actual, Expected) \
    TestNotEqual(TEXT(#Actual " != " #Expected), Actual, Expected)

#define ASSERT_EQ(Actual, Expected) \
    if (!TestEqual(TEXT(#Actual " == " #Expected), Actual, Expected)) { return false; }

#define ASSERT_NE(Actual, Expected) \
    if (!TestNotEqual(TEXT(#Actual " != " #Expected), Actual, Expected)) { return false; }

// Comparison assertions
#define EXPECT_LT(Actual, Expected) \
    TestTrue(TEXT(#Actual " < " #Expected), Actual < Expected)

#define EXPECT_LE(Actual, Expected) \
    TestTrue(TEXT(#Actual " <= " #Expected), Actual <= Expected)

#define EXPECT_GT(Actual, Expected) \
    TestTrue(TEXT(#Actual " > " #Expected), Actual > Expected)

#define EXPECT_GE(Actual, Expected) \
    TestTrue(TEXT(#Actual " >= " #Expected), Actual >= Expected)

// Null pointer assertions
#define EXPECT_NULL(Pointer) \
    TestNull(TEXT(#Pointer " is null"), Pointer)

#define EXPECT_NOT_NULL(Pointer) \
    TestNotNull(TEXT(#Pointer " is not null"), Pointer)

#define ASSERT_NOT_NULL(Pointer) \
    if (!TestNotNull(TEXT(#Pointer " is not null"), Pointer)) { return false; }

// Floating point assertions
#define EXPECT_NEAR(Actual, Expected, Tolerance) \
    TestTrue(TEXT(#Actual " ~= " #Expected), \
        FMath::Abs(Actual - Expected) <= Tolerance)

// String assertions
#define EXPECT_STR_EQ(Actual, Expected) \
    TestEqual(TEXT(#Actual " == " #Expected), Actual, Expected)

#define EXPECT_STR_CONTAINS(Haystack, Needle) \
    TestTrue(TEXT(#Haystack " contains " #Needle), \
        Haystack.Contains(Needle))

// Array/Container assertions
#define EXPECT_ARRAY_SIZE(Array, ExpectedSize) \
    TestEqual(TEXT(#Array ".Num() == " #ExpectedSize), Array.Num(), ExpectedSize)

#define EXPECT_ARRAY_CONTAINS(Array, Element) \
    TestTrue(TEXT(#Array " contains " #Element), Array.Contains(Element))

// Validation context assertions
#define EXPECT_VALID(Context) \
    TestTrue(TEXT(#Context " is valid"), Context.IsValid())

#define EXPECT_HAS_ERRORS(Context) \
    TestTrue(TEXT(#Context " has errors"), Context.ValidationErrors.Num() > 0)

#define EXPECT_NO_ERRORS(Context) \
    TestTrue(TEXT(#Context " has no errors"), Context.ValidationErrors.Num() == 0)
```

### Test Utilities

Helper functions for common testing operations:

```cpp
// DelveDeepTestUtilities.h

namespace DelveDeepTestUtils
{
    // UObject creation helpers
    template<typename T>
    T* CreateTestObject()
    {
        return NewObject<T>();
    }

    template<typename T>
    T* CreateTestObjectWithOuter(UObject* Outer)
    {
        return NewObject<T>(Outer);
    }

    // Game instance creation for subsystem testing
    UGameInstance* CreateTestGameInstance();

    // Subsystem retrieval
    template<typename T>
    T* GetTestSubsystem(UGameInstance* GameInstance)
    {
        if (!GameInstance)
        {
            return nullptr;
        }
        return GameInstance->GetSubsystem<T>();
    }

    // Test data asset creation
    template<typename T>
    T* CreateTestDataAsset(const FString& AssetName)
    {
        T* DataAsset = NewObject<T>();
        // Set default test values
        return DataAsset;
    }

    // Mock object factories
    class FMockWorld
    {
    public:
        static UWorld* Create();
        static void Destroy(UWorld* World);
    };

    class FMockGameInstance
    {
    public:
        static UGameInstance* Create();
        static void Destroy(UGameInstance* GameInstance);
    };

    // Test data generators
    FDelveDeepCharacterData CreateTestCharacterData(
        const FString& Name,
        float Health = 100.0f,
        float Damage = 10.0f);

    FDelveDeepMonsterConfig CreateTestMonsterConfig(
        const FString& Name,
        float Health = 50.0f,
        float Damage = 5.0f);

    // Validation helpers
    bool ValidateTestObject(UObject* Object, FValidationContext& Context);

    // Performance measurement
    class FScopedTestTimer
    {
    public:
        FScopedTestTimer(const FString& TestName);
        ~FScopedTestTimer();
        double GetElapsedMs() const;

    private:
        FString Name;
        double StartTime;
    };

    // Memory tracking
    class FScopedMemoryTracker
    {
    public:
        FScopedMemoryTracker();
        ~FScopedMemoryTracker();
        uint64 GetAllocatedBytes() const;
        int32 GetAllocationCount() const;

    private:
        uint64 StartMemory;
        int32 StartAllocations;
    };
}
```

### Async Test Support

Utilities for testing asynchronous operations:

```cpp
// DelveDeepAsyncTestCommands.h

// Latent command for advancing time
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
    FAdvanceTimeCommand,
    float, DeltaTime);

bool FAdvanceTimeCommand::Update()
{
    // Advance world time
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().Tick(DeltaTime);
    }
    return true;
}

// Latent command for waiting on condition
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
    FWaitForConditionCommand,
    TFunction<bool()>, Condition,
    float, TimeoutSeconds);

bool FWaitForConditionCommand::Update()
{
    if (Condition())
    {
        return true;
    }

    ElapsedTime += GetWorld()->GetDeltaSeconds();
    if (ElapsedTime >= TimeoutSeconds)
    {
        UE_LOG(LogTemp, Error, TEXT("Condition timeout after %.2f seconds"), TimeoutSeconds);
        return true;
    }

    return false;
}

// Latent command for callback verification
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
    FVerifyCallbackCommand,
    TSharedPtr<bool>, bCallbackInvoked);

bool FVerifyCallbackCommand::Update()
{
    return *bCallbackInvoked;
}

// Helper macros for async testing
#define ADD_ADVANCE_TIME(DeltaTime) \
    ADD_LATENT_AUTOMATION_COMMAND(FAdvanceTimeCommand(DeltaTime))

#define ADD_WAIT_FOR_CONDITION(Condition, Timeout) \
    ADD_LATENT_AUTOMATION_COMMAND(FWaitForConditionCommand(Condition, Timeout))

#define ADD_VERIFY_CALLBACK(CallbackFlag) \
    ADD_LATENT_AUTOMATION_COMMAND(FVerifyCallbackCommand(CallbackFlag))
```

### Test Fixtures

Base classes for organizing tests with setup/teardown:

```cpp
// DelveDeepTestFixtures.h

// Base test fixture
class FDelveDeepTestFixture
{
public:
    virtual ~FDelveDeepTestFixture() = default;

    virtual void BeforeEach()
    {
        // Override in derived classes
    }

    virtual void AfterEach()
    {
        // Override in derived classes
        CleanupTestObjects();
    }

protected:
    void CleanupTestObjects()
    {
        for (UObject* Object : TestObjects)
        {
            if (IsValid(Object))
            {
                Object->ConditionalBeginDestroy();
            }
        }
        TestObjects.Empty();
    }

    template<typename T>
    T* CreateAndTrackObject()
    {
        T* Object = NewObject<T>();
        TestObjects.Add(Object);
        return Object;
    }

    TArray<UObject*> TestObjects;
};

// Subsystem test fixture
class FSubsystemTestFixture : public FDelveDeepTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FDelveDeepTestFixture::BeforeEach();
        GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    }

    virtual void AfterEach() override
    {
        if (GameInstance)
        {
            GameInstance->Shutdown();
            GameInstance = nullptr;
        }
        FDelveDeepTestFixture::AfterEach();
    }

protected:
    template<typename T>
    T* GetSubsystem()
    {
        return DelveDeepTestUtils::GetTestSubsystem<T>(GameInstance);
    }

    UGameInstance* GameInstance = nullptr;
};

// Integration test fixture
class FIntegrationTestFixture : public FSubsystemTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FSubsystemTestFixture::BeforeEach();
        
        // Initialize all subsystems
        ConfigManager = GetSubsystem<UDelveDeepConfigurationManager>();
        EventSubsystem = GetSubsystem<UDelveDeepEventSubsystem>();
        TelemetrySubsystem = GetSubsystem<UDelveDeepTelemetrySubsystem>();
    }

protected:
    UDelveDeepConfigurationManager* ConfigManager = nullptr;
    UDelveDeepEventSubsystem* EventSubsystem = nullptr;
    UDelveDeepTelemetrySubsystem* TelemetrySubsystem = nullptr;
};
```

### Test Organization

**Directory Structure:**

```
Source/DelveDeep/Private/Tests/
├── DelveDeepTestMacros.h           # Assertion macros
├── DelveDeepTestUtilities.h        # Test utilities
├── DelveDeepTestUtilities.cpp      # Test utilities implementation
├── DelveDeepAsyncTestCommands.h    # Async test support
├── DelveDeepTestFixtures.h         # Test fixtures
├── ConfigurationTests/
│   ├── ConfigurationManagerTests.cpp
│   ├── DataAssetValidationTests.cpp
│   └── CachePerformanceTests.cpp
├── EventTests/
│   ├── EventSubsystemTests.cpp
│   ├── EventFilteringTests.cpp
│   └── EventPerformanceTests.cpp
├── TelemetryTests/
│   ├── TelemetrySubsystemTests.cpp
│   ├── FrameTrackingTests.cpp
│   └── BudgetTrackingTests.cpp
├── CombatTests/
│   ├── DamageCalculationTests.cpp
│   ├── TargetingTests.cpp
│   └── CombatPerformanceTests.cpp
├── AITests/
│   ├── BehaviorTreeTests.cpp
│   ├── PerceptionTests.cpp
│   └── AIPerformanceTests.cpp
└── IntegrationTests/
    ├── ConfigurationEventIntegrationTests.cpp
    ├── EventTelemetryIntegrationTests.cpp
    └── FullSystemIntegrationTests.cpp
```

## Data Models

### Test Metadata

```cpp
// Test metadata for documentation generation
struct FTestMetadata
{
    FString TestName;
    FString TestSuite;
    FString Description;
    TArray<FString> Requirements;  // Requirement IDs this test verifies
    TArray<FString> Tags;          // Tags for filtering (unit, integration, performance)
    float ExpectedDuration;        // Expected execution time in seconds
};
```

### Test Result

```cpp
// Extended test result information
struct FDelveDeepTestResult
{
    FString TestName;
    bool bPassed;
    float ExecutionTime;
    TArray<FString> Errors;
    TArray<FString> Warnings;
    FDateTime ExecutionTimestamp;
    
    // Performance metrics
    uint64 MemoryAllocated;
    int32 AllocationCount;
    
    // Coverage information
    TArray<FString> CoveredFiles;
    float CodeCoveragePercentage;
};
```

### Test Report

```cpp
// Comprehensive test report
struct FDelveDeepTestReport
{
    FDateTime GenerationTime;
    FString BuildVersion;
    int32 TotalTests;
    int32 PassedTests;
    int32 FailedTests;
    int32 SkippedTests;
    float TotalExecutionTime;
    
    TArray<FDelveDeepTestResult> Results;
    TMap<FString, int32> TestsBySuite;
    TMap<FString, float> ExecutionTimeBySuite;
    
    // Coverage summary
    float OverallCodeCoverage;
    TMap<FString, float> CoverageBySystem;
    
    // Performance summary
    uint64 TotalMemoryAllocated;
    int32 TotalAllocations;
};
```

## Error Handling

### Test Failure Handling

```cpp
// Graceful test failure with detailed context
#define EXPECT_WITH_CONTEXT(Expression, Context) \
    if (!TestTrue(TEXT(#Expression), Expression)) \
    { \
        AddError(FString::Printf(TEXT("Context: %s"), *Context)); \
    }

// Validation-aware assertions
#define EXPECT_VALIDATION_SUCCESS(Context) \
    if (!Context.IsValid()) \
    { \
        AddError(FString::Printf(TEXT("Validation failed: %s"), *Context.GetReport())); \
    }
```

### Timeout Protection

```cpp
// Automatic timeout for async tests
class FTestTimeoutGuard
{
public:
    FTestTimeoutGuard(float TimeoutSeconds);
    ~FTestTimeoutGuard();
    bool IsTimedOut() const;

private:
    double StartTime;
    float Timeout;
};
```

### Resource Cleanup

```cpp
// RAII-style cleanup for test resources
class FScopedTestCleanup
{
public:
    FScopedTestCleanup(TFunction<void()> CleanupFunc);
    ~FScopedTestCleanup();

private:
    TFunction<void()> Cleanup;
};
```

## Testing Strategy

### Unit Tests

**Configuration System Tests:**
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FConfigurationManagerCacheTest,
    "DelveDeep.Configuration.CacheHitRate",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerCacheTest::RunTest(const FString& Parameters)
{
    // Create test game instance
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    ASSERT_NOT_NULL(GameInstance);

    // Get configuration manager
    UDelveDeepConfigurationManager* ConfigManager = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);
    ASSERT_NOT_NULL(ConfigManager);

    // Test cache hit rate
    const UDelveDeepCharacterData* Data1 = ConfigManager->GetCharacterData("Warrior");
    const UDelveDeepCharacterData* Data2 = ConfigManager->GetCharacterData("Warrior");
    
    EXPECT_EQ(Data1, Data2);  // Should return same cached instance
    EXPECT_GT(ConfigManager->GetCacheHitRate(), 0.5f);

    return true;
}
```

**Event System Tests:**
```cpp
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FEventFilteringTest,
    "DelveDeep.Events.Filtering",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

class FEventFilteringTestFixture : public FSubsystemTestFixture
{
public:
    virtual void BeforeEach() override
    {
        FSubsystemTestFixture::BeforeEach();
        EventSubsystem = GetSubsystem<UDelveDeepEventSubsystem>();
        bCallbackInvoked = false;
    }

    UDelveDeepEventSubsystem* EventSubsystem = nullptr;
    bool bCallbackInvoked = false;
};

bool FEventFilteringTest::RunTest(const FString& Parameters)
{
    FEventFilteringTestFixture Fixture;
    Fixture.BeforeEach();

    // Register filtered listener
    FGameplayTag CombatTag = FGameplayTag::RequestGameplayTag("DelveDeep.Combat");
    Fixture.EventSubsystem->RegisterListener(
        CombatTag,
        FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
        {
            Fixture.bCallbackInvoked = true;
        }));

    // Broadcast matching event
    FDelveDeepEventPayload Payload;
    Payload.EventTag = CombatTag;
    Fixture.EventSubsystem->BroadcastEvent(Payload);

    EXPECT_TRUE(Fixture.bCallbackInvoked);

    Fixture.AfterEach();
    return true;
}
```

### Integration Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FConfigurationEventIntegrationTest,
    "DelveDeep.Integration.ConfigurationEvents",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationEventIntegrationTest::RunTest(const FString& Parameters)
{
    FIntegrationTestFixture Fixture;
    Fixture.BeforeEach();

    // Verify subsystems are initialized
    ASSERT_NOT_NULL(Fixture.ConfigManager);
    ASSERT_NOT_NULL(Fixture.EventSubsystem);

    // Test cross-system interaction
    bool bEventReceived = false;
    Fixture.EventSubsystem->RegisterListener(
        FGameplayTag::RequestGameplayTag("DelveDeep.Configuration.Loaded"),
        FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
        {
            bEventReceived = true;
        }));

    // Trigger configuration load
    Fixture.ConfigManager->ReloadAllData();

    EXPECT_TRUE(bEventReceived);

    Fixture.AfterEach();
    return true;
}
```

### Performance Tests

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FConfigurationQueryPerformanceTest,
    "DelveDeep.Performance.ConfigurationQuery",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::PerfFilter)

bool FConfigurationQueryPerformanceTest::RunTest(const FString& Parameters)
{
    UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
    UDelveDeepConfigurationManager* ConfigManager = 
        DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);

    // Measure query performance
    DelveDeepTestUtils::FScopedTestTimer Timer("Configuration Query");
    
    for (int32 i = 0; i < 1000; ++i)
    {
        ConfigManager->GetCharacterData("Warrior");
    }

    double ElapsedMs = Timer.GetElapsedMs();
    double AvgQueryMs = ElapsedMs / 1000.0;

    EXPECT_LT(AvgQueryMs, 1.0);  // Target: <1ms per query

    return true;
}
```

## Performance Considerations

### Test Execution Optimization

**Parallel Test Execution:**
- Tests marked with `EAutomationTestFlags::ProductFilter` can run in parallel
- Use `EAutomationTestFlags::EditorContext` for tests requiring editor
- Use `EAutomationTestFlags::ClientContext` for runtime-only tests

**Test Filtering:**
```bash
# Run only unit tests
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunFilter Product" -unattended

# Run specific test suite
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep.Configuration" -unattended

# Run performance tests
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunFilter Perf" -unattended
```

### Memory Management

**Test Object Cleanup:**
- Use `FDelveDeepTestFixture::CleanupTestObjects()` for automatic cleanup
- Track all created UObjects in fixture
- Call `ConditionalBeginDestroy()` in AfterEach()

**Memory Leak Detection:**
```cpp
DelveDeepTestUtils::FScopedMemoryTracker Tracker;
// ... test code ...
uint64 LeakedBytes = Tracker.GetAllocatedBytes();
EXPECT_EQ(LeakedBytes, 0);
```

## CI/CD Integration

### Test Execution Scripts

**RunTests.sh (Unix):**
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

**RunTests.bat (Windows):**
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

### Report Generation

**Test Report Parser:**
```cpp
class FTestReportGenerator
{
public:
    static FDelveDeepTestReport GenerateReport(const FString& ReportPath);
    static bool ExportToMarkdown(const FDelveDeepTestReport& Report, const FString& OutputPath);
    static bool ExportToHTML(const FDelveDeepTestReport& Report, const FString& OutputPath);
    static bool ExportToJUnit(const FDelveDeepTestReport& Report, const FString& OutputPath);
};
```

## Blueprint Integration

### Blueprint Test Utilities

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepTestBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Testing")
    static bool RunTestSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Testing")
    static FDelveDeepTestReport GetLastTestReport();

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Testing")
    static bool IsTestEnvironment();
};
```

## Future Extensibility

### Code Coverage Integration

```cpp
class FCodeCoverageTracker
{
public:
    void StartTracking();
    void StopTracking();
    FCodeCoverageReport GenerateReport();
    bool ExportToHTML(const FString& OutputPath);
    bool ExportToXML(const FString& OutputPath);  // Cobertura format
};
```

### Visual Test Runner

```cpp
// Future: In-editor test runner UI
class SDelveDeepTestRunner : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDelveDeepTestRunner) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void RefreshTestList();
    void RunSelectedTests();
    void ShowTestResults();
};
```

### Automated Regression Detection

```cpp
class FRegressionDetector
{
public:
    bool CompareTestResults(
        const FDelveDeepTestReport& Baseline,
        const FDelveDeepTestReport& Current);
    
    TArray<FRegressionReport> GetRegressions() const;
};
```

## Dependencies

- **Unreal Engine Automation System**: Core testing infrastructure
- **FValidationContext**: From Enhanced Validation System
- **UDelveDeepConfigurationManager**: For configuration testing
- **UDelveDeepEventSubsystem**: For event testing
- **UDelveDeepTelemetrySubsystem**: For performance testing
- **No external dependencies**: Pure UE5 integration
