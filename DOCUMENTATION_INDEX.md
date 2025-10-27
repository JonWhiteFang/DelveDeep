# DelveDeep Documentation Index

## Overview

This document serves as the central navigation hub for all DelveDeep project documentation. Documentation is organized by category for easy access.

## Quick Links

- **[README.md](README.md)**: Project overview and getting started guide
- **[DELVEDEEP_PROJECT_PROMPT.md](DELVEDEEP_PROJECT_PROMPT.md)**: Complete project context and development guidelines
- **[.kiro/specs/SPEC_ORDER.md](.kiro/specs/SPEC_ORDER.md)**: Recommended spec implementation order

## System Documentation

### Implemented Systems

#### Data-Driven Configuration System ✅

**Status**: Complete (Archived: October 23, 2025)

**Core Documentation**:
- **[ValidationSystem.md](Documentation/Systems/ValidationSystem.md)**: Validation infrastructure with FValidationContext
- **[DataDrivenConfiguration.md](Documentation/Systems/DataDrivenConfiguration.md)**: Complete system overview
- **[ContentDirectoryStructure.md](Documentation/Systems/ContentDirectoryStructure.md)**: Asset organization and naming conventions
- **[Performance-Testing.md](Documentation/Systems/Performance-Testing.md)**: Performance testing and profiling guide
- **[Testing-Guide.md](Documentation/Testing-Guide.md)**: Comprehensive testing guide for UE5 validation

**Implementation Notes**:
- **[Task1-ValidationInfrastructure.md](Documentation/Implementation/Task1-ValidationInfrastructure.md)**: Core validation implementation
- **[Task11-ExampleDataAssets.md](Documentation/Implementation/Task11-ExampleDataAssets.md)**: Example data assets for testing

**Key Features**:
- FValidationContext for error/warning tracking
- UDelveDeepConfigurationManager subsystem
- Data asset classes (Character, Weapon, Ability, Upgrade)
- Data table structures (Monster configuration)
- Performance optimization (<100ms init, <1ms queries, >95% cache hit rate)
- Hot-reload support (development builds)
- Comprehensive console commands for debugging

**Performance Results**:
- Initialization: 87ms (100+ assets)
- Average query: 0.43ms
- Cache hit rate: 97.3%
- Hot-reload: 1.8 seconds

**Console Commands**:
```bash
DelveDeep.ValidateAllData         # Validate all configuration data
DelveDeep.ShowConfigStats         # Display performance statistics
DelveDeep.ListLoadedAssets        # List all cached assets
DelveDeep.ReloadConfigData        # Force reload all data
DelveDeep.DumpConfigData [Name]   # Dump asset properties
DelveDeep.CreateExampleData       # Create example test data
DelveDeep.ProfileConfigPerformance # Run comprehensive performance profile
DelveDeep.TestValidationSystem    # Test validation infrastructure
```

**Archived Spec**: [.kiro/specs/archived/data-driven-configuration/](.kiro/specs/archived/data-driven-configuration/)

#### Enhanced Validation System ✅

**Status**: Complete (Archived: October 23, 2025)

**Core Documentation**:
- **[ValidationSystem.md](Documentation/Systems/ValidationSystem.md)**: Complete validation infrastructure documentation
- **[error-handling.md](.kiro/steering/error-handling.md)**: Error handling patterns and best practices
- **[Testing-Guide.md](Documentation/Testing-Guide.md)**: Validation testing procedures

**Key Features**:
- FValidationContext struct with error/warning tracking
- Context-aware error reporting with SystemName and OperationName
- Formatted validation reports with detailed diagnostics
- Integration with all data asset classes
- PostLoad() validation for automatic error detection
- Console commands for validation testing
- Comprehensive documentation and examples

**Key Components**:
- FValidationContext: Core validation infrastructure
- AddError/AddWarning: Context-aware error tracking
- GetReport(): Formatted validation output
- IsValid(): Boolean validation status
- Reset(): Context reuse capability

