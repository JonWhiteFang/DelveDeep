# Data-Driven Configuration System

## Overview

The DelveDeep Data-Driven Configuration System provides a comprehensive, high-performance solution for managing game configuration data through UDataAsset classes and UDataTable structures. The system enables designers to balance the game without code changes while maintaining sub-millisecond query performance and comprehensive validation.

## Status

**✓ Complete** - All requirements implemented and tested

## Core Components

### 1. FValidationContext

A Blueprint-compatible struct that tracks validation errors and warnings with detailed context information.

**Location**: `Source/DelveDeep/Public/DelveDeepValidation.h`

**Key Features**:
- Error and warning tracking with automatic logging
- Formatted report generation
- System and operation context tracking
- Blueprint integration support

**Usage Example**:
```cpp
FValidationContext Context;
Context.SystemName = TEXT("Configuration");
Context.OperationName = TEXT("LoadCharacterData");

if (Health <= 0.0f)
{
    Context.AddError(TEXT("Health must be positive"));
}

if (!Context.IsValid())
{
    UE_LOG(LogDelveDeepConfig, Error, TEXT("\n%s"), *Context.GetReport());
}
```

### 2. UDelveDeepConfigurationManager

A UGameInstanceSubsystem that provides centralized access to all configuration data with caching and performance tracking.

**Location**: `Source/DelveDeep/Public/DelveDeepConfigurationManager.h`

**Key Features**:
- Automatic asset loading on initialization
- TMap-based caching for O(1) lookups
- Performance metrics tracking (cache hits, misses, query time)
- Hot-reload support in development builds
- Comprehensive validation on load
- Blueprint-callable query functions

**Query Functions**:
```cpp
const UDelveDeepCharacterData* GetCharacterData(FName CharacterName) const;
const FDelveDeepMonsterConfig* GetMonsterConfig(FName MonsterName) const;
const UDelveDeepUpgradeData* GetUpgradeData(FName UpgradeName) const;
const UDelveDeepWeaponData* GetWeaponData(FName WeaponName) const;
const UDelveDeepAbilityData* GetAbilityData(FName AbilityName) const;
```

### 3. Data Asset Classes

#### UDelveDeepCharacterData

Defines character class configuration including stats, resources, and combat parameters.

**Properties**:
- Display information (name, description)
- Base stats (health, damage, speed, armor)
- Resource system (max resource, regen rate)
- Combat parameters (attack speed, range)
- Asset references (starting weapon, abilities)

**Validation**:
- Health: 1-10000
- Damage: 1-1000
- Speed: 50-1000
- Armor: 0-100

#### UDelveDeepWeaponData

Defines weapon configuration including combat stats and projectile parameters.

**Properties**:
- Display information (name, description)
- Combat stats (damage, attack speed, range, damage type)
- Projectile parameters (speed, piercing, max pierce targets)
- Special ability reference

**Validation**:
- Damage > 0
- Attack speed > 0.1
- Range > 10

#### UDelveDeepAbilityData

Defines ability configuration including timing, costs, and effects.

**Properties**:
- Display information (name, description)
- Timing parameters (cooldown, cast time, duration)
- Resource cost and damage parameters
- AoE parameters (radius, affects allies)

**Validation**:
- Cooldown > 0.1
- Resource cost >= 0
- Damage multiplier >= 0

#### UDelveDeepUpgradeData

Defines upgrade configuration including costs, scaling, and stat modifications.

**Properties**:
- Display information (name, description)
- Cost parameters (base cost, scaling factor, max level)
- Stat modifications (health, damage, speed, armor)
- Required upgrades (dependencies)

**Validation**:
- Base cost > 0
- Scaling factor: 1.0-10.0
- Max level > 0

**Cost Calculation**:
```cpp
int32 CalculateCostForLevel(int32 Level) const
{
    return FMath::RoundToInt(BaseCost * FMath::Pow(CostScalingFactor, Level));
}
```

### 4. Data Table Structures

#### FDelveDeepMonsterConfig

Defines monster configuration in data tables for bulk data management.

**Properties**:
- Display information (name, description)
- Stats (health, damage, speed, armor)
- AI parameters (detection range, attack range, behavior type)
- Reward parameters (coin drops, experience)

**Validation**:
- Health > 0
- Damage >= 0
- All ranges positive

## Directory Structure

All configuration data must follow the standard directory structure:

```
Content/Data/
├── Characters/          # Character data assets (DA_Character_*)
├── Monsters/           # Monster data tables (DT_Monster_*)
├── Upgrades/           # Upgrade data assets (DA_Upgrade_*)
├── Weapons/            # Weapon data assets (DA_Weapon_*)
└── Abilities/          # Ability data assets (DA_Ability_*)
```

**Naming Conventions**:
- Data Assets: `DA_[Category]_[Name]`
- Data Tables: `DT_[Category]_[Name]`

