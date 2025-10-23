# Design Document

## Overview

The Data-Driven Configuration System provides a robust, performant, and designer-friendly infrastructure for managing all game parameters in DelveDeep. Built on Unreal Engine 5.6's UDataAsset and UDataTable systems, this design enables rapid iteration on game balance without code changes while maintaining type safety, validation, and Blueprint integration.

The system consists of three primary layers:
1. **Data Asset Layer**: Strongly-typed UDataAsset classes for complex configurations
2. **Data Table Layer**: Row-based UDataTable structures for bulk data (monsters, items)
3. **Management Layer**: Centralized UGameInstanceSubsystem for caching and access

This architecture ensures sub-millisecond data access, comprehensive validation, and seamless integration with all other DelveDeep systems.

## Architecture

### System Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Game Instance                             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │     UDelveDeepConfigurationManager (Subsystem)        │  │
│  │                                                         │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────┐  │  │
│  │  │ Asset Cache │  │ Table Cache │  │  Validation  │  │  │
│  │  │   (TMap)    │  │   (TMap)    │  │   Context    │  │  │
│  │  └─────────────┘  └─────────────┘  └──────────────┘  │  │
│  │                                                         │  │
│  │  ┌──────────────────────────────────────────────────┐ │  │
│  │  │         Hot Reload Manager (Dev Only)            │ │  │
│  │  └──────────────────────────────────────────────────┘ │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Provides Access To
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Data Asset Classes                        │
│                                                               │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │ UDelveDeepChar-  │  │ UDelveDeepUpgrade│                │
│  │   acterData      │  │      Data        │                │
│  └──────────────────┘  └──────────────────┘                │
│                                                               │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │ UDelveDeepWeapon │  │ UDelveDeepAbility│                │
│  │      Data        │  │      Data        │                │
│  └──────────────────┘  └──────────────────┘                │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Stored In
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Data Table Structures                     │
│                                                               │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │ FDelveDeepMonster│  │ FDelveDeepItem   │                │
│  │     Config       │  │     Config       │                │
│  └──────────────────┘  └──────────────────┘                │
└─────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

**UDelveDeepConfigurationManager**
- Singleton access point for all configuration data
- Manages asset and table caching with TMap containers
- Handles initialization and validation during game startup
- Provides Blueprint-callable query functions
- Manages hot-reloading in development builds
- Tracks performance metrics and statistics

**Data Asset Classes (UDataAsset derivatives)**
- Store complex, hierarchical configuration data
- Support asset references for relationships (e.g., weapon → ability)
- Provide type-safe property access with UPROPERTY macros
- Enable Editor-based editing with metadata specifiers
- Support Blueprint read access for UI and gameplay logic

**Data Table Structures (FTableRowBase derivatives)**
- Store bulk data with consistent schema (monsters, items, upgrades)
- Enable CSV import/export for external editing
- Provide fast row lookup by name or ID
- Support validation through row post-edit callbacks

## Components and Interfaces

### Core Classes

#### UDelveDeepConfigurationManager

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepConfigurationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Character data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const UDelveDeepCharacterData* GetCharacterData(FName CharacterName) const;

    // Monster data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const FDelveDeepMonsterConfig* GetMonsterConfig(FName MonsterName) const;

    // Upgrade data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const UDelveDeepUpgradeData* GetUpgradeData(FName UpgradeName) const;

    // Weapon data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const UDelveDeepWeaponData* GetWeaponData(FName WeaponName) const;

    // Ability data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const UDelveDeepAbilityData* GetAbilityData(FName AbilityName) const;

    // Validation
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    bool ValidateAllData(FString& OutReport);

    // Performance metrics
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    void GetPerformanceStats(int32& OutCacheHits, int32& OutCacheMisses, float& OutAvgQueryTime) const;

private:
    // Asset caches
    UPROPERTY()
    TMap<FName, UDelveDeepCharacterData*> CharacterDataCache;

    UPROPERTY()
    TMap<FName, UDelveDeepUpgradeData*> UpgradeDataCache;

    UPROPERTY()
    TMap<FName, UDelveDeepWeaponData*> WeaponDataCache;

    UPROPERTY()
    TMap<FName, UDelveDeepAbilityData*> AbilityDataCache;

    // Data table caches
    UPROPERTY()
    UDataTable* MonsterConfigTable;

    UPROPERTY()
    UDataTable* ItemConfigTable;

    // Performance tracking
    mutable int32 CacheHits;
    mutable int32 CacheMisses;
    mutable double TotalQueryTime;
    mutable int32 QueryCount;

    // Hot reload support (dev builds only)
