# Task 11: Example Data Assets for Testing

## Overview

This document describes the example data assets created for testing the Data-Driven Configuration System. These assets provide valid test data that can be used to verify the configuration system's functionality without requiring UE5 Editor access.

## Implementation Details

### Files Created

1. **Source/DelveDeep/Public/DelveDeepExampleData.h**
   - Header file defining the example data utility class
   - Provides factory methods for creating example data assets
   - Includes FDelveDeepExampleDataSet structure for holding all example data

2. **Source/DelveDeep/Private/DelveDeepExampleData.cpp**
   - Implementation of example data creation functions
   - Creates programmatically instantiated data assets with valid test values

3. **Console Command Integration**
   - Added `DelveDeep.CreateExampleData` command to DelveDeepValidationCommands.cpp
   - Command creates all example data and validates it

## Example Data Assets

### 1. DA_Character_Warrior (Character Data)

**Display Information:**
- Name: "Warrior"
- Description: "A mighty warrior who excels in close combat. High health and armor make the Warrior a durable frontline fighter."

**Base Stats:**
- BaseHealth: 150.0
- BaseDamage: 15.0
- MoveSpeed: 250.0
- BaseArmor: 10.0

**Resource System:**
- MaxResource: 100.0
- ResourceRegenRate: 5.0

**Combat Parameters:**
- BaseAttackSpeed: 1.2
- AttackRange: 150.0

**Requirements Satisfied:** 1.2, 1.4, 9.2

### 2. DA_Weapon_Sword (Weapon Data)

**Display Information:**
- Name: "Iron Sword"
- Description: "A reliable iron sword. Deals consistent physical damage in melee range."

**Combat Stats:**
- BaseDamage: 20.0
- AttackSpeed: 1.0
- Range: 150.0
- DamageType: "Physical"

**Projectile Parameters:**
- ProjectileSpeed: 0.0 (melee weapon)
- bPiercing: false
- MaxPierceTargets: 1

**Requirements Satisfied:** 4.1, 4.4, 9.2

### 3. DA_Ability_Cleave (Ability Data)

**Display Information:**
- Name: "Cleave"
- Description: "Swing your weapon in a wide arc, damaging all enemies in front of you."

**Timing Parameters:**
- Cooldown: 5.0 seconds
- CastTime: 0.3 seconds
- Duration: 0.0 seconds (instant)

**Resource Cost:**
- ResourceCost: 20.0

**Damage Parameters:**
- DamageMultiplier: 1.5
- DamageType: "Physical"

**Area of Effect:**
- AoERadius: 200.0
- bAffectsAllies: false

**Requirements Satisfied:** 4.2, 4.3, 9.2

### 4. DA_Upgrade_HealthBoost (Upgrade Data)

**Display Information:**
- Name: "Health Boost"
- Description: "Increase your maximum health. Each level provides +10 health."

**Cost Parameters:**
- BaseCost: 100
- CostScalingFactor: 1.5
- MaxLevel: 10

**Stat Modifications:**
- HealthModifier: 10.0
- DamageModifier: 0.0
- MoveSpeedModifier: 0.0
- ArmorModifier: 0.0

**Cost Calculation Examples:**
- Level 1: 150 coins
- Level 5: 759 coins
- Level 10: 5,767 coins

**Requirements Satisfied:** 3.1, 3.2, 3.3, 3.5

### 5. DT_Monster_Configs (Data Table)

Contains 5 example monster configurations:

#### Goblin
- Health: 30.0
- Damage: 5.0
- MoveSpeed: 200.0
- Armor: 0.0
- DetectionRange: 400.0
- AttackRange: 100.0
- AIBehaviorType: "Melee"
- CoinDrop: 1-3
- ExperienceReward: 5

#### Orc
- Health: 80.0
- Damage: 15.0
- MoveSpeed: 180.0
- Armor: 5.0
- DetectionRange: 500.0
- AttackRange: 120.0
- AIBehaviorType: "Melee"
- CoinDrop: 5-10
- ExperienceReward: 15

#### Skeleton Archer
- Health: 40.0
- Damage: 10.0
- MoveSpeed: 150.0
- Armor: 0.0
- DetectionRange: 600.0
- AttackRange: 400.0
- AIBehaviorType: "Ranged"
- CoinDrop: 3-7
- ExperienceReward: 10

#### Troll
- Health: 200.0
- Damage: 30.0
- MoveSpeed: 120.0
- Armor: 10.0
- DetectionRange: 450.0
- AttackRange: 150.0
- AIBehaviorType: "Melee"
- CoinDrop: 15-25
- ExperienceReward: 40

#### Dark Mage
- Health: 60.0
- Damage: 20.0
- MoveSpeed: 160.0
- Armor: 2.0
- DetectionRange: 700.0
- AttackRange: 500.0
- AIBehaviorType: "Caster"
- CoinDrop: 10-20
- ExperienceReward: 25

**Requirements Satisfied:** 2.1, 2.5, 9.2

## Usage

### Creating Example Data Programmatically