See [ContentDirectoryStructure.md](ContentDirectoryStructure.md) for complete guidelines.

## Performance Characteristics

The system is optimized for high-performance queries:

### Initialization
- **Target**: < 100ms for 100+ assets
- **Actual**: Consistently meets target
- **Method**: Parallel asset loading with validation

### Single Query
- **Target**: < 1ms per query
- **Actual**: Consistently meets target
- **Method**: TMap-based O(1) cache lookups

### Bulk Queries
- **Target**: < 1ms average for 1000 queries
- **Actual**: Consistently meets target
- **Method**: Optimized cache access patterns

### Cache Hit Rate
- **Target**: > 95% for repeated queries
- **Actual**: Consistently exceeds target
- **Method**: Persistent caching across level transitions

See [Performance-Testing.md](Performance-Testing.md) for detailed performance testing guide.

## Hot-Reload Support

In development builds (!UE_BUILD_SHIPPING), the system supports hot-reloading of modified assets:

**Features**:
- Automatic detection of asset modifications
- Asset reload within 2 seconds
- Re-validation of reloaded assets
- Notification events on reload completion

**Implementation**:
```cpp
#if !UE_BUILD_SHIPPING
void SetupHotReload()
{
    FAssetRegistryModule& AssetRegistryModule = 
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    
    AssetReloadHandle = AssetRegistryModule.Get().OnAssetUpdated().AddUObject(
        this, &UDelveDeepConfigurationManager::OnAssetReloaded);
}
#endif
```

## Console Commands

### DelveDeep.ValidateAllData

Validates all loaded configuration data and generates a comprehensive report.

**Usage**: `DelveDeep.ValidateAllData`

**Output**: Validation report with all errors and warnings

### DelveDeep.ShowConfigStats

Displays current performance statistics including cache hits, misses, and average query time.

**Usage**: `DelveDeep.ShowConfigStats`

**Output**:
```
=== Configuration Manager Statistics ===
Total Queries: 1523
Cache Hits: 1487 (97.6%)
Cache Misses: 36 (2.4%)
Average Query Time: 0.23ms
```

### DelveDeep.ListLoadedAssets

Lists all cached configuration assets by type.

**Usage**: `DelveDeep.ListLoadedAssets`

**Output**: Asset names organized by category

### DelveDeep.ReloadConfigData

Forces a complete reload of all configuration data.

**Usage**: `DelveDeep.ReloadConfigData`

**Effect**: Clears caches and reloads all assets

### DelveDeep.DumpConfigData [AssetName]

Dumps all properties of a specified asset to the log.

**Usage**: `DelveDeep.DumpConfigData Warrior`

**Output**: Detailed property dump for debugging

### DelveDeep.CreateExampleData

Creates example data assets for testing purposes.

**Usage**: `DelveDeep.CreateExampleData`

**Output**: Creates Warrior, Sword, Cleave, HealthBoost, and Monster configs

## Testing

### Automated Tests

**Location**: `Source/DelveDeep/Private/Tests/`

**Test Files**:
- **ConfigurationManagerTests.cpp**: Configuration manager functionality
- **IntegrationTests.cpp**: System integration tests
- **PerformanceTests.cpp**: Performance benchmarks
- **ValidationTests.cpp**: Validation system tests

**Running Tests**:
```bash
# Run all configuration tests
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep.Config" -unattended

# Run performance tests
UnrealEditor-Cmd.exe DelveDeep -ExecCmds="Automation RunTests DelveDeep.Performance" -unattended
```

### Test Coverage

- ✅ Asset caching on first query
- ✅ Cached asset returned on subsequent queries
- ✅ Cache hit rate tracking accuracy
- ✅ Data table lookup by name
- ✅ Invalid name returns nullptr
- ✅ Query performance under target thresholds
- ✅ Upgrade cost calculation at various levels
- ✅ Asset reference resolution
- ✅ Validation error detection
- ✅ Hot-reload functionality

## Integration with Other Systems

The configuration system is designed to integrate with:

### Character System (Planned)
```cpp
// Load character configuration
const UDelveDeepCharacterData* CharacterData = 
    ConfigManager->GetCharacterData("Warrior");

// Apply stats to character
Character->SetMaxHealth(CharacterData->BaseHealth);
Character->SetBaseDamage(CharacterData->BaseDamage);
```

### Combat System (Planned)
```cpp
// Load weapon configuration
const UDelveDeepWeaponData* WeaponData = 
    ConfigManager->GetWeaponData("Sword");

// Apply weapon stats
Weapon->SetDamage(WeaponData->BaseDamage);
Weapon->SetAttackSpeed(WeaponData->AttackSpeed);
```

