# Testing Guidelines

**✓ Reference Implementation**: DelveDeep Configuration System includes comprehensive test suite.

## Testing Philosophy

- **DO NOT** automatically add tests unless explicitly requested by the user
- Tests should be written for core systems and critical functionality
- Focus on unit tests for isolated components and integration tests for system interactions
- Performance tests should validate target metrics (sub-millisecond queries, sub-100ms initialization)

## Unit Testing Framework

Use Google Test or Catch2 for C++ unit tests:

```cpp
// Example unit test structure
TEST(DelveDeepConfig, CharacterDataValidation)
{
    // Arrange
    UDelveDeepCharacterData* CharacterData = NewObject<UDelveDeepCharacterData>();
    CharacterData->BaseHealth = -10.0f; // Invalid value
    
    // Act
    FValidationContext Context;
    bool bIsValid = CharacterData->Validate(Context);
    
    // Assert
    EXPECT_FALSE(bIsValid);
    EXPECT_GT(Context.ValidationErrors.Num(), 0);
}
```

## Test Organization

**DelveDeep Configuration System Test Structure:**

```
Source/DelveDeep/Private/Tests/
├── ConfigurationManagerTests.cpp  # Configuration manager tests (✓ Implemented)
├── IntegrationTests.cpp          # Integration tests (✓ Implemented)
├── PerformanceTests.cpp          # Performance benchmarks (✓ Implemented)
├── ValidationTests.cpp           # Validation framework tests (✓ Implemented)
├── CombatTests.cpp               # Combat system tests (Planned)
└── AITests.cpp                   # AI system tests (Planned)
```

**Implemented Tests (Configuration System)**:
- Asset caching on first query
- Cached asset returned on subsequent queries
- Cache hit rate tracking accuracy
- Data table lookup by name
- Invalid name returns nullptr
- Query performance under target thresholds
- Upgrade cost calculation at various levels
- Asset reference resolution
- Validation error detection
- Hot-reload functionality

## Console Command Testing

**✓ Implemented**: Configuration system provides comprehensive console commands.

All systems should provide console commands for manual testing:

```cpp
// Register console commands in system initialization
static FAutoConsoleCommand ValidateAllDataCmd(
    TEXT("DelveDeep.ValidateAllData"),
    TEXT("Validates all loaded configuration data"),
    FConsoleCommandDelegate::CreateStatic(&UDelveDeepConfigurationManager::ValidateAllDataCommand)
);
```

### Common Test Commands

**Configuration System Commands (✓ Implemented):**

```bash
# Validation (✓ Implemented)
DelveDeep.ValidateAllData              # Validate all configuration data
DelveDeep.TestValidationSystem         # Test validation infrastructure
DelveDeep.ValidateSampleData           # Validate sample data
DelveDeep.CreateExampleData            # Create example test data

# Performance (✓ Implemented)
DelveDeep.ShowConfigStats              # Display cache stats and query times
DelveDeep.ProfileConfigPerformance     # Run comprehensive performance profile

# Debug (✓ Implemented)
DelveDeep.ListLoadedAssets             # List all cached assets
DelveDeep.DumpConfigData [AssetName]   # Dump asset properties
DelveDeep.ReloadConfigData             # Force reload all data

# Planned Commands
DelveDeep.ValidateCharacterData        # Validate character data only
DelveDeep.TestInputSystem              # Test input system
DelveDeep.ProfileConfigLoad            # Profile data loading
DelveDeep.TestConfigQueries [Count]    # Run N queries for performance test
```

## Integration Testing

Test system interactions and data flow:

```cpp
TEST(DelveDeepConfig, CharacterSystemIntegration)
{
    // Test that character system can load and apply character data
    UDelveDeepConfigurationManager* ConfigManager = GetConfigManager();
    const UDelveDeepCharacterData* WarriorData = ConfigManager->GetCharacterData("Warrior");
    
    ASSERT_NE(WarriorData, nullptr);
    EXPECT_GT(WarriorData->BaseHealth, 0.0f);
    EXPECT_NE(WarriorData->StartingWeapon.IsNull(), true);
}
```

## Performance Testing

**✓ Implemented**: Configuration system includes comprehensive performance tests.

Validate performance targets:

```cpp
TEST(DelveDeepConfig, QueryPerformance)
{
    UDelveDeepConfigurationManager* ConfigManager = GetConfigManager();
    
    // Measure 1000 queries
    double StartTime = FPlatformTime::Seconds();
    for (int32 i = 0; i < 1000; ++i)
    {
        ConfigManager->GetCharacterData("Warrior");
    }
    double EndTime = FPlatformTime::Seconds();
    
    double AvgQueryTime = (EndTime - StartTime) / 1000.0 * 1000.0; // Convert to ms
    EXPECT_LT(AvgQueryTime, 1.0); // Target: < 1ms per query
}
```

**Performance Targets Achieved**:
- Initialization: <100ms for 100+ assets ✓
- Single Query: <1ms per query ✓
- Bulk Queries: <1ms average for 1000 queries ✓
- Cache Hit Rate: >95% for repeated queries ✓

## Test Coverage Requirements

**Configuration System Coverage (✓ Achieved):**

### Critical Systems (Must Have Tests)
- Data validation logic ✓
- Configuration manager caching ✓
- Save/load functionality (Planned)
- Combat damage calculations (Planned)
- Upgrade cost calculations ✓

### Important Systems (Should Have Tests)
- Input handling
- AI behavior
- Progression system
- Asset reference resolution

### Nice to Have Tests
- UI interactions
- Visual effects
- Audio playback

## Test Naming Conventions

```cpp
// Pattern: TEST(SystemName, TestScenario)
TEST(DelveDeepConfig, LoadsCharacterDataSuccessfully)
TEST(DelveDeepConfig, RejectsInvalidHealthValue)
TEST(DelveDeepCombat, CalculatesDamageCorrectly)
TEST(DelveDeepCombat, AppliesDamageTypeModifiers)
```

## Assertions and Expectations

```cpp
// Use EXPECT for non-fatal checks
EXPECT_EQ(value, expected);
EXPECT_NE(pointer, nullptr);
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
EXPECT_GT(value, threshold);
EXPECT_LT(value, threshold);

// Use ASSERT for fatal checks (stops test execution)
ASSERT_NE(pointer, nullptr);
ASSERT_TRUE(IsValid(object));
```

## Mock Objects and Test Fixtures

```cpp
class ConfigurationTestFixture : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Create test game instance
        GameInstance = NewObject<UGameInstance>();
        ConfigManager = GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
    }
    
    virtual void TearDown() override
    {
        // Cleanup
        ConfigManager = nullptr;
        GameInstance = nullptr;
    }
    
    UGameInstance* GameInstance;
    UDelveDeepConfigurationManager* ConfigManager;
};

TEST_F(ConfigurationTestFixture, LoadsDataOnInitialization)
{
    ASSERT_NE(ConfigManager, nullptr);
    // Test using ConfigManager
}
```

## Running Tests

```bash
# Run all tests
UnrealEditor-Cmd.exe ProjectName -ExecCmds="Automation RunTests DelveDeep" -unattended

# Run specific test suite
UnrealEditor-Cmd.exe ProjectName -ExecCmds="Automation RunTests DelveDeep.Config" -unattended

# Run with verbose output
UnrealEditor-Cmd.exe ProjectName -ExecCmds="Automation RunTests DelveDeep" -unattended -log
```

## Continuous Integration

Tests should run automatically in CI/CD pipeline:
- Run on every commit to main branch
- Run on pull requests
- Fail build if tests fail
- Generate test coverage reports
