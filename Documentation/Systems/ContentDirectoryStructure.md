# Content Directory Structure

## Overview

This document defines the standard directory structure, naming conventions, and organization best practices for DelveDeep's data-driven configuration system. Following these guidelines ensures consistency, maintainability, and optimal performance of the configuration system.

## Standard Directory Structure

All configuration data assets and data tables must be organized under the `Content/Data/` directory with the following structure:

```
Content/
└── Data/
    ├── Characters/          # Character class configuration data assets
    ├── Monsters/           # Monster configuration data tables
    ├── Upgrades/           # Progression system upgrade data assets
    ├── Weapons/            # Weapon configuration data assets
    └── Abilities/          # Ability and skill configuration data assets
```

### Directory Descriptions

#### Content/Data/Characters/
Contains UDelveDeepCharacterData assets that define base stats, starting equipment, and resource parameters for each playable character class (Warrior, Ranger, Mage, Necromancer).

**Expected Assets:**
- Character base stats (health, damage, speed, armor)
- Resource system parameters (max resource, regeneration rate)
- Combat parameters (attack speed, attack range)
- Starting equipment references (weapon, abilities)

#### Content/Data/Monsters/
Contains UDataTable assets with FDelveDeepMonsterConfig rows that define enemy monster configurations, AI behavior, and reward parameters.

**Expected Assets:**
- Monster stats (health, damage, speed, armor)
- AI behavior parameters (detection range, attack range, behavior type)
- Loot and reward parameters (coin drops, experience)

#### Content/Data/Upgrades/
Contains UDelveDeepUpgradeData assets that define progression system upgrades, costs, scaling factors, and stat modifications.

**Expected Assets:**
- Upgrade costs and scaling parameters
- Stat modification values (health, damage, speed, armor modifiers)
- Upgrade dependencies and requirements

#### Content/Data/Weapons/
Contains UDelveDeepWeaponData assets that define weapon stats, combat parameters, and special abilities.

**Expected Assets:**
- Combat stats (damage, attack speed, range)
- Projectile parameters (speed, piercing, max pierce targets)
- Special ability references

#### Content/Data/Abilities/
Contains UDelveDeepAbilityData assets that define character abilities, skills, timing parameters, and effects.

**Expected Assets:**
- Timing parameters (cooldown, cast time, duration)
- Resource costs and damage parameters
- Area of effect parameters (radius, affects allies flag)

## Naming Conventions

### Data Asset Naming Convention

**Pattern:** `DA_[Category]_[Name]`

**Rules:**
- **Prefix:** All data assets MUST use the `DA_` prefix
- **Category:** The category should match the directory name (singular form)
- **Name:** Descriptive name using PascalCase
- **No Spaces:** Use underscores to separate components

**Examples:**
```
DA_Character_Warrior
DA_Character_Ranger
DA_Character_Mage
DA_Character_Necromancer

DA_Weapon_Sword
DA_Weapon_Bow
DA_Weapon_Staff
DA_Weapon_Wand

DA_Ability_Cleave
DA_Ability_PiercingShot
DA_Ability_Fireball
DA_Ability_RaiseDead

DA_Upgrade_HealthBoost
DA_Upgrade_DamageBoost
DA_Upgrade_SpeedBoost
DA_Upgrade_ArmorBoost
```

### Data Table Naming Convention

**Pattern:** `DT_[Category]_[Name]`

**Rules:**
- **Prefix:** All data tables MUST use the `DT_` prefix
- **Category:** The category should match the directory name (singular form)
- **Name:** Descriptive name using PascalCase, typically plural for collections
- **No Spaces:** Use underscores to separate components

**Examples:**
```
DT_Monster_Configs
DT_Monster_BossConfigs
DT_Item_Configs
DT_Upgrade_Costs
```

## Asset Organization Best Practices

### 1. Consistent Directory Placement

**DO:**
- Place all character data assets in `Content/Data/Characters/`
- Place all monster data tables in `Content/Data/Monsters/`
- Place all upgrade data assets in `Content/Data/Upgrades/`
- Place all weapon data assets in `Content/Data/Weapons/`
- Place all ability data assets in `Content/Data/Abilities/`

**DON'T:**
- Place data assets outside the standard directory structure
- Mix different asset types in the same directory
- Create subdirectories unless explicitly needed for large collections

**Warning:** The configuration system logs warnings when data assets are found outside the standard directory structure.

### 2. Logical Grouping

Group related assets together within their category directory:

```
Content/Data/Weapons/
├── DA_Weapon_Sword           # Melee weapons
├── DA_Weapon_Axe
├── DA_Weapon_Bow             # Ranged weapons
├── DA_Weapon_Crossbow
├── DA_Weapon_Staff           # Magic weapons
└── DA_Weapon_Wand
```

### 3. Descriptive Naming

Use clear, descriptive names that indicate the asset's purpose:

**Good Examples:**
```
DA_Character_Warrior
DA_Weapon_FlamingSword
DA_Ability_HealingWave
DA_Upgrade_MaxHealthIncrease
```

**Bad Examples:**
```
DA_Char1                      # Not descriptive
DA_WeaponData                 # Too generic
DA_Ability_Test               # Temporary naming
DA_NewUpgrade                 # Unclear purpose
```

### 4. Asset References

When creating asset references (TSoftObjectPtr), use the full path:

```cpp
// In C++ code
TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon = 
    TSoftObjectPtr<UDelveDeepWeaponData>(FSoftObjectPath(
        TEXT("/Game/Data/Weapons/DA_Weapon_Sword.DA_Weapon_Sword")));

// In data assets (Editor)
StartingWeapon: /Game/Data/Weapons/DA_Weapon_Sword
```

