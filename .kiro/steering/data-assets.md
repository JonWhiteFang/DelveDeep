# Data Asset Guidelines

## Core Principle

Move all hardcoded values to UDataAsset classes and UDataTable structures. This enables designers to balance the game without code changes and supports rapid iteration.

## Data Asset Structure

### UDataAsset Classes

Use for complex, hierarchical configuration:

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

    // Base stats with validation ranges
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "1.0", ClampMax = "10000.0"))
    float BaseHealth = 100.0f;

    // Asset references using soft pointers
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;

    // Validation
    virtual void PostLoad() override;
    bool Validate(FValidationContext& Context) const;
};
```

### Data Tables

Use for bulk data with consistent schema:

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepMonsterConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
    FText MonsterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "1.0"))
    float Health = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
        meta = (ClampMin = "0.0"))
    float Damage = 5.0f;

    // Validation on import
    void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, 
        TArray<FString>& OutCollectedImportProblems) override;
    
    bool Validate(FValidationContext& Context) const;
};
```

## Directory Structure

Organize data assets in a standard hierarchy:

```
Content/Data/
├── Characters/          # Character data assets
│   ├── DA_Character_Warrior.uasset
│   ├── DA_Character_Ranger.uasset
│   ├── DA_Character_Mage.uasset
│   └── DA_Character_Necromancer.uasset
├── Monsters/           # Monster data tables
│   └── DT_Monster_Configs.uasset
├── Upgrades/           # Upgrade data assets
│   ├── DA_Upgrade_HealthBoost.uasset
│   ├── DA_Upgrade_DamageBoost.uasset
│   └── DA_Upgrade_SpeedBoost.uasset
├── Weapons/            # Weapon data assets
│   ├── DA_Weapon_Sword.uasset
│   ├── DA_Weapon_Bow.uasset
│   └── DA_Weapon_Staff.uasset
└── Abilities/          # Ability data assets
    ├── DA_Ability_Cleave.uasset
    ├── DA_Ability_PiercingShot.uasset
    └── DA_Ability_Fireball.uasset
```

## Naming Conventions

### Data Assets
- **Prefix**: `DA_`
- **Pattern**: `DA_[Category]_[Name]`
- **Examples**: `DA_Character_Warrior`, `DA_Weapon_Sword`, `DA_Upgrade_HealthBoost`

### Data Tables
- **Prefix**: `DT_`
- **Pattern**: `DT_[Category]_[Name]`
- **Examples**: `DT_Monster_Configs`, `DT_Item_Configs`, `DT_Upgrade_Costs`

## TSoftObjectPtr Usage

Use soft object pointers for asset references to reduce memory footprint:

```cpp
// In data asset class
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
TArray<TSoftObjectPtr<UDelveDeepAbilityData>> StartingAbilities;

// Loading soft references
void LoadAssets()
{
    if (!StartingWeapon.IsNull())
    {
        UDelveDeepWeaponData* LoadedWeapon = StartingWeapon.LoadSynchronous();
        if (LoadedWeapon)
        {
            // Use loaded weapon
        }
    }
}
```

## Validation in Data Assets

### PostLoad Validation

```cpp
void UDelveDeepCharacterData::PostLoad()
{
    Super::PostLoad();
    
    FValidationContext Context;
    Context.SystemName = TEXT("Configuration");
    Context.OperationName = TEXT("LoadCharacterData");
    
    if (!Validate(Context))
    {
        UE_LOG(LogDelveDeepConfig, Error, 
            TEXT("Character data validation failed: %s"), *Context.GetReport());
    }
}
```

### Validation Implementation

```cpp
bool UDelveDeepCharacterData::Validate(FValidationContext& Context) const
{
    bool bIsValid = true;
    
    // Validate numeric ranges
    if (BaseHealth <= 0.0f || BaseHealth > 10000.0f)
    {
        Context.AddError(FString::Printf(
            TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), BaseHealth));
        bIsValid = false;
    }
    
    // Validate asset references
    if (StartingWeapon.IsNull())
    {
        Context.AddWarning(TEXT("No starting weapon assigned"));
    }
    
    // Validate dependencies
    for (const TSoftObjectPtr<UDelveDeepUpgradeData>& RequiredUpgrade : RequiredUpgrades)
    {
        if (RequiredUpgrade.IsNull())
        {
            Context.AddError(TEXT("Null reference in required upgrades"));
            bIsValid = false;
        }
    }
    
    return bIsValid;
}
```

### Editor Data Validation

Use Unreal's built-in data validation for Editor integration:

```cpp
#if WITH_EDITOR
virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override
{
    EDataValidationResult Result = Super::IsDataValid(Context);
    
    if (BaseHealth <= 0.0f)
    {
        Context.AddError(FText::FromString(TEXT("BaseHealth must be greater than 0")));
        Result = EDataValidationResult::Invalid;
    }
    
    if (StartingWeapon.IsNull())
    {
        Context.AddWarning(FText::FromString(TEXT("No starting weapon assigned")));
    }
    
    return Result;
}
#endif
```

## Data Table Validation