#if !UE_BUILD_SHIPPING
    void SetupHotReload();
    void OnAssetReloaded(const FAssetData& AssetData);
    FDelegateHandle AssetReloadHandle;
#endif

    // Internal loading functions
    void LoadCharacterData();
    void LoadUpgradeData();
    void LoadWeaponData();
    void LoadAbilityData();
    void LoadDataTables();

    // Validation helpers
    bool ValidateCharacterData(const UDelveDeepCharacterData* Data, FValidationContext& Context) const;
    bool ValidateMonsterConfig(const FDelveDeepMonsterConfig* Config, FValidationContext& Context) const;
    bool ValidateUpgradeData(const UDelveDeepUpgradeData* Data, FValidationContext& Context) const;
    bool ValidateWeaponData(const UDelveDeepWeaponData* Data, FValidationContext& Context) const;
    bool ValidateAbilityData(const UDelveDeepAbilityData* Data, FValidationContext& Context) const;
};
```

#### UDelveDeepCharacterData

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepCharacterData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Display information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
    FText Description;

    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0", ClampMax = "10000.0"))
    float BaseHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float BaseArmor = 0.0f;

    // Resource system
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float MaxResource = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float ResourceRegenRate = 5.0f;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float BaseAttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
    float AttackRange = 100.0f;

    // Starting equipment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<TSoftObjectPtr<UDelveDeepAbilityData>> StartingAbilities;

    // Validation
    virtual void PostLoad() override;
    bool Validate(FValidationContext& Context) const;
};
```

#### FDelveDeepMonsterConfig

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepMonsterConfig : public FTableRowBase
{
    GENERATED_BODY()

    // Display information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText MonsterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText Description;

    // Base stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
    float Health = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
    float Damage = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
    float MoveSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
    float Armor = 0.0f;

    // AI behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "100.0"))
    float DetectionRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "10.0"))
    float AttackRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    FName AIBehaviorType = "Melee";

    // Loot and rewards
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
    int32 CoinDropMin = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
    int32 CoinDropMax = 5;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
    int32 ExperienceReward = 10;

    // Validation
    void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override;
    bool Validate(FValidationContext& Context) const;
};
```

#### UDelveDeepUpgradeData

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepUpgradeData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Display information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText UpgradeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
    FText Description;

    // Cost parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "1"))
    int32 BaseCost = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float CostScalingFactor = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "1"))
    int32 MaxLevel = 10;

    // Stat modifications
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float HealthModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float DamageModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float MoveSpeedModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    float ArmorModifier = 0.0f;

    // Dependencies
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
    TArray<TSoftObjectPtr<UDelveDeepUpgradeData>> RequiredUpgrades;

    // Helper function for cost calculation
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    int32 CalculateCostForLevel(int32 Level) const;

    // Validation
    virtual void PostLoad() override;
    bool Validate(FValidationContext& Context) const;
};
```

#### UDelveDeepWeaponData

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepWeaponData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Display information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText WeaponName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
    FText Description;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "1.0"))
    float BaseDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.1"))
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "10.0"))
    float Range = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FName DamageType = "Physical";

    // Projectile parameters (for ranged weapons)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
    float ProjectileSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
    bool bPiercing = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "1"))
    int32 MaxPierceTargets = 1;

    // Special abilities
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TSoftObjectPtr<UDelveDeepAbilityData> SpecialAbility;

    // Validation
    virtual void PostLoad() override;
    bool Validate(FValidationContext& Context) const;
};
```

#### UDelveDeepAbilityData

```cpp
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepAbilityData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Display information
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText AbilityName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
    FText Description;

    // Timing parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.1"))
    float Cooldown = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
    float CastTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
    float Duration = 0.0f;

    // Resource cost
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0.0"))
    float ResourceCost = 10.0f;

    // Damage parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
    FName DamageType = "Physical";

    // Area of effect
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0.0"))
    float AoERadius = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE")
    bool bAffectsAllies = false;

    // Validation
    virtual void PostLoad() override;
    bool Validate(FValidationContext& Context) const;
};
```

### Validation System

#### FValidationContext

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FValidationContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString OperationName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FString> ValidationWarnings;

    void AddError(const FString& Error);
    void AddWarning(const FString& Warning);
    bool IsValid() const { return ValidationErrors.Num() == 0; }
    FString GetReport() const;
    void Reset();
};
```

