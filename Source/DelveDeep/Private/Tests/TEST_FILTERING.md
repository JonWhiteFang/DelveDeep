# Test Filtering and Tagging Guide

This document explains how to filter and run specific tests in the DelveDeep automated testing framework.

## Test Filters

### Product Filter (Unit Tests)

**Flag:** `EAutomationTestFlags::ProductFilter`

**Characteristics:**
- Fast-running unit tests
- Test individual components in isolation
- Run in parallel for speed
- Target: <30 seconds for full suite
- Default filter for CI/CD pipelines

**Usage:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Product" -unattended
```

**Examples:**
- Configuration manager caching tests
- Event system filtering tests
- Data validation tests
- Subsystem initialization tests

### Performance Filter (Performance Tests)

**Flag:** `EAutomationTestFlags::PerfFilter`

**Characteristics:**
- Performance benchmarking tests
- Measure execution time and memory usage
- Verify performance targets are met
- May run slower than unit tests
- Run separately from unit tests

**Usage:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Perf" -unattended
```

**Examples:**
- Query performance benchmarks (<1ms target)
- Event broadcast performance (<1ms for 50 listeners)
- Cache hit rate tests (>95% target)
- Memory allocation tests

### Stress Filter (Stress Tests)

**Flag:** `EAutomationTestFlags::StressFilter`

**Characteristics:**
- Long-running stress tests
- Test system behavior under load
- Not run by default in CI
- Used for capacity planning and stability testing

**Usage:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject -ExecCmds="Automation RunFilter Stress" -unattended
```

**Examples:**
- High-load event broadcasting (1000+ events)
- Large dataset queries (1000+ assets)
- Memory stress tests
- Long-running operations

## Test Name Pattern Filtering

Tests follow a hierarchical naming pattern that enables filtering by system or feature:

### Pattern Format
```
DelveDeep.<System>.<Feature>
```

### By System

**Configuration Tests:**
```bash
Automation RunTests DelveDeep.Configuration
```
Runs all configuration system tests including:
- Asset caching
- Data validation
- Query performance
- Hot-reload

**Event Tests:**
```bash
Automation RunTests DelveDeep.Events
```
Runs all event system tests including:
- Event broadcasting
- Filtering by GameplayTags
- Priority execution
- Deferred processing

**Telemetry Tests:**
```bash
Automation RunTests DelveDeep.Telemetry
```
Runs all telemetry system tests including:
- Frame tracking
- Memory tracking
- Budget validation
- Performance baselines

**Integration Tests:**
```bash
Automation RunTests DelveDeep.Integration
```
Runs all integration tests including:
- Configuration + Events
- Events + Telemetry
- Multi-system state consistency

### By Feature

**Cache Tests:**
```bash
Automation RunTests DelveDeep.Configuration.Cache
```

**Validation Tests:**
```bash
Automation RunTests DelveDeep.Validation
```

**Performance Tests:**
```bash
Automation RunTests DelveDeep.Performance
```

**Memory Tests:**
```bash
Automation RunTests DelveDeep.Memory
```

### Specific Test

Run a single test by its full name:
```bash
Automation RunTests DelveDeep.Configuration.AssetCaching
```

## Command Line Examples

### Development Workflow

**Run all unit tests (fast feedback):**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject \
    -ExecCmds="Automation RunFilter Product" \
    -unattended \
    -nopause
```

**Run tests for specific system you're working on:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject \
    -ExecCmds="Automation RunTests DelveDeep.Configuration" \
    -unattended \
    -nopause
```

**Run single test during debugging:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject \
    -ExecCmds="Automation RunTests DelveDeep.Configuration.AssetCaching" \
    -unattended \
    -nopause \
    -log
```

### CI/CD Pipeline

**Standard CI test run:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject \
    -ExecCmds="Automation RunFilter Product" \
    -ReportOutputPath="TestResults" \
    -unattended \
    -nopause \
    -NullRHI \
    -log
```

**Performance benchmarking:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject \
    -ExecCmds="Automation RunFilter Perf" \
    -ReportOutputPath="PerfResults" \
    -unattended \
    -nopause \
    -NullRHI \
    -log
```

**Nightly stress tests:**
```bash
UnrealEditor-Cmd.exe DelveDeep.uproject \
    -ExecCmds="Automation RunFilter Stress" \
    -ReportOutputPath="StressResults" \
    -unattended \
    -nopause \
    -NullRHI \
    -log
```

