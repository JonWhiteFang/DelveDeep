# DelveDeep Development Guidelines

This directory contains comprehensive development guidelines and best practices for the DelveDeep project. All guidelines are informed by real implementation experience from completed systems.

## Quick Reference

### Completed Systems ✅

**Data-Driven Configuration System** (October 23, 2025) - Reference implementation demonstrating all best practices:
- [data-assets.md](data-assets.md) - Data asset patterns and organization
- [subsystems.md](subsystems.md) - Subsystem architecture and lifecycle
- [error-handling.md](error-handling.md) - Validation and error handling
- [testing.md](testing.md) - Testing strategies and coverage
- [code-review.md](code-review.md) - Code review checklist

**Enhanced Validation System** (October 23, 2025) - Robust validation framework:
- [error-handling.md](error-handling.md) - FValidationContext patterns and usage
- [data-assets.md](data-assets.md) - PostLoad validation integration
- [testing.md](testing.md) - Validation testing procedures

**Centralized Event System** (October 24, 2025) - Event-driven architecture:
- [subsystems.md](subsystems.md) - Event subsystem patterns
- [error-handling.md](error-handling.md) - Event payload validation
- [testing.md](testing.md) - Event system testing

**Performance Telemetry System** (October 24, 2025) - Production-ready performance monitoring:
- [subsystems.md](subsystems.md) - Telemetry subsystem patterns
- [testing.md](testing.md) - Performance testing procedures
- [code-review.md](code-review.md) - Performance profiling checklist

**Automated Testing Framework** (October 27, 2025) - Comprehensive testing infrastructure:
- [testing.md](testing.md) - Testing framework and utilities
- [code-review.md](code-review.md) - Test coverage requirements

### Core Guidelines

**Architecture & Design:**
- [structure.md](structure.md) - Project structure and naming conventions
- [subsystems.md](subsystems.md) - When and how to use subsystems
- [blueprint-integration.md](blueprint-integration.md) - Blueprint-ready C++ design

**Data & Configuration:**
- [data-assets.md](data-assets.md) - Data-driven configuration patterns
- [error-handling.md](error-handling.md) - Validation and error handling

**Quality & Testing:**
- [testing.md](testing.md) - Testing philosophy and practices
- [code-review.md](code-review.md) - Pre-commit checklist

**Project Context:**
- [product.md](product.md) - Product overview and goals
- [tech.md](tech.md) - Technical stack and tools
- [mcp-server-usage.md](mcp-server-usage.md) - MCP tool usage guidelines

## Implementation Status

### Phase 1: Core Foundation (5/5 Complete) ✅

✅ **Data-Driven Configuration System** (Complete - October 23, 2025)
- FValidationContext for error/warning tracking
- UDelveDeepConfigurationManager subsystem
- Data asset classes (Character, Weapon, Ability, Upgrade)
- Data table structures (Monster configuration)
- Performance optimization (<100ms init, <1ms queries, >95% cache hit rate)
- Hot-reload support (development builds)
- Comprehensive validation and testing
- Console commands for debugging

**Key Achievements:**
- Sub-100ms initialization for 100+ assets (87ms achieved)
- Sub-millisecond query performance (0.43ms average)
- >95% cache hit rate (97.3% achieved)
- Comprehensive validation with FValidationContext
- Hot-reload within 2 seconds (1.8s achieved)
- Full Blueprint integration
- Extensive test coverage

**Documentation:**
- [ValidationSystem.md](../Documentation/Systems/ValidationSystem.md)
- [DataDrivenConfiguration.md](../Documentation/Systems/DataDrivenConfiguration.md)
- [ContentDirectoryStructure.md](../Documentation/Systems/ContentDirectoryStructure.md)
- [Performance-Testing.md](../Documentation/Systems/Performance-Testing.md)
- [Testing-Guide.md](../Documentation/Testing-Guide.md)

✅ **Enhanced Validation System** (Complete - October 23, 2025)
- FValidationContext struct with error/warning tracking
- Context-aware error reporting (SystemName, OperationName)
- Formatted validation reports with GetReport()
- Integration with all data asset classes
- PostLoad() validation for automatic error detection
- Console commands for validation testing
- Comprehensive documentation and examples