## Data Models

### Content Directory Structure

```
Content/
└── Data/
    ├── Characters/
    │   ├── DA_Character_Warrior.uasset
    │   ├── DA_Character_Ranger.uasset
    │   ├── DA_Character_Mage.uasset
    │   └── DA_Character_Necromancer.uasset
    ├── Monsters/
    │   └── DT_Monster_Configs.uasset
    ├── Upgrades/
    │   ├── DA_Upgrade_HealthBoost.uasset
    │   ├── DA_Upgrade_DamageBoost.uasset
    │   └── DA_Upgrade_SpeedBoost.uasset
    ├── Weapons/
    │   ├── DA_Weapon_Sword.uasset
    │   ├── DA_Weapon_Bow.uasset
    │   └── DA_Weapon_Staff.uasset
    └── Abilities/
        ├── DA_Ability_Cleave.uasset
        ├── DA_Ability_PiercingShot.uasset
        └── DA_Ability_Fireball.uasset
```

### Data Relationships

```
UDelveDeepCharacterData
    ├─> StartingWeapon (TSoftObjectPtr<UDelveDeepWeaponData>)
    └─> StartingAbilities (TArray<TSoftObjectPtr<UDelveDeepAbilityData>>)

UDelveDeepWeaponData
    └─> SpecialAbility (TSoftObjectPtr<UDelveDeepAbilityData>)

UDelveDeepUpgradeData
    └─> RequiredUpgrades (TArray<TSoftObjectPtr<UDelveDeepUpgradeData>>)
```

## Error Handling

### Validation Strategy

**Load-Time Validation**
- All data assets validate on PostLoad()
- Data tables validate on row import via OnPostDataImport()
- Configuration manager validates all data during Initialize()
- Validation failures log detailed errors with asset name, property, and expected range

**Runtime Validation**
- Null pointer checks before accessing cached data
- Range validation for numeric queries
- Type checking for template-based asset retrieval
- Performance validation (query time, cache hit rate)

### Error Recovery

**Missing Assets**
- Log warning with asset name and expected path
- Return nullptr for missing data assets
- Use default fallback values where appropriate
- Continue initialization rather than crash

**Invalid Data**
- Log error with specific validation failure details
- Clamp out-of-range values to valid ranges
- Disable hot-reload for corrupted assets
- Provide console command to re-validate after fixes

**Performance Issues**
- Log warning when query count exceeds threshold (1000/frame)
- Track cache miss rate and suggest preloading
- Profile slow data table lookups
- Recommend data structure optimization

### Logging Categories

```cpp
DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All);

// Usage examples:
UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load character data: %s"), *CharacterName.ToString());
UE_LOG(LogDelveDeepConfig, Warning, TEXT("Data asset outside standard directory: %s"), *AssetPath);
UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration system initialized: %d assets loaded"), AssetCount);
```

## Testing Strategy

### Unit Tests

**Data Asset Validation Tests**
```cpp
TEST(DelveDeepConfig, CharacterDataValidation)
{
    // Test valid character data passes validation
    // Test invalid health value fails validation
    // Test invalid damage value fails validation
    // Test missing weapon reference logs warning
}

TEST(DelveDeepConfig, UpgradeCostCalculation)
{
    // Test base cost calculation
    // Test cost scaling at various levels
    // Test max level clamping
}
```

**Configuration Manager Tests**
```cpp
TEST(DelveDeepConfig, AssetCaching)
{
    // Test asset loaded on first query
    // Test cached asset returned on subsequent queries
    // Test cache hit rate tracking
}

TEST(DelveDeepConfig, DataTableLookup)
{
    // Test monster config lookup by name
    // Test invalid name returns nullptr
    // Test lookup performance under 0.5ms
}
```

### Integration Tests

**System Integration**
```cpp
TEST(DelveDeepConfig, CharacterSystemIntegration)
{
    // Test character system can load character data
    // Test character stats applied from data asset
    // Test starting weapon and abilities loaded correctly
}

TEST(DelveDeepConfig, CombatSystemIntegration)
{
    // Test weapon data loaded for combat calculations
    // Test ability data loaded for skill execution
    // Test damage type modifiers applied correctly
}
```