```cpp
void FDelveDeepMonsterConfig::OnPostDataImport(const UDataTable* InDataTable, 
    const FName InRowName, TArray<FString>& OutCollectedImportProblems)
{
    FValidationContext Context;
    Context.SystemName = TEXT("Configuration");
    Context.OperationName = TEXT("ImportMonsterData");
    
    if (!Validate(Context))
    {
        for (const FString& Error : Context.ValidationErrors)
        {
            OutCollectedImportProblems.Add(FString::Printf(
                TEXT("Row '%s': %s"), *InRowName.ToString(), *Error));
        }
    }
}

bool FDelveDeepMonsterConfig::Validate(FValidationContext& Context) const
{
    bool bIsValid = true;
    
    if (Health <= 0.0f)
    {
        Context.AddError(TEXT("Health must be positive"));
        bIsValid = false;
    }
    
    if (Damage < 0.0f)
    {
        Context.AddError(TEXT("Damage cannot be negative"));
        bIsValid = false;
    }
    
    return bIsValid;
}
```

## Asset Reference Relationships

```cpp
// Character references weapon and abilities
UDelveDeepCharacterData
    ├─> StartingWeapon (TSoftObjectPtr<UDelveDeepWeaponData>)
    └─> StartingAbilities (TArray<TSoftObjectPtr<UDelveDeepAbilityData>>)

// Weapon references special ability
UDelveDeepWeaponData
    └─> SpecialAbility (TSoftObjectPtr<UDelveDeepAbilityData>)

// Upgrade references other upgrades (dependencies)
UDelveDeepUpgradeData
    └─> RequiredUpgrades (TArray<TSoftObjectPtr<UDelveDeepUpgradeData>>)
```

## Configuration Manager Access

Centralize data asset access through a subsystem:

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepConfigurationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Character data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const UDelveDeepCharacterData* GetCharacterData(FName CharacterName) const;

    // Monster data access
    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
    const FDelveDeepMonsterConfig* GetMonsterConfig(FName MonsterName) const;

private:
    // Asset caches
    UPROPERTY()
    TMap<FName, UDelveDeepCharacterData*> CharacterDataCache;

    UPROPERTY()
    UDataTable* MonsterConfigTable;
};
```

## Hot Reload Support (Development Only)

```cpp
#if !UE_BUILD_SHIPPING
void UDelveDeepConfigurationManager::SetupHotReload()
{
    // Register for asset reload notifications
    FAssetRegistryModule& AssetRegistryModule = 
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    
    AssetReloadHandle = AssetRegistryModule.Get().OnAssetUpdated().AddUObject(
        this, &UDelveDeepConfigurationManager::OnAssetReloaded);
}

void UDelveDeepConfigurationManager::OnAssetReloaded(const FAssetData& AssetData)
{
    // Reload modified data asset
    if (AssetData.AssetClass == UDelveDeepCharacterData::StaticClass()->GetFName())
    {
        // Reload and re-validate
        UE_LOG(LogDelveDeepConfig, Display, 
            TEXT("Hot-reloading character data: %s"), *AssetData.AssetName.ToString());
    }
}
#endif
```

## Meta Specifiers for Editor

```cpp
// Clamp values in Editor
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
    meta = (ClampMin = "1.0", ClampMax = "10000.0"))
float BaseHealth = 100.0f;

// UI slider range
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", 
    meta = (UIMin = "0.0", UIMax = "100.0"))
float HealthPercentage = 100.0f;

// Multi-line text
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", 
    meta = (MultiLine = true))
FText Description;

// Asset bundles for streaming
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", 
    meta = (AssetBundles = "Menu"))
TSoftClassPtr<class AGameMapTile> MapTileClass;

// Edit condition
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
bool bHasSpecialAbility = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", 
    meta = (EditCondition = "bHasSpecialAbility"))
float SpecialAbilityCooldown = 5.0f;

// Tooltip
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat",
    meta = (ToolTip = "Damage dealt per attack"))
float AttackDamage = 10.0f;
```

## Best Practices

1. **Use TSoftObjectPtr** for all asset references to reduce memory
2. **Validate in PostLoad()** to catch errors early
3. **Provide meaningful ranges** with ClampMin/ClampMax meta tags
4. **Use descriptive categories** for Editor organization
5. **Add tooltips** for complex properties
6. **Implement hot-reload** in development builds for rapid iteration
7. **Cache loaded assets** in configuration manager
8. **Log validation failures** with detailed context
9. **Use data tables** for bulk data (monsters, items)
10. **Follow naming conventions** (DA_ for assets, DT_ for tables)

## Common Pitfalls to Avoid

❌ **Don't** hardcode values in C++ that should be data-driven
❌ **Don't** use hard object pointers (UObject*) for asset references
❌ **Don't** skip validation in PostLoad()
❌ **Don't** place data assets outside standard directory structure
❌ **Don't** forget to add DELVEDEEP_API macro for public classes

✅ **Do** use TSoftObjectPtr for asset references
✅ **Do** validate all data on load
✅ **Do** provide fallback values for missing assets
✅ **Do** organize assets in standard directories
✅ **Do** use meta specifiers for better Editor experience
