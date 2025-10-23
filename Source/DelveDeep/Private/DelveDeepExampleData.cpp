#include "DelveDeepExampleData.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepMonsterConfig.h"
#include "Engine/DataTable.h"

UDelveDeepCharacterData* UDelveDeepExampleData::CreateExampleWarriorData(UObject* Outer)
{
	UDelveDeepCharacterData* WarriorData = NewObject<UDelveDeepCharacterData>(Outer, UDelveDeepCharacterData::StaticClass(), TEXT("DA_Character_Warrior"));
	
	if (WarriorData)
	{
		// Display information
		WarriorData->CharacterName = FText::FromString(TEXT("Warrior"));
		WarriorData->Description = FText::FromString(TEXT("A mighty warrior who excels in close combat. High health and armor make the Warrior a durable frontline fighter."));
		
		// Base stats
		WarriorData->BaseHealth = 150.0f;
		WarriorData->BaseDamage = 15.0f;
		WarriorData->MoveSpeed = 250.0f;
		WarriorData->BaseArmor = 10.0f;
		
		// Resource system
		WarriorData->MaxResource = 100.0f;
		WarriorData->ResourceRegenRate = 5.0f;
		
		// Combat parameters
		WarriorData->BaseAttackSpeed = 1.2f;
		WarriorData->AttackRange = 150.0f;
		
		// Starting equipment (soft references would be set in Editor)
		// WarriorData->StartingWeapon = TSoftObjectPtr<UDelveDeepWeaponData>(FSoftObjectPath(TEXT("/Game/Data/Weapons/DA_Weapon_Sword")));
	}
	
	return WarriorData;
}

UDelveDeepWeaponData* UDelveDeepExampleData::CreateExampleSwordData(UObject* Outer)
{
	UDelveDeepWeaponData* SwordData = NewObject<UDelveDeepWeaponData>(Outer, UDelveDeepWeaponData::StaticClass(), TEXT("DA_Weapon_Sword"));
	
	if (SwordData)
	{
		// Display information
		SwordData->WeaponName = FText::FromString(TEXT("Iron Sword"));
		SwordData->Description = FText::FromString(TEXT("A reliable iron sword. Deals consistent physical damage in melee range."));
		
		// Combat stats
		SwordData->BaseDamage = 20.0f;
		SwordData->AttackSpeed = 1.0f;
		SwordData->Range = 150.0f;
		SwordData->DamageType = FName(TEXT("Physical"));
		
		// Projectile parameters (not used for melee)
		SwordData->ProjectileSpeed = 0.0f;
		SwordData->bPiercing = false;
		SwordData->MaxPierceTargets = 1;
		
		// Special ability (soft reference would be set in Editor)
		// SwordData->SpecialAbility = TSoftObjectPtr<UDelveDeepAbilityData>(FSoftObjectPath(TEXT("/Game/Data/Abilities/DA_Ability_Cleave")));
	}
	
	return SwordData;
}

UDelveDeepAbilityData* UDelveDeepExampleData::CreateExampleCleaveData(UObject* Outer)
{
	UDelveDeepAbilityData* CleaveData = NewObject<UDelveDeepAbilityData>(Outer, UDelveDeepAbilityData::StaticClass(), TEXT("DA_Ability_Cleave"));
	
	if (CleaveData)
	{
		// Display information
		CleaveData->AbilityName = FText::FromString(TEXT("Cleave"));
		CleaveData->Description = FText::FromString(TEXT("Swing your weapon in a wide arc, damaging all enemies in front of you."));
		
		// Timing parameters
		CleaveData->Cooldown = 5.0f;
		CleaveData->CastTime = 0.3f;
		CleaveData->Duration = 0.0f;
		
		// Resource cost
		CleaveData->ResourceCost = 20.0f;
		
		// Damage parameters
		CleaveData->DamageMultiplier = 1.5f;
		CleaveData->DamageType = FName(TEXT("Physical"));
		
		// Area of effect
		CleaveData->AoERadius = 200.0f;
		CleaveData->bAffectsAllies = false;
	}
	
	return CleaveData;
}

UDelveDeepUpgradeData* UDelveDeepExampleData::CreateExampleHealthBoostData(UObject* Outer)
{
	UDelveDeepUpgradeData* HealthBoostData = NewObject<UDelveDeepUpgradeData>(Outer, UDelveDeepUpgradeData::StaticClass(), TEXT("DA_Upgrade_HealthBoost"));
	
	if (HealthBoostData)
	{
		// Display information
		HealthBoostData->UpgradeName = FText::FromString(TEXT("Health Boost"));
		HealthBoostData->Description = FText::FromString(TEXT("Increase your maximum health. Each level provides +10 health."));
		
		// Cost parameters
		HealthBoostData->BaseCost = 100;
		HealthBoostData->CostScalingFactor = 1.5f;
		HealthBoostData->MaxLevel = 10;
		
		// Stat modifications
		HealthBoostData->HealthModifier = 10.0f;
		HealthBoostData->DamageModifier = 0.0f;
		HealthBoostData->MoveSpeedModifier = 0.0f;
		HealthBoostData->ArmorModifier = 0.0f;
		
		// No dependencies for this basic upgrade
	}
	
	return HealthBoostData;
}