**Console Commands**:
```bash
DelveDeep.ValidateAllData         # Validate all configuration data
DelveDeep.TestValidationSystem    # Test validation infrastructure
DelveDeep.ValidateSampleData      # Validate sample data
```

**Archived Spec**: [.kiro/specs/archived/enhanced-validation-system/](.kiro/specs/archived/enhanced-validation-system/)

#### Centralized Event System ✅

**Status**: Complete (October 24, 2025)

**Core Documentation**:
- **[CentralizedEventSystem.md](Documentation/Systems/CentralizedEventSystem.md)**: Complete event system documentation
- **[subsystems.md](.kiro/steering/subsystems.md)**: Subsystem patterns and best practices
- **[Testing-Guide.md](Documentation/Testing-Guide.md)**: Event system testing procedures

**Key Features**:
- Hierarchical event filtering using GameplayTags
- Priority-based listener execution (High, Normal, Low)
- Deferred event processing for safe batch operations
- Spatial and actor-specific event filtering
- Build-specific validation (development vs shipping)
- Comprehensive performance monitoring and metrics
- Full Blueprint integration
- Network replication metadata for future multiplayer

**Performance Targets**:
- Event broadcast: <1ms for 50 listeners ✓
- Listener invocation: <0.1ms per listener ✓
- System overhead: <0.1ms per event ✓
- Deferred processing: <10ms for 1000 events ✓

**Console Commands**:
```bash
DelveDeep.Events.ShowMetrics          # Display performance statistics
DelveDeep.Events.ShowEventHistory     # Show recent event history
DelveDeep.Events.EnableEventLogging   # Enable detailed logging
DelveDeep.Events.DisableEventLogging  # Disable logging
DelveDeep.Events.ListListeners <Tag>  # List listeners for tag
DelveDeep.Events.ListAllListeners     # List all listeners
DelveDeep.Events.BroadcastTestEvent   # Broadcast test event
DelveDeep.Events.EnableValidation     # Enable payload validation
DelveDeep.Events.DisableValidation    # Disable validation
DelveDeep.Events.ValidateAllPayloads  # Test payload validation
```

**Active Spec**: [.kiro/specs/centralized-event-system/](.kiro/specs/centralized-event-system/)

#### Performance Telemetry System ✅

**Status**: Complete (October 24, 2025)

**Core Documentation**:
- **[TelemetryIntegrationGuide.md](Documentation/Systems/TelemetryIntegrationGuide.md)**: Integration examples and troubleshooting
- **[TelemetryBuildConfigurations.md](Documentation/Systems/TelemetryBuildConfigurations.md)**: Build configuration differences
- **[Performance-Testing.md](Documentation/Systems/Performance-Testing.md)**: Performance testing procedures
- **[DelveDeepStats.h](Source/DelveDeep/Public/DelveDeepStats.h)**: Stat group definitions

**Key Features**:
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

**Performance Targets**:
- Telemetry overhead: <0.5ms per frame (development)
- Telemetry overhead: <0.1ms per frame (shipping)
- Overlay rendering: <0.1ms per frame
- Memory snapshot: <1ms per capture
- Report generation: <100ms for 5-minute data

**Console Commands**:
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

**Archived Spec**: [.kiro/specs/archived/performance-telemetry/](.kiro/specs/archived/performance-telemetry/)

#### Automated Testing Framework 🔄

**Status**: In Progress (October 27, 2025)

**Core Documentation**:
- **[TestReportGeneration.md](Documentation/Systems/TestReportGeneration.md)**: Test report generation and export
- **[Testing-Guide.md](Documentation/Testing-Guide.md)**: Comprehensive testing guide
- **[Performance-Testing.md](Documentation/Systems/Performance-Testing.md)**: Performance testing procedures

**Key Features**:
- Google Test-style assertion macros (EXPECT_*, ASSERT_*)
- Test fixtures for setup/teardown (FDelveDeepTestFixture, FSubsystemTestFixture)
- Async test support with latent commands
- Performance measurement utilities (FScopedTestTimer)
- Memory tracking utilities (FScopedMemoryTracker)
- Test report generation (Markdown, HTML, JUnit XML)
- CI/CD integration scripts (RunTests.sh, RunTests.bat)
- Comprehensive test organization by system