```cpp
// Create all example data at once
FDelveDeepExampleDataSet ExampleData;
UDelveDeepExampleData::CreateAllExampleData(GetTransientPackage(), ExampleData);

// Access individual data assets
UDelveDeepCharacterData* Warrior = ExampleData.WarriorData;
UDelveDeepWeaponData* Sword = ExampleData.SwordData;
UDelveDeepAbilityData* Cleave = ExampleData.CleaveData;
UDelveDeepUpgradeData* HealthBoost = ExampleData.HealthBoostData;
UDataTable* MonsterTable = ExampleData.MonsterConfigTable;
```

### Creating Individual Data Assets

```cpp
// Create individual data assets
UDelveDeepCharacterData* Warrior = UDelveDeepExampleData::CreateExampleWarriorData(GetTransientPackage());
UDelveDeepWeaponData* Sword = UDelveDeepExampleData::CreateExampleSwordData(GetTransientPackage());
UDelveDeepAbilityData* Cleave = UDelveDeepExampleData::CreateExampleCleaveData(GetTransientPackage());
UDelveDeepUpgradeData* HealthBoost = UDelveDeepExampleData::CreateExampleHealthBoostData(GetTransientPackage());
UDataTable* MonsterTable = UDelveDeepExampleData::CreateExampleMonsterConfigTable(GetTransientPackage());
```

### Console Command

```bash
# Create and validate all example data
DelveDeep.CreateExampleData
```

**Output Example:**
```
=== Creating Example Data Assets ===
✓ Created DA_Character_Warrior: Warrior
✓ Created DA_Weapon_Sword: Iron Sword
✓ Created DA_Ability_Cleave: Cleave
✓ Created DA_Upgrade_HealthBoost: Health Boost
  Cost at Level 1: 150
  Cost at Level 5: 759
  Cost at Level 10: 5767
✓ Created DT_Monster_Configs with 5 monsters:
  ✓ Goblin: Goblin (HP: 30, DMG: 5)
  ✓ Orc: Orc (HP: 80, DMG: 15)
  ✓ SkeletonArcher: Skeleton Archer (HP: 40, DMG: 10)
  ✓ Troll: Troll (HP: 200, DMG: 30)
  ✓ DarkMage: Dark Mage (HP: 60, DMG: 20)

=== Example Data Creation Complete ===
Success: 5 | Failures: 0
All example data assets created and validated successfully!
```

## Validation

All example data assets are validated during creation:

1. **Character Data Validation**
   - BaseHealth within range (1-10000)
   - BaseDamage within range (1-1000)
   - MoveSpeed within range (50-1000)
   - BaseArmor within range (0-100)

2. **Weapon Data Validation**
   - BaseDamage > 0
   - AttackSpeed > 0.1
   - Range > 10

3. **Ability Data Validation**
   - Cooldown > 0.1
   - ResourceCost >= 0
   - DamageMultiplier >= 0

4. **Upgrade Data Validation**
   - BaseCost > 0
   - CostScalingFactor between 1.0 and 10.0
   - MaxLevel > 0

5. **Monster Config Validation**
   - Health > 0
   - Damage >= 0
   - All ranges positive

## Testing Integration

These example data assets can be used in unit tests:

```cpp
TEST(DelveDeepConfig, ExampleDataValidation)
{
    FDelveDeepExampleDataSet ExampleData;
    UDelveDeepExampleData::CreateAllExampleData(GetTransientPackage(), ExampleData);
    
    // Test character data
    ASSERT_NE(ExampleData.WarriorData, nullptr);
    EXPECT_GT(ExampleData.WarriorData->BaseHealth, 0.0f);
    
    // Test weapon data
    ASSERT_NE(ExampleData.SwordData, nullptr);
    EXPECT_GT(ExampleData.SwordData->BaseDamage, 0.0f);
    
    // Test ability data
    ASSERT_NE(ExampleData.CleaveData, nullptr);
    EXPECT_GT(ExampleData.CleaveData->Cooldown, 0.0f);
    
    // Test upgrade data
    ASSERT_NE(ExampleData.HealthBoostData, nullptr);
    EXPECT_GT(ExampleData.HealthBoostData->BaseCost, 0);
    
    // Test monster table
    ASSERT_NE(ExampleData.MonsterConfigTable, nullptr);
    EXPECT_EQ(ExampleData.MonsterConfigTable->GetRowNames().Num(), 5);
}
```

## Requirements Coverage

This implementation satisfies the following requirements from the specification:

- **Requirement 1.1**: Character data loading (Warrior example)
- **Requirement 2.1**: Monster configuration data table (5 monsters)
- **Requirement 3.1**: Upgrade data with cost parameters (Health Boost)
- **Requirement 4.1**: Weapon and ability data (Sword and Cleave)

## Notes

- All data assets are created in the transient package for testing purposes
- In a production environment with UE5 Editor access, these would be created as actual .uasset files
- The example data provides a complete set of valid test data for the configuration system
- All values are within the validated ranges defined in the data asset classes
- The console command provides an easy way to create and validate example data during development

## Future Enhancements

When UE5 Editor access is available:

1. Create actual .uasset files in Content/Data/ directories
2. Set up soft object pointer references between assets (e.g., Warrior → Sword → Cleave)
3. Create additional example data for Ranger, Mage, and Necromancer characters
4. Add more monster configurations for different difficulty levels
5. Create upgrade dependency chains for testing