### 5. Version Control Considerations

- Keep data assets in source control
- Use meaningful commit messages when modifying data assets
- Test data asset changes before committing
- Document breaking changes to data asset structure

### 6. Performance Considerations

- Limit the number of assets in a single directory to < 100 for optimal loading
- Use data tables for bulk data (50+ similar configurations)
- Use soft object pointers (TSoftObjectPtr) for asset references to reduce memory footprint
- Avoid circular references between data assets

## Complete Naming Examples

### Character Data Assets
```
Content/Data/Characters/
├── DA_Character_Warrior.uasset
├── DA_Character_Ranger.uasset
├── DA_Character_Mage.uasset
└── DA_Character_Necromancer.uasset
```

### Monster Data Tables
```
Content/Data/Monsters/
├── DT_Monster_Configs.uasset          # Standard monsters
└── DT_Monster_BossConfigs.uasset      # Boss monsters (optional)
```

### Upgrade Data Assets
```
Content/Data/Upgrades/
├── DA_Upgrade_HealthBoost.uasset
├── DA_Upgrade_DamageBoost.uasset
├── DA_Upgrade_SpeedBoost.uasset
├── DA_Upgrade_ArmorBoost.uasset
├── DA_Upgrade_ResourceBoost.uasset
└── DA_Upgrade_AttackSpeedBoost.uasset
```

### Weapon Data Assets
```
Content/Data/Weapons/
├── DA_Weapon_Sword.uasset
├── DA_Weapon_Axe.uasset
├── DA_Weapon_Bow.uasset
├── DA_Weapon_Crossbow.uasset
├── DA_Weapon_Staff.uasset
└── DA_Weapon_Wand.uasset
```

### Ability Data Assets
```
Content/Data/Abilities/
├── DA_Ability_Cleave.uasset           # Warrior abilities
├── DA_Ability_ShieldBash.uasset
├── DA_Ability_PiercingShot.uasset     # Ranger abilities
├── DA_Ability_MultiShot.uasset
├── DA_Ability_Fireball.uasset         # Mage abilities
├── DA_Ability_IceNova.uasset
├── DA_Ability_RaiseDead.uasset        # Necromancer abilities
└── DA_Ability_LifeDrain.uasset
```

## Data Asset Relationships

Data assets can reference other data assets using TSoftObjectPtr. Follow these patterns:

### Character → Weapon → Ability
```
DA_Character_Warrior
    └─> StartingWeapon: DA_Weapon_Sword
            └─> SpecialAbility: DA_Ability_Cleave
```

### Character → Abilities
```
DA_Character_Mage
    └─> StartingAbilities:
            ├─> DA_Ability_Fireball
            └─> DA_Ability_IceNova
```

### Upgrade → Required Upgrades
```
DA_Upgrade_AdvancedHealthBoost
    └─> RequiredUpgrades:
            └─> DA_Upgrade_HealthBoost
```

## Validation and Error Checking

The configuration system automatically validates asset organization:

### Automatic Validation
- **Load-Time Validation:** All data assets validate on PostLoad()
- **Directory Validation:** Warns when assets are outside standard directories
- **Reference Validation:** Validates all TSoftObjectPtr references
- **Naming Validation:** Checks for proper DA_ and DT_ prefixes

### Console Commands
```
DelveDeep.ValidateAllData              # Validate all configuration data
DelveDeep.ListLoadedAssets             # List all cached data assets
DelveDeep.DumpConfigData [AssetName]   # Dump asset properties to log
```

### Common Validation Errors

**Error: Asset Outside Standard Directory**
```
LogDelveDeepConfig: Warning: Data asset outside standard directory: 
/Game/Content/MyFolder/DA_Character_Test
Expected: /Game/Data/Characters/
```

**Solution:** Move the asset to the correct directory.

**Error: Invalid Naming Convention**
```
LogDelveDeepConfig: Warning: Data asset does not follow naming convention: 
CharacterWarrior
Expected: DA_Character_Warrior
```

**Solution:** Rename the asset to follow the DA_[Category]_[Name] pattern.

**Error: Missing Asset Reference**
```
LogDelveDeepConfig: Warning: Missing asset reference in DA_Character_Warrior: 
StartingWeapon is null
```

**Solution:** Assign a valid weapon data asset to the StartingWeapon property.

## Migration and Refactoring

When reorganizing existing assets:

1. **Plan the Migration:** Document which assets need to move
2. **Update References:** Use the Editor's reference viewer to find all references
3. **Test Thoroughly:** Validate all data after migration
4. **Update Documentation:** Keep this document current with any structural changes

## Future Extensibility

The directory structure is designed to accommodate future asset types:

### Planned Additions
```
Content/Data/
├── Items/              # Item configuration (Phase 7)
├── Quests/             # Quest and objective configuration (Phase 8)
├── Localization/       # Localization string tables (Phase 8)
└── Mods/               # Mod support data (Phase 8)
```

When adding new asset types:
1. Create a new directory under `Content/Data/`
2. Define appropriate naming convention (DA_ or DT_ prefix)
3. Update this documentation
4. Update the configuration manager to load the new asset type

## Summary

Following these guidelines ensures:
- **Consistency:** All team members organize assets the same way
- **Maintainability:** Easy to locate and modify configuration data
- **Performance:** Optimal loading and caching behavior
- **Validation:** Automatic detection of organizational issues
- **Scalability:** Structure supports future growth

For questions or clarifications, refer to the Configuration System design document or contact the development team.