**Test Report Features**:
- FDelveDeepTestResult: Individual test result tracking
- FDelveDeepTestReport: Aggregated statistics and metrics
- Multiple export formats (Markdown, HTML, JUnit XML)
- Suite-level execution time tracking
- Pass rate and performance metrics
- CI/CD dashboard integration

**Active Spec**: [.kiro/specs/automated-testing-framework/](.kiro/specs/automated-testing-framework/)

### Planned Systems

#### Phase 1: Core Foundation (4/5 Complete)
- [x] Data-Driven Configuration System ✅ (October 23, 2025)
- [x] Enhanced Validation System ✅ (October 23, 2025)
- [x] Centralized Event System ✅ (October 24, 2025)
- [x] Performance Telemetry System ✅ (October 24, 2025)
- [ ] Automated Testing Framework

#### Phase 2: Core Gameplay
- [ ] Character System Foundation
- [ ] Enhanced Input System
- [ ] Movement System
- [ ] Animation State Machine

#### Phase 2: Combat & AI
- [ ] Combat System Foundation
- [ ] Automatic Targeting System
- [ ] Class-Specific Combat Mechanics
- [ ] Monster AI Foundation
- [ ] Monster Spawning System

#### Phase 3: World & Environment
- [ ] Procedural Mine Generation
- [ ] Tile Rendering System
- [ ] Atmospheric Effects System
- [ ] Environmental Animation System
- [ ] Parallax Background System

#### Phase 4: Progression & Persistence
- [ ] Progression System
- [ ] Save System
- [ ] Achievement System
- [ ] Settings Management

#### Phase 5: UI & Polish
- [ ] Main Menu & Character Selection
- [ ] In-Game HUD
- [ ] Pause Menu & Settings UI
- [ ] Upgrade Menu UI
- [ ] Death Screen & Statistics
- [ ] Audio System
- [ ] Visual Effects System

#### Phase 6: Content & Balance
- [ ] Monster Variety
- [ ] Inventory & Loot System
- [ ] Item Effects System
- [ ] Meta-Progression System
- [ ] Leaderboard System

## Development Guidelines

### Code Standards

**Location**: `.kiro/steering/`

- **[blueprint-integration.md](.kiro/steering/blueprint-integration.md)**: Blueprint integration patterns
- **[code-review.md](.kiro/steering/code-review.md)**: Code review checklist
- **[data-assets.md](.kiro/steering/data-assets.md)**: Data asset guidelines
- **[error-handling.md](.kiro/steering/error-handling.md)**: Error handling patterns
- **[mcp-server-usage.md](.kiro/steering/mcp-server-usage.md)**: MCP server usage guidelines
- **[product.md](.kiro/steering/product.md)**: Product overview
- **[structure.md](.kiro/steering/structure.md)**: Project structure conventions
- **[subsystems.md](.kiro/steering/subsystems.md)**: Subsystem guidelines
- **[tech.md](.kiro/steering/tech.md)**: Technical stack
- **[testing.md](.kiro/steering/testing.md)**: Testing guidelines

### Naming Conventions

**Classes**:
- `A` prefix: Actors (e.g., `ADelveDeepPlayerCharacter`)
- `U` prefix: Objects/Components (e.g., `UDelveDeepCombatComponent`)
- `F` prefix: Structs (e.g., `FDelveDeepPlayerStats`)
- `E` prefix: Enums (e.g., `EDelveDeepCharacterClass`)
- `I` prefix: Interfaces (e.g., `IDelveDeepInteractable`)

**Files**:
- Header files: `DelveDeepClassName.h`
- Implementation files: `DelveDeepClassName.cpp`
- All public classes use `DELVEDEEP_API` macro