### Progression System (Planned)
```cpp
// Load upgrade configuration
const UDelveDeepUpgradeData* UpgradeData = 
    ConfigManager->GetUpgradeData("HealthBoost");

// Calculate upgrade cost
int32 Cost = UpgradeData->CalculateCostForLevel(PlayerLevel);
```

### Monster AI (Planned)
```cpp
// Load monster configuration
const FDelveDeepMonsterConfig* MonsterConfig = 
    ConfigManager->GetMonsterConfig("Goblin");

// Apply monster stats
Monster->SetHealth(MonsterConfig->Health);
Monster->SetDamage(MonsterConfig->Damage);
```

## Best Practices

### 1. Use Configuration Manager for All Data Access

❌ **Don't** load assets directly:
```cpp
UDelveDeepCharacterData* Data = LoadObject<UDelveDeepCharacterData>(
    nullptr, TEXT("/Game/Data/Characters/DA_Character_Warrior"));
```

✅ **Do** use the configuration manager:
```cpp
const UDelveDeepCharacterData* Data = 
    ConfigManager->GetCharacterData("Warrior");
```

### 2. Validate Data on Load

❌ **Don't** skip validation:
```cpp
void PostLoad()
{
    Super::PostLoad();
    // No validation
}
```

✅ **Do** validate in PostLoad():
```cpp
void PostLoad()
{
    Super::PostLoad();
    
    FValidationContext Context;
    if (!Validate(Context))
    {
        UE_LOG(LogDelveDeepConfig, Error, TEXT("\n%s"), *Context.GetReport());
    }
}
```

### 3. Use TSoftObjectPtr for Asset References

❌ **Don't** use hard object pointers:
```cpp
UPROPERTY(EditAnywhere)
UDelveDeepWeaponData* StartingWeapon;
```

✅ **Do** use soft object pointers:
```cpp
UPROPERTY(EditAnywhere)
TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;
```

### 4. Follow Directory Structure

❌ **Don't** place assets outside standard directories:
```
Content/MyFolder/CharacterWarrior.uasset
```

✅ **Do** follow the standard structure:
```
Content/Data/Characters/DA_Character_Warrior.uasset
```

### 5. Use Descriptive Names

❌ **Don't** use generic names:
```
DA_Char1
DA_WeaponData
DA_Test
```

✅ **Do** use descriptive names:
```
DA_Character_Warrior
DA_Weapon_FlamingSword
DA_Ability_Cleave
```

## Troubleshooting

### Slow Initialization

**Symptom**: Initialization exceeds 100ms

**Solutions**:
1. Check asset count with `DelveDeep.ListLoadedAssets`
2. Profile with `stat DelveDeepConfig`
3. Consider lazy loading non-critical assets

### Slow Queries

**Symptom**: Queries exceed 1ms

**Solutions**:
1. Check cache hit rate with `DelveDeep.ShowConfigStats`
2. Verify assets are being cached correctly
3. Profile with `stat DelveDeepConfig`

### Low Cache Hit Rate

**Symptom**: Cache hit rate below 95%

**Solutions**:
1. Verify asset names are consistent
2. Check for typos in asset references
3. Ensure assets exist in expected directories

### Validation Failures

**Symptom**: Assets fail validation on load

**Solutions**:
1. Check validation report for specific errors
2. Verify property values are within valid ranges
3. Ensure all required references are assigned

## Future Enhancements

Potential improvements for future versions:

1. **Async Asset Loading**: Load assets asynchronously during initialization
2. **Asset Streaming**: Stream assets on-demand to reduce memory
3. **Cache Warming**: Pre-load commonly used assets
4. **Query Batching**: Batch multiple queries together
5. **Validation Caching**: Cache validation results for unchanged assets
6. **Memory Pooling**: Pool validation context objects

## Related Documentation

- **[ValidationSystem.md](ValidationSystem.md)**: Validation infrastructure details
- **[ContentDirectoryStructure.md](ContentDirectoryStructure.md)**: Asset organization guide
- **[Performance-Testing.md](Performance-Testing.md)**: Performance testing guide
- **[Task1-ValidationInfrastructure.md](../Implementation/Task1-ValidationInfrastructure.md)**: Implementation notes
- **[Task11-ExampleDataAssets.md](../Implementation/Task11-ExampleDataAssets.md)**: Example data assets

## Summary

The Data-Driven Configuration System provides a robust, high-performance foundation for managing game configuration data. With comprehensive validation, sub-millisecond queries, and hot-reload support, the system enables rapid iteration and balancing without code changes.

**Key Achievements**:
- ✅ Sub-100ms initialization
- ✅ Sub-millisecond queries
- ✅ >95% cache hit rate
- ✅ Comprehensive validation
- ✅ Hot-reload support
- ✅ Blueprint integration
- ✅ Extensive testing
- ✅ Complete documentation

The system is production-ready and serves as the foundation for all future game systems.
