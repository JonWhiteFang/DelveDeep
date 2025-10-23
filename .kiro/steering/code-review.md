# Code Review Checklist

**✓ Reference Implementation**: DelveDeep Configuration System demonstrates all best practices.

## Pre-Commit Checklist

Before committing code, verify:

### Performance
- [ ] Tick disabled unless specifically needed (`PrimaryActorTick.bCanEverTick = false`)
- [ ] Tick interval set for non-critical updates if ticking is required
- [ ] Containers pre-allocated when size is known (`Reserve()`)
- [ ] Object pooling used for frequently spawned objects
- [ ] `SCOPE_CYCLE_COUNTER` added for performance-critical code

### Validation

**Configuration System Examples:**

- [x] All input parameters validated before processing ✓
  - All data assets validate in PostLoad()
  - Query functions validate parameters

- [x] Null pointer checks added for all UObject references ✓
  - IsValid() checks before dereferencing
  - Const pointer returns prevent modification

- [x] `FValidationContext` used for comprehensive validation ✓
  - All data assets use FValidationContext
  - Formatted reports with detailed context

- [x] Data assets implement `PostLoad()` validation ✓
  - Character, Weapon, Ability, Upgrade, Monster configs
  - Automatic validation on load

- [x] Error messages include context (what, why, where) ✓
  - SystemName and OperationName in context
  - Expected ranges in error messages

### Blueprint Integration
- [ ] All classes use `DELVEDEEP_API` macro
- [ ] Classes marked with `BlueprintType` and `Blueprintable` where appropriate
- [ ] Properties use appropriate specifiers (`EditAnywhere`, `BlueprintReadOnly`, etc.)
- [ ] Functions use appropriate specifiers (`BlueprintCallable`, `BlueprintPure`)
- [ ] Expensive const functions marked with `BlueprintPure = false`
- [ ] Categories added to all `UFUNCTION` and `UPROPERTY` declarations
- [ ] Meta specifiers used for better Editor experience (`ClampMin`, `ClampMax`, etc.)

### Memory Management

**Configuration System Examples:**

- [x] `TSoftObjectPtr` used for asset references instead of hard pointers ✓
  - All asset references use TSoftObjectPtr
  - Reduces memory footprint significantly

- [x] `UPROPERTY()` used for cached data to prevent garbage collection ✓
  - All cache maps use UPROPERTY()
  - Prevents premature garbage collection

- [x] Resources cleaned up in destructors/`Deinitialize()` ✓
  - Hot-reload callbacks unregistered
  - All caches cleared properly

- [x] No memory leaks from manual allocations ✓
  - All objects managed by UE5 GC
  - No raw pointers or manual allocations

### Naming Conventions
- [ ] Classes use correct prefixes (`A`, `U`, `F`, `E`, `I`)
- [ ] Files named `DelveDeepClassName.h/.cpp`
- [ ] Data assets use `DA_` prefix
- [ ] Data tables use `DT_` prefix
- [ ] Variables use descriptive names

### Logging
- [ ] Appropriate log severity used (Error, Warning, Display, Verbose)
- [ ] Log messages include context and relevant values
- [ ] Dedicated log category used (`LogDelveDeepConfig`, `LogDelveDeepCombat`, etc.)

### Documentation
- [ ] Public APIs have Doxygen-style comments
- [ ] Complex logic has explanatory comments
- [ ] Function parameters documented
- [ ] Usage examples provided for non-obvious functionality

## Common UE5 Pitfalls to Avoid

### Memory & Pointers
❌ **Don't** use raw pointers for UObject references without validation
❌ **Don't** forget to use `UPROPERTY()` for UObject pointers
❌ **Don't** use hard object pointers for asset references
❌ **Don't** store pointers to objects that might be garbage collected

✅ **Do** use `IsValid()` to check UObject validity
✅ **Do** use `TSoftObjectPtr` for asset references
✅ **Do** use `UPROPERTY()` to prevent garbage collection
✅ **Do** validate pointers before dereferencing

### Performance
❌ **Don't** leave tick enabled when not needed
❌ **Don't** perform expensive operations every frame
❌ **Don't** create objects in tight loops without pooling
❌ **Don't** use `TArray::Add()` in loops without `Reserve()`

✅ **Do** disable tick by default
✅ **Do** use tick intervals for non-critical updates
✅ **Do** implement object pooling for frequently spawned objects
✅ **Do** pre-allocate containers when size is known

### Blueprint Integration
❌ **Don't** expose raw pointers without validation
❌ **Don't** make expensive const functions automatically pure
❌ **Don't** forget `DELVEDEEP_API` macro for public classes
❌ **Don't** forget to add Category to UFUNCTION/UPROPERTY

✅ **Do** validate all Blueprint-callable function inputs
✅ **Do** use `BlueprintPure = false` for expensive const functions
✅ **Do** use appropriate specifiers for Blueprint exposure
✅ **Do** provide meaningful return values and error context

### Data Assets
❌ **Don't** hardcode values that should be data-driven
❌ **Don't** skip validation in `PostLoad()`
❌ **Don't** place data assets outside standard directory structure
❌ **Don't** use hard references for optional assets