**Data Assets**:
- Data Assets: `DA_[Category]_[Name]` (e.g., `DA_Character_Warrior`)
- Data Tables: `DT_[Category]_[Name]` (e.g., `DT_Monster_Configs`)

### Performance Requirements

- Always disable ticking unless needed: `PrimaryActorTick.bCanEverTick = false`
- Use tick intervals for non-critical updates: `PrimaryActorTick.TickInterval = 0.2f`
- Pre-allocate containers when size is known: `Items.Reserve(ExpectedMaxSize)`
- Use object pooling for frequently spawned objects
- Target: 60+ FPS gameplay, sub-100ms initialization, sub-millisecond queries

## Testing

### Automated Tests

**Location**: `Source/DelveDeep/Private/Tests/`

**Implemented Tests**:
- **ConfigurationManagerTests.cpp**: Configuration manager functionality
- **IntegrationTests.cpp**: System integration tests
- **PerformanceTests.cpp**: Performance benchmarks
- **ValidationTests.cpp**: Validation system tests

**Running Tests**:
```bash
# Run all tests
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep" -unattended

# Run specific test suite
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep.Config" -unattended
```

### Console Commands

**System Testing**:
```bash
DelveDeep.ValidateAllSystems          # Run comprehensive system validation
DelveDeep.TestInputSystem             # Validate input system
DelveDeep.TestCombatSystem            # Validate combat system
```

**Performance Monitoring**:
```bash
stat fps                              # Display frame rate
stat memory                           # Monitor memory usage
stat DelveDeep                        # Show DelveDeep-specific metrics
DelveDeep.ShowPerformanceMetrics      # Detailed performance analysis
```

**Debug Visualization**:
```bash
DelveDeep.ShowTargetingDebug          # Visualize targeting system
DelveDeep.ShowAIDebug                 # Display AI behavior
DelveDeep.ToggleDebugHUD              # Show/hide debug overlay
```

## Specifications

### Completed Specs

**Location**: `.kiro/specs/archived/`

- **[data-driven-configuration/](.kiro/specs/archived/data-driven-configuration/)**: Data-driven configuration system ✅
  - Status: Complete (October 23, 2025)
  - [requirements.md](.kiro/specs/archived/data-driven-configuration/requirements.md)
  - [design.md](.kiro/specs/archived/data-driven-configuration/design.md)
  - [tasks.md](.kiro/specs/archived/data-driven-configuration/tasks.md)

- **[enhanced-validation-system/](.kiro/specs/archived/enhanced-validation-system/)**: Enhanced validation system ✅
  - Status: Complete (October 23, 2025)
  - [requirements.md](.kiro/specs/archived/enhanced-validation-system/requirements.md)
  - [design.md](.kiro/specs/archived/enhanced-validation-system/design.md)
  - [tasks.md](.kiro/specs/archived/enhanced-validation-system/tasks.md)

### Active Specs

**Location**: `.kiro/specs/`

No active specs currently. Next priority: Performance Telemetry.

### Spec Development Order

See **[SPEC_ORDER.md](.kiro/specs/SPEC_ORDER.md)** for the recommended implementation sequence.

## Character Classes

### Warrior
- **Role**: Melee tank with area damage
- **Combat Style**: Close-quarters with cleave attacks
- **Resource**: Rage (builds with attacks and damage taken)

### Ranger
- **Role**: Ranged DPS specialist
- **Combat Style**: Projectile-based attacks with piercing
- **Resource**: Focus (regenerates over time)

### Mage
- **Role**: Elemental caster with area effects
- **Combat Style**: Spell-casting with mana management
- **Resource**: Mana (regenerates slowly)

### Necromancer
- **Role**: Summoner with minion control
- **Combat Style**: Minion-based combat with life manipulation
- **Resource**: Soul Energy (gained from kills)

## Technical Stack

- **Engine**: Unreal Engine 5.6
- **Language**: Modern C++ (C++20)
- **Build System**: Unreal Build Tool (UBT)
- **Rendering**: Paper2D for 2D sprite-based graphics
- **Input**: Enhanced Input System (UE5 standard)
- **AI**: Behavior Trees with AI Perception
- **UI**: UMG (Unreal Motion Graphics)
- **Audio**: Unreal Audio Engine with spatial audio support