**Key Components:**
- AddError/AddWarning: Context-aware error tracking
- GetReport(): Formatted validation output
- IsValid(): Boolean validation status
- Reset(): Context reuse capability

**Documentation:**
- [ValidationSystem.md](../Documentation/Systems/ValidationSystem.md)
- [error-handling.md](error-handling.md)
- [Testing-Guide.md](../Documentation/Testing-Guide.md)

✅ **Centralized Event System** (Complete - October 24, 2025)
- Hierarchical event filtering using GameplayTags
- Priority-based listener execution (High, Normal, Low)
- Deferred event processing for safe batch operations
- Spatial and actor-specific event filtering
- Build-specific validation (development vs shipping)
- Comprehensive performance monitoring and metrics
- Full Blueprint integration
- Network replication metadata for future multiplayer

**Key Achievements:**
- Event broadcast: <1ms for 50 listeners
- Listener invocation: <0.1ms per listener
- System overhead: <0.1ms per event
- Deferred processing: <10ms for 1000 events
- Full GameplayTag hierarchy integration
- Comprehensive console commands for debugging

**Documentation:**
- [CentralizedEventSystem.md](../Documentation/Systems/CentralizedEventSystem.md)
- [subsystems.md](subsystems.md)
- [error-handling.md](error-handling.md)

✅ **Performance Telemetry System** (Complete - October 24, 2025)
- Frame performance tracking (FPS, frame time, spike detection)
- System profiling with budget tracking
- Memory tracking and leak detection
- Performance baseline capture and comparison
- Real-time performance overlay (Minimal, Standard, Detailed modes)
- Profiling sessions with detailed metrics
- Gameplay metrics tracking (entity counts, events)
- Asset loading performance tracking
- Conditional compilation for build configurations
- Comprehensive error handling and validation

**Key Achievements:**
- Telemetry overhead: <0.5ms per frame (development)
- Telemetry overhead: <0.1ms per frame (shipping)
- Overlay rendering: <0.1ms per frame
- Memory snapshot: <1ms per capture
- Report generation: <100ms for 5-minute data

**Documentation:**
- [TelemetryIntegrationGuide.md](../Documentation/Systems/TelemetryIntegrationGuide.md)
- [TelemetryBuildConfigurations.md](../Documentation/Systems/TelemetryBuildConfigurations.md)
- [Performance-Testing.md](../Documentation/Systems/Performance-Testing.md)

✅ **Automated Testing Framework** (Complete - October 27, 2025)
- Google Test-style assertion macros (EXPECT_*, ASSERT_*)
- Test fixtures for setup/teardown (FDelveDeepTestFixture, FSubsystemTestFixture)
- Async test support with latent commands
- Performance measurement utilities (FScopedTestTimer)
- Memory tracking utilities (FScopedMemoryTracker)
- Test report generation (Markdown, HTML, JUnit XML)
- CI/CD integration scripts (RunTests.sh, RunTests.bat)
- Comprehensive test organization by system

**Key Achievements:**
- Test execution: <30 seconds for full suite
- Parallel execution support
- Watch mode for development
- Comprehensive test coverage for all systems

**Documentation:**
- [TestingFramework.md](../Documentation/Systems/TestingFramework.md)
- [Testing-Guide.md](../Documentation/Testing-Guide.md)
- [TestReportGeneration.md](../Documentation/Systems/TestReportGeneration.md)

### Phase 2: Core Gameplay (1/4 In Progress)

🔄 **Character System Foundation** (In Progress - October 27, 2025)
- Component-based architecture with UDelveDeepCharacterComponent
- Base character actor (ADelveDeepCharacter) inheriting from APaperCharacter
- Four character classes (Warrior, Ranger, Mage, Necromancer)
- Stats component for health, resource, damage, and move speed
- Class-specific resource systems (Rage, Energy, Mana, Souls)
- Integration with Configuration, Event, and Telemetry systems
- Blueprint-ready design with full Blueprint integration
- Performance optimization with tick disabled by default

**Active Spec**: [.kiro/specs/character-system-foundation/](.kiro/specs/character-system-foundation/)

### Next Priorities