✅ **Do** move configuration to data assets
✅ **Do** validate data on load
✅ **Do** follow naming conventions (DA_, DT_)
✅ **Do** use `TSoftObjectPtr` for asset references

### Subsystems
❌ **Don't** create manual singletons
❌ **Don't** forget to call `Super::Initialize()` and `Super::Deinitialize()`
❌ **Don't** assume subsystems initialize in a specific order
❌ **Don't** store raw pointers to other subsystems

✅ **Do** use `UGameInstanceSubsystem` for game-wide functionality
✅ **Do** validate subsystem availability before use
✅ **Do** clean up resources in `Deinitialize()`
✅ **Do** use `GetSubsystem<T>()` for accessing other subsystems

## Code Quality Red Flags

### Architecture Issues
- Manual singleton implementations
- God classes with too many responsibilities
- Tight coupling between unrelated systems
- Circular dependencies
- Missing abstraction layers

### Performance Issues
- Tick enabled unnecessarily
- Expensive operations in tick functions
- No object pooling for frequently spawned objects
- Unoptimized container usage
- Missing `SCOPE_CYCLE_COUNTER` in critical paths

### Maintainability Issues
- Magic numbers instead of named constants
- Unclear variable/function names
- Missing or outdated comments
- Inconsistent naming conventions
- Duplicate code that should be refactored

### Safety Issues
- Unchecked pointer dereferences
- Missing input validation
- No error handling
- Assumptions about initialization order
- Race conditions in multi-threaded code

## Code Review Questions

### Functionality
- Does the code do what it's supposed to do?
- Are edge cases handled correctly?
- Is error handling comprehensive?
- Are there any logical errors?

### Performance
- Is tick disabled when not needed?
- Are expensive operations optimized?
- Is memory usage reasonable?
- Are there any obvious bottlenecks?

### Maintainability
- Is the code easy to understand?
- Are names descriptive and consistent?
- Is the code properly documented?
- Is the code modular and reusable?

### Blueprint Integration
- Are classes properly exposed to Blueprint?
- Are functions and properties categorized?
- Are expensive operations marked correctly?
- Is the Blueprint API intuitive?

### Testing
- Can the code be easily tested?
- Are there unit tests for critical functionality?
- Are console commands provided for manual testing?
- Is validation comprehensive?

## Automated Checks

**Configuration System Test Results:**

### Static Analysis
```bash
# Run clang-tidy
clang-tidy Source/**/*.cpp -- -std=c++20

# Check formatting
clang-format --dry-run --Werror Source/**/*.cpp
```

### Build Checks
```bash
# Build with warnings as errors
UnrealBuildTool -WarningsAsErrors

# Run static code analysis
UnrealBuildTool -StaticAnalyzer=VisualCpp
```

### Test Execution
```bash
# Run all unit tests (✓ Configuration system passes all tests)
UnrealEditor-Cmd.exe ProjectName -ExecCmds="Automation RunTests DelveDeep" -unattended

# Run validation commands (✓ Implemented)
DelveDeep.ValidateAllSystems
DelveDeep.ValidateAllData
```

**Configuration System Test Coverage:**
- ✓ Asset caching tests
- ✓ Cache hit rate tests
- ✓ Query performance tests
- ✓ Validation tests
- ✓ Integration tests
- ✓ Hot-reload tests

## Documentation Requirements

### Public APIs
```cpp
/**
 * Applies damage to the target actor.
 * 
 * @param Target The actor to damage (must be valid)
 * @param Damage The amount of damage to apply (must be positive)
 * @return True if damage was applied successfully, false otherwise
 * 
 * @note This function validates the target and damage value before applying damage.
 * @warning This function broadcasts damage events to all listeners.
 */
UFUNCTION(BlueprintCallable, Category = "DelveDeep|Combat")
bool ApplyDamage(AActor* Target, float Damage);
```

### Complex Logic
```cpp
// Calculate upgrade cost using exponential scaling
// Formula: FinalCost = BaseCost * (ScalingFactor ^ UpgradeLevel)
// Example: BaseCost=100, ScalingFactor=1.5, Level=3 -> Cost=337.5
int32 CalculateUpgradeCost(int32 Level) const
{
    return FMath::RoundToInt(BaseCost * FMath::Pow(CostScalingFactor, Level));
}
```

### System Documentation
- Update `DOCUMENTATION_INDEX.md` when implementing new systems
- Create system documentation in `Documentation/` appropriate category
- Include architecture diagrams for complex systems
- Provide usage examples and integration guides

## Final Checklist

**Configuration System Completion Status:**

Before marking code as ready for review:

- [x] Code compiles without warnings ✓
- [x] All tests pass ✓
- [x] Console commands work as expected ✓
- [x] Performance targets met ✓
  - <100ms initialization
  - <1ms queries
  - >95% cache hit rate
- [x] Documentation updated ✓
  - ValidationSystem.md
  - ContentDirectoryStructure.md
  - Performance-Testing.md
  - DataDrivenConfiguration.md
- [x] Pre-commit checklist completed ✓
- [x] No TODO comments left unresolved ✓
- [x] Code follows project conventions ✓
- [x] Blueprint integration tested (if applicable) ✓
- [x] Validation comprehensive and tested ✓