UDataTable* UDelveDeepExampleData::CreateExampleMonsterConfigTable(UObject* Outer)
{
	UDataTable* MonsterTable = NewObject<UDataTable>(Outer, UDataTable::StaticClass(), TEXT("DT_Monster_Configs"));
	
	if (MonsterTable)
	{
		MonsterTable->RowStruct = FDelveDeepMonsterConfig::StaticStruct();
		
		// Create example monster configs
		FDelveDeepMonsterConfig GoblinConfig;
		GoblinConfig.MonsterName = FText::FromString(TEXT("Goblin"));
		GoblinConfig.Description = FText::FromString(TEXT("A weak but numerous enemy. Low health and damage."));
		GoblinConfig.Health = 30.0f;
		GoblinConfig.Damage = 5.0f;
		GoblinConfig.MoveSpeed = 200.0f;
		GoblinConfig.Armor = 0.0f;
		GoblinConfig.DetectionRange = 400.0f;
		GoblinConfig.AttackRange = 100.0f;
		GoblinConfig.AIBehaviorType = FName(TEXT("Melee"));
		GoblinConfig.CoinDropMin = 1;
		GoblinConfig.CoinDropMax = 3;
		GoblinConfig.ExperienceReward = 5;
		MonsterTable->AddRow(FName(TEXT("Goblin")), GoblinConfig);
		
		FDelveDeepMonsterConfig OrcConfig;
		OrcConfig.MonsterName = FText::FromString(TEXT("Orc"));
		OrcConfig.Description = FText::FromString(TEXT("A tough melee fighter with high health and damage."));
		OrcConfig.Health = 80.0f;
		OrcConfig.Damage = 15.0f;
		OrcConfig.MoveSpeed = 180.0f;
		OrcConfig.Armor = 5.0f;
		OrcConfig.DetectionRange = 500.0f;
		OrcConfig.AttackRange = 120.0f;
		OrcConfig.AIBehaviorType = FName(TEXT("Melee"));
		OrcConfig.CoinDropMin = 5;
		OrcConfig.CoinDropMax = 10;
		OrcConfig.ExperienceReward = 15;
		MonsterTable->AddRow(FName(TEXT("Orc")), OrcConfig);
		
		FDelveDeepMonsterConfig SkeletonArcherConfig;
		SkeletonArcherConfig.MonsterName = FText::FromString(TEXT("Skeleton Archer"));
		SkeletonArcherConfig.Description = FText::FromString(TEXT("An undead archer that attacks from range."));
		SkeletonArcherConfig.Health = 40.0f;
		SkeletonArcherConfig.Damage = 10.0f;
		SkeletonArcherConfig.MoveSpeed = 150.0f;
		SkeletonArcherConfig.Armor = 0.0f;
		SkeletonArcherConfig.DetectionRange = 600.0f;
		SkeletonArcherConfig.AttackRange = 400.0f;
		SkeletonArcherConfig.AIBehaviorType = FName(TEXT("Ranged"));
		SkeletonArcherConfig.CoinDropMin = 3;
		SkeletonArcherConfig.CoinDropMax = 7;
		SkeletonArcherConfig.ExperienceReward = 10;
		MonsterTable->AddRow(FName(TEXT("SkeletonArcher")), SkeletonArcherConfig);
		
		FDelveDeepMonsterConfig TrollConfig;
		TrollConfig.MonsterName = FText::FromString(TEXT("Troll"));
		TrollConfig.Description = FText::FromString(TEXT("A massive creature with devastating damage and high health."));
		TrollConfig.Health = 200.0f;
		TrollConfig.Damage = 30.0f;
		TrollConfig.MoveSpeed = 120.0f;
		TrollConfig.Armor = 10.0f;
		TrollConfig.DetectionRange = 450.0f;
		TrollConfig.AttackRange = 150.0f;
		TrollConfig.AIBehaviorType = FName(TEXT("Melee"));
		TrollConfig.CoinDropMin = 15;
		TrollConfig.CoinDropMax = 25;
		TrollConfig.ExperienceReward = 40;
		MonsterTable->AddRow(FName(TEXT("Troll")), TrollConfig);
		
		FDelveDeepMonsterConfig DarkMageConfig;
		DarkMageConfig.MonsterName = FText::FromString(TEXT("Dark Mage"));
		DarkMageConfig.Description = FText::FromString(TEXT("A powerful spellcaster with magical attacks."));
		DarkMageConfig.Health = 60.0f;
		DarkMageConfig.Damage = 20.0f;
		DarkMageConfig.MoveSpeed = 160.0f;
		DarkMageConfig.Armor = 2.0f;
		DarkMageConfig.DetectionRange = 700.0f;
		DarkMageConfig.AttackRange = 500.0f;
		DarkMageConfig.AIBehaviorType = FName(TEXT("Caster"));
		DarkMageConfig.CoinDropMin = 10;
		DarkMageConfig.CoinDropMax = 20;
		DarkMageConfig.ExperienceReward = 25;
		MonsterTable->AddRow(FName(TEXT("DarkMage")), DarkMageConfig);
	}
	
	return MonsterTable;
}

void UDelveDeepExampleData::CreateAllExampleData(UObject* Outer, FDelveDeepExampleDataSet& OutDataSet)
{
	OutDataSet.WarriorData = CreateExampleWarriorData(Outer);
	OutDataSet.SwordData = CreateExampleSwordData(Outer);
	OutDataSet.CleaveData = CreateExampleCleaveData(Outer);
	OutDataSet.HealthBoostData = CreateExampleHealthBoostData(Outer);
	OutDataSet.MonsterConfigTable = CreateExampleMonsterConfigTable(Outer);
}