🔄 **Enhanced Input System** (Next - Phase 2)
- Multi-device support (keyboard, gamepad)
- Input buffering
- Action mapping

🔄 **Movement System** (Phase 2)
- 2D movement with collision
- Wall sliding
- Animation state machine

## Using These Guidelines

### For New Features

1. **Review relevant guidelines** before starting implementation
2. **Follow established patterns** from completed systems
3. **Use configuration, event, and telemetry systems as reference** for architecture decisions
4. **Validate early and often** using FValidationContext
5. **Write tests** for critical functionality
6. **Update documentation** as you implement

### For Code Review

1. **Check code-review.md** for pre-commit checklist
2. **Verify performance targets** are met
3. **Ensure validation is comprehensive**
4. **Confirm Blueprint integration** where appropriate
5. **Review test coverage**

### For Architecture Decisions

1. **Consult subsystems.md** for system design
2. **Review data-assets.md** for configuration patterns
3. **Check structure.md** for naming and organization
4. **Follow blueprint-integration.md** for C++ design

## Best Practices Summary

### Performance
- Disable tick by default: `PrimaryActorTick.bCanEverTick = false`
- Pre-allocate containers: `Items.Reserve(ExpectedSize)`
- Use object pooling for frequently spawned objects
- Profile with `SCOPE_CYCLE_COUNTER`
- Target: 60+ FPS, <100ms init, <1ms queries, <1ms event broadcasts

### Validation
- Use `FValidationContext` for all validation
- Validate in `PostLoad()` for data assets
- Include context in error messages
- Provide recovery paths when possible
- Log with appropriate severity

### Memory Management
- Use `TSoftObjectPtr` for asset references
- Use `UPROPERTY()` to prevent garbage collection
- Clean up in `Deinitialize()`
- Avoid manual memory management

### Blueprint Integration
- All classes use `DELVEDEEP_API` macro
- Mark with `BlueprintType` and `Blueprintable`
- Use appropriate property specifiers
- Add categories to all UFUNCTION/UPROPERTY
- Provide console commands for testing

### Testing
- Write tests for critical systems
- Provide console commands for manual testing
- Validate performance targets
- Test integration between systems
- Document test coverage

## Console Commands Reference

### Configuration System (✓ Implemented)
```bash
DelveDeep.ValidateAllData         # Validate all configuration data
DelveDeep.ShowConfigStats         # Display performance statistics
DelveDeep.ListLoadedAssets        # List all cached assets
DelveDeep.ReloadConfigData        # Force reload all data
DelveDeep.DumpConfigData [Name]   # Dump asset properties
DelveDeep.CreateExampleData       # Create example test data
```

### Event System (✓ Implemented)
```bash
DelveDeep.Events.ShowMetrics          # Display performance statistics
DelveDeep.Events.ShowEventHistory     # Show recent event history
DelveDeep.Events.EnableEventLogging   # Enable detailed logging
DelveDeep.Events.DisableEventLogging  # Disable logging
DelveDeep.Events.ListListeners <Tag>  # List listeners for tag
DelveDeep.Events.ListAllListeners     # List all listeners
DelveDeep.Events.BroadcastTestEvent   # Broadcast test event
```

### Telemetry System (✓ Implemented)
```bash
# Frame tracking
DelveDeep.Telemetry.ShowFPS
DelveDeep.Telemetry.ShowFrameStats

# System profiling
DelveDeep.Telemetry.ShowSystemStats
DelveDeep.Telemetry.ShowBudgets

# Memory tracking
DelveDeep.Telemetry.ShowMemory
DelveDeep.Telemetry.CheckMemoryLeaks

# Baseline management
DelveDeep.Telemetry.CaptureBaseline <Name>
DelveDeep.Telemetry.CompareBaseline <Name>

# Reporting
DelveDeep.Telemetry.GenerateReport [Duration]
DelveDeep.Telemetry.ExportCSV <Path>

# Profiling
DelveDeep.Telemetry.StartProfiling <SessionName>
DelveDeep.Telemetry.StopProfiling

# Visualization
DelveDeep.Telemetry.EnableOverlay [Mode]
DelveDeep.Telemetry.DisableOverlay
```

