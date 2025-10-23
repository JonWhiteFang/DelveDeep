# DelveDeep Development Guidelines

This directory contains comprehensive development guidelines and best practices for the DelveDeep project. All guidelines are informed by real implementation experience from completed systems.

## Quick Reference

### Completed Systems ✓

**Data-Driven Configuration System** - Reference implementation demonstrating all best practices:
- [data-assets.md](data-assets.md) - Data asset patterns and organization
- [subsystems.md](subsystems.md) - Subsystem architecture and lifecycle
- [error-handling.md](error-handling.md) - Validation and error handling
- [testing.md](testing.md) - Testing strategies and coverage
- [code-review.md](code-review.md) - Code review checklist

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

### Phase 1: Core Foundation

✅ **Data-Driven Configuration System** (Complete)
- FValidationContext for error/warning tracking
- UDelveDeepConfigurationManager subsystem
- Data asset classes (Character, Weapon, Ability, Upgrade)
- Data table structures (Monster configuration)
- Performance optimization (<100ms init, <1ms queries, >95% cache hit rate)
- Hot-reload support (development builds)
- Comprehensive validation and testing
- Console commands for debugging

**Key Achievements:**
- Sub-100ms initialization for 100+ assets
- Sub-millisecond query performance
- >95% cache hit rate
- Comprehensive validation with FValidationContext
- Hot-reload within 2 seconds
- Full Blueprint integration
- Extensive test coverage

**Documentation:**
- [ValidationSystem.md](../Documentation/Systems/ValidationSystem.md)
- [ContentDirectoryStructure.md](../Documentation/Systems/ContentDirectoryStructure.md)
- [Performance-Testing.md](../Documentation/Systems/Performance-Testing.md)
- [DataDrivenConfiguration.md](../Documentation/Systems/DataDrivenConfiguration.md)

### Next Priorities

🔄 **Centralized Event System** (Planned)
- Gameplay event bus using GameplayTags
- Loose coupling between systems
- Event-driven architecture

🔄 **Automated Testing Framework** (Partial)
- Configuration system tests complete
- Expand to other systems as implemented

🔄 **Character System Foundation** (Planned)
- Component-based architecture
- Four character classes
- Integration with configuration system

## Using These Guidelines

### For New Features

1. **Review relevant guidelines** before starting implementation
2. **Follow established patterns** from completed systems
3. **Use configuration system as reference** for architecture decisions
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
- Target: 60+ FPS, <100ms init, <1ms queries

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
│   └── DataDrivenConfiguration.md    # System overview ✓
└── Implementation/
    ├── Task1-ValidationInfrastructure.md  # Implementation notes ✓
    └── Task11-ExampleDataAssets.md        # Example data ✓
```

## Contributing

When implementing new systems:

1. **Follow established patterns** from configuration system
2. **Use FValidationContext** for validation
3. **Implement as subsystem** if game-wide functionality
4. **Write comprehensive tests**
5. **Provide console commands** for debugging
6. **Update documentation** in Documentation/Systems/
7. **Update this README** with implementation status

## Questions?

- Check [DOCUMENTATION_INDEX.md](../../DOCUMENTATION_INDEX.md) for complete documentation
- Review completed configuration system for reference implementation
- Consult specific steering documents for detailed guidance

---

**Last Updated**: October 23, 2025

**Status**: Phase 1 - Data-Driven Configuration System Complete

**Next**: Centralized Event System, Character System Foundation