## Test Organization by Category

### Unit Tests (ProductFilter)

**Configuration System:**
- `DelveDeep.Configuration.AssetCaching`
- `DelveDeep.Configuration.CacheHitRate`
- `DelveDeep.Configuration.DataTableLookup`
- `DelveDeep.Configuration.InvalidNameHandling`

**Event System:**
- `DelveDeep.Events.Broadcasting`
- `DelveDeep.Events.Filtering`
- `DelveDeep.Events.PriorityExecution`
- `DelveDeep.Events.DeferredProcessing`

**Telemetry System:**
- `DelveDeep.Telemetry.FrameTracking`
- `DelveDeep.Telemetry.MemoryTracking`
- `DelveDeep.Telemetry.BudgetValidation`

**Validation:**
- `DelveDeep.Validation.CharacterData`
- `DelveDeep.Validation.MonsterConfig`
- `DelveDeep.Validation.ErrorReporting`

### Integration Tests (ProductFilter)

**Cross-System:**
- `DelveDeep.Integration.ConfigurationEventCommunication`
- `DelveDeep.Integration.ConfigurationValidationEvent`
- `DelveDeep.Integration.EventTelemetryPerformanceTracking`
- `DelveDeep.Integration.MultiSystemStateConsistency`

### Performance Tests (PerfFilter)

**Query Performance:**
- `DelveDeep.Performance.ConfigurationQuery`
- `DelveDeep.Performance.CacheHitRate`
- `DelveDeep.Performance.BulkQueries`

**Event Performance:**
- `DelveDeep.Performance.EventBroadcast`
- `DelveDeep.Performance.ListenerInvocation`
- `DelveDeep.Performance.DeferredProcessing`

**Memory Performance:**
- `DelveDeep.Memory.AllocationTracking`
- `DelveDeep.Memory.LeakDetection`
- `DelveDeep.Memory.BudgetValidation`

## Using Test Scripts

### Unix/Linux/Mac

**RunTests.sh:**
```bash
#!/bin/bash
./RunTests.sh
```

Features:
- Runs all unit tests (ProductFilter)
- Generates test reports in TestResults/
- Exits with non-zero code on failure
- Uses headless mode (-NullRHI)

### Windows

**RunTests.bat:**
```batch
RunTests.bat
```

Features:
- Runs all unit tests (ProductFilter)
- Generates test reports in TestResults/
- Exits with non-zero code on failure
- Uses headless mode (-NullRHI)

## Test Report Generation

All test runs can generate reports for CI/CD integration:

```bash
-ReportOutputPath="TestResults"
```

Report formats:
- JSON (machine-readable)
- HTML (human-readable)
- JUnit XML (CI integration)

## Best Practices

### During Development

1. **Run unit tests frequently** - Fast feedback loop
2. **Filter by system** - Only run tests for code you're changing
3. **Run specific test** - Debug individual test failures
4. **Use -log flag** - Get detailed output for debugging

### Before Committing

1. **Run all unit tests** - Ensure no regressions
2. **Run integration tests** - Verify cross-system compatibility
3. **Check performance tests** - Ensure targets are met
4. **Review test reports** - Check for warnings

### In CI/CD

1. **Run unit tests on every commit** - Fast feedback
2. **Run performance tests nightly** - Track performance trends
3. **Run stress tests weekly** - Verify stability
4. **Generate reports** - Track test history

## Troubleshooting

### Tests Not Running

**Check test flags:**
```cpp
EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
```

**Verify test name pattern:**
```cpp
"DelveDeep.Configuration.AssetCaching"
```

### Tests Timing Out

**Increase timeout for async tests:**
```cpp
ADD_WAIT_FOR_CONDITION(Condition, 10.0f);  // 10 second timeout
```

**Check for infinite loops or blocking operations**

### Tests Failing in CI

**Use -NullRHI flag** - Headless mode for CI
**Check for editor-only dependencies** - Use `#if WITH_EDITOR`
**Verify test isolation** - Tests should not depend on each other

## References

- [Unreal Engine Automation System](https://docs.unrealengine.com/en-US/TestingAndOptimization/Automation/)
- [DelveDeep Testing Framework README](README.md)
- [Test Utilities Documentation](DelveDeepTestUtilities.h)
- [Test Fixtures Documentation](DelveDeepTestFixtures.h)
