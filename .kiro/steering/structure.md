# Project Structure & Conventions

## Directory Organization

```
DelveDeep/
├── Source/                    # C++ source code
│   ├── DelveDeep/            # Main game module
│   │   ├── Public/           # Header files (.h)
│   │   └── Private/          # Implementation files (.cpp)
│   ├── DelveDeepCore/        # Shared utilities
│   ├── DelveDeepUI/          # UI systems
│   └── DelveDeepAudio/       # Audio systems
├── Content/                   # Game assets (provided externally)
│   └── Data/                 # Configuration data assets
│       ├── Characters/       # Character data assets
│       ├── Monsters/         # Monster data tables
│       ├── Upgrades/         # Upgrade data assets
│       ├── Weapons/          # Weapon data assets
│       └── Abilities/        # Ability data assets
├── Config/                    # Engine and game configuration
├── Documentation/             # System documentation
└── .kiro/                    # Kiro IDE configuration
    ├── specs/                # Feature specifications
    └── steering/             # AI assistant guidance
```

## Naming Conventions (Mandatory)

### C++ Classes
- **Actors**: `A` prefix (e.g., `ADelveDeepPlayerCharacter`)
- **Objects/Components**: `U` prefix (e.g., `UDelveDeepCombatComponent`)
- **Structs**: `F` prefix (e.g., `FDelveDeepPlayerStats`)
- **Enums**: `E` prefix (e.g., `EDelveDeepCharacterClass`)
- **Interfaces**: `I` prefix (e.g., `IDelveDeepInteractable`)

### Files
- Header files: `DelveDeepClassName.h`
- Implementation files: `DelveDeepClassName.cpp`
- All public classes use `DELVEDEEP_API` macro

### Data Assets
- **Data Assets**: `DA_[Category]_[Name]` (e.g., `DA_Character_Warrior`)
- **Data Tables**: `DT_[Category]_[Name]` (e.g., `DT_Monster_Configs`)

## Architecture Patterns

### Component-Based Design
Prefer composition over inheritance. Use component-based architecture with `UDelveDeepCharacterComponent` as base class for character-specific systems.

### Blueprint-Ready Classes
All classes must be Blueprint-compatible:
```cpp
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep")
class DELVEDEEP_API ADelveDeepActor : public AActor
{
    GENERATED_BODY()
    
    UFUNCTION(BlueprintCallable, Category = "DelveDeep")
    void PerformAction();
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DelveDeep")
    float ActionValue = 100.0f;
};
```

### Data-Driven Configuration
Move hardcoded values to UDataAsset classes and UDataTable structures. Use centralized `UDelveDeepConfigurationManager` subsystem for access.

### Event-Driven Communication
Use centralized event system (`UDelveDeepEventSubsystem`) for loose coupling between systems. Avoid direct dependencies where possible.

### Validation Pattern
Use `FValidationContext` for comprehensive validation with error/warning tracking:
```cpp
bool ValidateGameplayAction(AActor* Actor, int32 Value, FValidationContext& Context)
{
    if (!IsValid(Actor))
    {
        Context.AddError(TEXT("Invalid actor reference"));
        return false;
    }
    return true;
}
```

## Performance Requirements

### Tick Optimization
- Always disable ticking unless specifically needed: `PrimaryActorTick.bCanEverTick = false`
- Use tick intervals for non-critical updates: `PrimaryActorTick.TickInterval = 0.2f`

### Memory Management
- Pre-allocate containers when size is known: `Items.Reserve(ExpectedMaxSize)`
- Prefer UE5 containers (`TArray`, `TMap`, `TSet`) over STL
- Use object pooling for frequently spawned objects (projectiles, effects, monsters)
- Use `TSoftObjectPtr` for asset references to reduce memory footprint

### Profiling
Use `SCOPE_CYCLE_COUNTER` for performance-critical code:
```cpp
DECLARE_STATS_GROUP(TEXT("DelveDeep"), STATGROUP_DelveDeep, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Combat System"), STAT_CombatSystem, STATGROUP_DelveDeep);

void UDelveDeepCombatComponent::ProcessCombat()
{
    SCOPE_CYCLE_COUNTER(STAT_CombatSystem);
    // Combat logic
}
```

## Logging

Use dedicated log category:
```cpp
DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All);

UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load: %s"), *Name);
UE_LOG(LogDelveDeepConfig, Warning, TEXT("Using fallback value"));
UE_LOG(LogDelveDeepConfig, Display, TEXT("System initialized"));
```

## Documentation

- Update `DOCUMENTATION_INDEX.md` when implementing new systems
- Create system documentation in `Documentation/` appropriate category
- Use Doxygen-style comments for public APIs
- Include usage examples in header comments