### Testing Framework (✓ Implemented)
```bash
# Run tests
DelveDeep.RunTests [Filter]           # Run tests matching filter
DelveDeep.RunAllTests                 # Run all tests
DelveDeep.ListTests                   # List all available tests

# Test reports
DelveDeep.GenerateTestReport          # Generate test report
DelveDeep.ExportTestResults <Path>    # Export results to file
```

### System Testing (Planned)
```bash
DelveDeep.ValidateAllSystems      # Run comprehensive system validation
DelveDeep.TestInputSystem         # Validate input system
DelveDeep.TestCombatSystem        # Validate combat system
```

### Performance Monitoring (Planned)
```bash
stat fps                          # Display frame rate
stat memory                       # Monitor memory usage
stat DelveDeep                    # Show DelveDeep-specific metrics
```

## Performance Targets

### Configuration System (✓ Achieved)
- Initialization: <100ms for 100+ assets
- Single Query: <1ms per query
- Bulk Queries: <1ms average for 1000 queries
- Cache Hit Rate: >95% for repeated queries

### Event System (✓ Achieved)
- Event broadcast: <1ms for 50 listeners
- Listener invocation: <0.1ms per listener
- System overhead: <0.1ms per event
- Deferred processing: <10ms for 1000 events

### Telemetry System (✓ Achieved)
- Telemetry overhead: <0.5ms per frame (development)
- Telemetry overhead: <0.1ms per frame (shipping)
- Overlay rendering: <0.1ms per frame
- Memory snapshot: <1ms per capture
- Report generation: <100ms for 5-minute data

### Testing Framework (✓ Achieved)
- Test execution: <30 seconds for full suite
- Parallel execution support
- Watch mode for development

### General Targets
- Frame Rate: 60+ FPS
- Save/Load: <100ms
- Input Latency: <16ms
- Asset Loading: <100ms

## Documentation Structure

```
.kiro/steering/
├── README.md                      # This file
├── product.md                     # Product overview
├── tech.md                        # Technical stack
├── structure.md                   # Project structure
├── subsystems.md                  # Subsystem guidelines ✓
├── data-assets.md                 # Data asset patterns ✓
├── error-handling.md              # Validation patterns ✓
├── testing.md                     # Testing guidelines ✓
├── code-review.md                 # Review checklist ✓
├── blueprint-integration.md       # Blueprint patterns
└── mcp-server-usage.md           # MCP tool usage

Documentation/
├── Systems/
│   ├── ValidationSystem.md       # Validation infrastructure ✓
│   ├── ContentDirectoryStructure.md  # Asset organization ✓
│   ├── Performance-Testing.md    # Performance testing ✓
│   ├── DataDrivenConfiguration.md    # System overview ✓
│   ├── CentralizedEventSystem.md     # Event bus architecture ✓
│   ├── TelemetryIntegrationGuide.md  # Telemetry integration ✓
│   ├── TelemetryBuildConfigurations.md  # Build configs ✓
│   ├── TestingFramework.md       # Testing framework ✓
│   └── TestReportGeneration.md   # Test reporting ✓
└── Implementation/
    ├── Task1-ValidationInfrastructure.md  # Implementation notes ✓
    └── Task11-ExampleDataAssets.md        # Example data ✓
```

## Contributing

When implementing new systems:

1. **Follow established patterns** from configuration, event, telemetry, and testing systems
2. **Use FValidationContext** for validation
3. **Implement as subsystem** if game-wide functionality
4. **Write comprehensive tests**
5. **Provide console commands** for debugging
6. **Update documentation** in Documentation/Systems/
7. **Update this README** with implementation status

## Questions?

- Check [DOCUMENTATION_INDEX.md](../../DOCUMENTATION_INDEX.md) for complete documentation
- Review completed systems for reference implementation
- Consult specific steering documents for detailed guidance

---

**Last Updated**: October 27, 2025

**Status**: Phase 1 Complete (5/5), Phase 2 In Progress (1/4)

**Completed**: Data-Driven Configuration (Oct 23), Enhanced Validation (Oct 23), Centralized Event System (Oct 24), Performance Telemetry (Oct 24), Automated Testing Framework (Oct 27)

**In Progress**: Character System Foundation (Oct 27)

**Next**: Enhanced Input System