### Performance Tests

**Load Time Tests**
- Measure total initialization time (target: < 100ms)
- Measure individual asset load times
- Test with maximum expected asset count (100+ assets)

**Query Performance Tests**
- Measure single query time (target: < 1ms)
- Measure bulk query performance (1000 queries)
- Test cache hit rate (target: > 95%)

### Console Commands for Testing

```cpp
// Validation commands
DelveDeep.ValidateAllData              // Validate all loaded configuration data
DelveDeep.ValidateCharacterData        // Validate only character data assets
DelveDeep.ValidateMonsterData          // Validate only monster configurations

// Performance commands
DelveDeep.ShowConfigStats              // Display cache stats and query performance
DelveDeep.ProfileConfigLoad            // Profile data loading performance
DelveDeep.TestConfigQueries [Count]    // Run performance test with N queries

// Debug commands
DelveDeep.ListLoadedAssets             // List all cached data assets
DelveDeep.ReloadConfigData             // Force reload all configuration data
DelveDeep.DumpConfigData [AssetName]   // Dump asset properties to log
```

## Performance Considerations

### Caching Strategy

**Asset Caching**
- All data assets cached in TMap on first access
- Soft object pointers resolved once and cached
- Cache persists for game instance lifetime
- No redundant disk reads after initialization

**Data Table Caching**
- Data tables loaded once during initialization
- Row pointers cached for fast lookup
- No per-query parsing or deserialization

### Memory Management

**Memory Footprint**
- Estimated 1-2 KB per character data asset (4 total = 8 KB)
- Estimated 500 bytes per monster config (50 configs = 25 KB)
- Estimated 1 KB per upgrade data asset (20 upgrades = 20 KB)
- Estimated 1 KB per weapon data asset (10 weapons = 10 KB)
- Estimated 1 KB per ability data asset (20 abilities = 20 KB)
- **Total estimated: ~100 KB for full configuration data**

**Optimization Strategies**
- Use TSoftObjectPtr for asset references (4 bytes vs full object)
- Lazy load referenced assets only when needed
- Unload unused assets in shipping builds (if memory constrained)
- Use data tables for bulk data (more memory efficient than individual assets)

### Query Performance

**Target Performance**
- Single asset query: < 1 millisecond
- Data table row lookup: < 0.5 milliseconds
- Bulk validation: < 100 milliseconds
- Hot reload: < 2 seconds

**Optimization Techniques**
- TMap hash-based lookup (O(1) average case)
- Const reference returns (no copying)
- Inline validation checks
- SCOPE_CYCLE_COUNTER for profiling

### Hot Reload Performance

**Development Build Only**
- File watcher monitors data asset directories
- Asset registry notifications trigger reload
- Incremental reload (only changed assets)
- Validation runs only on reloaded assets
- Disabled in shipping builds (zero overhead)

## Implementation Notes

### Module Dependencies

```cpp
// DelveDeep.Build.cs
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "GameplayTags"  // For event system integration
});

PrivateDependencyModuleNames.AddRange(new string[]
{
    "AssetRegistry"  // For hot reload support
});
```

### Blueprint Integration

**All data assets and structures exposed to Blueprint:**
- UCLASS(BlueprintType) for data asset classes
- USTRUCT(BlueprintType) for data table structures
- UFUNCTION(BlueprintCallable) for query functions
- UPROPERTY(BlueprintReadOnly) for data properties

**Blueprint Usage Example:**
```
Get Configuration Manager → Get Character Data (Name: "Warrior") → Get Base Health
```

### Editor Integration

**Property Metadata**
- ClampMin/ClampMax for numeric ranges
- MultiLine for long text descriptions
- Category for property organization
- ToolTip for designer guidance

**Data Table CSV Import**
- Support CSV import for monster configurations
- Support CSV import for item configurations
- Column headers match UPROPERTY names
- Validation runs on import

### Future Extensibility

**Planned Extensions**
- Item configuration data assets (Phase 7)
- Quest/objective configuration (Phase 8)
- Localization string tables (Phase 8)
- Mod support data exposure (Phase 8)

**Design Considerations**
- Template-based asset retrieval for new types
- Extensible validation system for custom rules
- Plugin-friendly architecture for modding
- Version migration support for data format changes