## Module Structure

```
Source/
├── DelveDeep/          # Main game module (DELVEDEEP_API)
│   ├── Public/         # Header files
│   └── Private/        # Implementation files
│       └── Tests/      # Unit tests
├── DelveDeepCore/      # Shared utilities (planned)
├── DelveDeepUI/        # UI systems (planned)
└── DelveDeepAudio/     # Audio systems (planned)
```

## Content Structure

```
Content/
└── Data/                 # Configuration data assets
    ├── Characters/       # Character data assets
    ├── Monsters/         # Monster data tables
    ├── Upgrades/         # Upgrade data assets
    ├── Weapons/          # Weapon data assets
    └── Abilities/        # Ability data assets
```

## Development Workflow

1. **Requirements**: Define feature requirements with acceptance criteria
2. **Design**: Create comprehensive design documents
3. **Tasks**: Break down implementation into actionable tasks
4. **Implementation**: Execute tasks with validation and testing
5. **Documentation**: Update system documentation

## Contributing

### Code Quality Checklist

- [ ] Follows UE5 naming conventions
- [ ] Blueprint-ready design
- [ ] Automated tests for core functionality
- [ ] Doxygen-style comments for public APIs
- [ ] System documentation updated
- [ ] Performance profiled and optimized
- [ ] Validation comprehensive

### Performance Checklist

- [ ] Tick disabled when not needed
- [ ] Object pooling for frequently spawned objects
- [ ] Containers pre-allocated when size known
- [ ] SCOPE_CYCLE_COUNTER added for critical code
- [ ] Performance targets met

### Validation Checklist

- [ ] FValidationContext used for validation
- [ ] Data validated in PostLoad() for data assets
- [ ] Detailed error messages with context
- [ ] Validation failures logged appropriately

## Project Status

**Current Phase**: Phase 1 - Core Foundation (4/5 Complete)

**Completed**:
- ✅ Data-Driven Configuration System (October 23, 2025)
- ✅ Enhanced Validation System (October 23, 2025)
- ✅ Centralized Event System (October 24, 2025)
- ✅ Performance Telemetry System (October 24, 2025)

**In Progress**:
- None

**Next Up**:
- Automated Testing Framework

## Quick Reference

### Important Files

- **Project File**: `DelveDeep.uproject`
- **Build Configuration**: `Source/DelveDeep/DelveDeep.Build.cs`
- **Main Module Header**: `Source/DelveDeep/Public/DelveDeep.h`
- **Configuration Manager**: `Source/DelveDeep/Public/DelveDeepConfigurationManager.h`
- **Validation System**: `Source/DelveDeep/Public/DelveDeepValidation.h`

### Key Directories

- **Source Code**: `Source/DelveDeep/`
- **Documentation**: `Documentation/`
- **Specifications**: `.kiro/specs/`
- **Development Guidelines**: `.kiro/steering/`
- **Tests**: `Source/DelveDeep/Private/Tests/`

### External Resources

- **Unreal Engine 5.6 Documentation**: https://docs.unrealengine.com/5.6/
- **C++20 Reference**: https://en.cppreference.com/w/cpp/20
- **Paper2D Documentation**: https://docs.unrealengine.com/5.6/paper-2d/

---

**Last Updated**: October 24, 2025

**Project Status**: Phase 1 Foundation - 4/5 Systems Complete

**Completed Systems**: Data-Driven Configuration (Oct 23), Enhanced Validation (Oct 23), Centralized Event System (Oct 24), Performance Telemetry (Oct 24)

**Target Platform**: Windows, macOS (future: Linux, Console)

**Performance Achieved**: <100ms initialization, <1ms queries, >95% cache hit rate, <1ms event broadcasts

**Performance Target**: 60+ FPS gameplay, sub-100ms save/load times, sub-millisecond data queries
