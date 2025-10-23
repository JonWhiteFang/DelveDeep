// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationCommands.h"
#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepMonsterConfig.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogDelveDeepConfig);

namespace DelveDeepConsoleCommands
{
	/**
	 * Console command: DelveDeep.ValidateAllData
	 * Validates all loaded configuration data and logs the report.
	 */
	static void ValidateAllDataCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (!World || !World->GetGameInstance())
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ValidateAllData: No valid game instance found"));
			return;
		}

		UDelveDeepConfigurationManager* ConfigManager = 
			World->GetGameInstance()->GetSubsystem<UDelveDeepConfigurationManager>();

		if (!ConfigManager)
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ValidateAllData: Configuration Manager not found"));
			return;
		}

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Validating All Configuration Data ==="));

		FString ValidationReport;
		bool bIsValid = ConfigManager->ValidateAllData(ValidationReport);

		if (bIsValid)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Result: SUCCESS - All data is valid"));
		}
		else
		{
			UE_LOG(LogDelveDeepConfig, Warning, TEXT("Validation Result: FAILED - Issues found"));
		}

		if (!ValidationReport.IsEmpty())
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *ValidationReport);
		}
		else
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("No validation issues found."));
		}

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Validation Complete ==="));
	}

	/**
	 * Console command: DelveDeep.ShowConfigStats
	 * Displays performance statistics for the configuration system.
	 */
	static void ShowConfigStatsCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (!World || !World->GetGameInstance())
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ShowConfigStats: No valid game instance found"));
			return;
		}

		UDelveDeepConfigurationManager* ConfigManager = 
			World->GetGameInstance()->GetSubsystem<UDelveDeepConfigurationManager>();

		if (!ConfigManager)
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ShowConfigStats: Configuration Manager not found"));
			return;
		}

		int32 CacheHits = 0;
		int32 CacheMisses = 0;
		float AvgQueryTime = 0.0f;

		ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

		int32 TotalQueries = CacheHits + CacheMisses;
		float CacheHitRate = (TotalQueries > 0) ? (static_cast<float>(CacheHits) / TotalQueries * 100.0f) : 0.0f;

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Configuration System Performance Stats ==="));
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Total Queries:    %d"), TotalQueries);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache Hits:       %d"), CacheHits);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache Misses:     %d"), CacheMisses);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache Hit Rate:   %.2f%%"), CacheHitRate);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Avg Query Time:   %.4f ms"), AvgQueryTime);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Stats Complete ==="));
	}

	/**
	 * Console command: DelveDeep.ListLoadedAssets
	 * Lists all cached configuration assets by type.
	 */
	static void ListLoadedAssetsCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (!World || !World->GetGameInstance())
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ListLoadedAssets: No valid game instance found"));
			return;
		}

		UDelveDeepConfigurationManager* ConfigManager = 
			World->GetGameInstance()->GetSubsystem<UDelveDeepConfigurationManager>();

		if (!ConfigManager)
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ListLoadedAssets: Configuration Manager not found"));
			return;
		}

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Loaded Configuration Assets ==="));

		// List character data
		int32 CharacterCount = 0;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\nCharacter Data:"));
		for (TFieldIterator<FProperty> PropIt(UDelveDeepConfigurationManager::StaticClass()); PropIt; ++PropIt)
		{
			if (FMapProperty* MapProp = CastField<FMapProperty>(*PropIt))
			{
				if (MapProp->GetName() == TEXT("CharacterDataCache"))
				{
					const void* MapPtr = MapProp->ContainerPtrToValuePtr<void>(ConfigManager);
					FScriptMapHelper MapHelper(MapProp, MapPtr);
					CharacterCount = MapHelper.Num();
					
					for (int32 i = 0; i < MapHelper.Num(); ++i)
					{
						if (MapHelper.IsValidIndex(i))
						{
							FName* KeyPtr = (FName*)MapHelper.GetKeyPtr(i);
							if (KeyPtr)
							{
								UE_LOG(LogDelveDeepConfig, Display, TEXT("  - %s"), *KeyPtr->ToString());
							}
						}
					}
					break;
				}
			}
		}
		if (CharacterCount == 0)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  (none)"));
		}

		// List upgrade data
		int32 UpgradeCount = 0;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\nUpgrade Data:"));
		for (TFieldIterator<FProperty> PropIt(UDelveDeepConfigurationManager::StaticClass()); PropIt; ++PropIt)
		{
			if (FMapProperty* MapProp = CastField<FMapProperty>(*PropIt))
			{
				if (MapProp->GetName() == TEXT("UpgradeDataCache"))
				{
					const void* MapPtr = MapProp->ContainerPtrToValuePtr<void>(ConfigManager);
					FScriptMapHelper MapHelper(MapProp, MapPtr);
					UpgradeCount = MapHelper.Num();
					
					for (int32 i = 0; i < MapHelper.Num(); ++i)
					{
						if (MapHelper.IsValidIndex(i))
						{
							FName* KeyPtr = (FName*)MapHelper.GetKeyPtr(i);
							if (KeyPtr)
							{
								UE_LOG(LogDelveDeepConfig, Display, TEXT("  - %s"), *KeyPtr->ToString());
							}
						}
					}
					break;
				}
			}
		}
		if (UpgradeCount == 0)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  (none)"));
		}

		// List weapon data
		int32 WeaponCount = 0;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\nWeapon Data:"));
		for (TFieldIterator<FProperty> PropIt(UDelveDeepConfigurationManager::StaticClass()); PropIt; ++PropIt)
		{
			if (FMapProperty* MapProp = CastField<FMapProperty>(*PropIt))
			{
				if (MapProp->GetName() == TEXT("WeaponDataCache"))
				{
					const void* MapPtr = MapProp->ContainerPtrToValuePtr<void>(ConfigManager);
					FScriptMapHelper MapHelper(MapProp, MapPtr);
					WeaponCount = MapHelper.Num();
					
					for (int32 i = 0; i < MapHelper.Num(); ++i)
					{
						if (MapHelper.IsValidIndex(i))
						{
							FName* KeyPtr = (FName*)MapHelper.GetKeyPtr(i);
							if (KeyPtr)
							{
								UE_LOG(LogDelveDeepConfig, Display, TEXT("  - %s"), *KeyPtr->ToString());
							}
						}
					}
					break;
				}
			}
		}
		if (WeaponCount == 0)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  (none)"));
		}

		// List ability data
		int32 AbilityCount = 0;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\nAbility Data:"));
		for (TFieldIterator<FProperty> PropIt(UDelveDeepConfigurationManager::StaticClass()); PropIt; ++PropIt)
		{
			if (FMapProperty* MapProp = CastField<FMapProperty>(*PropIt))
			{
				if (MapProp->GetName() == TEXT("AbilityDataCache"))
				{
					const void* MapPtr = MapProp->ContainerPtrToValuePtr<void>(ConfigManager);
					FScriptMapHelper MapHelper(MapProp, MapPtr);
					AbilityCount = MapHelper.Num();
					
					for (int32 i = 0; i < MapHelper.Num(); ++i)
					{
						if (MapHelper.IsValidIndex(i))
						{
							FName* KeyPtr = (FName*)MapHelper.GetKeyPtr(i);
							if (KeyPtr)
							{
								UE_LOG(LogDelveDeepConfig, Display, TEXT("  - %s"), *KeyPtr->ToString());
							}
						}
					}
					break;
				}
			}
		}
		if (AbilityCount == 0)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  (none)"));
		}

		int32 TotalAssets = CharacterCount + UpgradeCount + WeaponCount + AbilityCount;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\nTotal Assets: %d"), TotalAssets);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== List Complete ==="));
	}

	/**
	 * Console command: DelveDeep.ReloadConfigData
	 * Forces a full reload of all configuration data.
	 */
	static void ReloadConfigDataCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (!World || !World->GetGameInstance())
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ReloadConfigData: No valid game instance found"));
			return;
		}

		UDelveDeepConfigurationManager* ConfigManager = 
			World->GetGameInstance()->GetSubsystem<UDelveDeepConfigurationManager>();

		if (!ConfigManager)
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.ReloadConfigData: Configuration Manager not found"));
			return;
		}

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Reloading All Configuration Data ==="));

		// Deinitialize and reinitialize the subsystem
		ConfigManager->Deinitialize();
		
		FSubsystemCollectionBase DummyCollection;
		ConfigManager->Initialize(DummyCollection);

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Reload Complete ==="));
	}

	/**
	 * Console command: DelveDeep.DumpConfigData <AssetName>
	 * Dumps all properties of a specified configuration asset.
	 */
	static void DumpConfigDataCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (!World || !World->GetGameInstance())
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.DumpConfigData: No valid game instance found"));
			return;
		}

		if (Args.Num() < 1)
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.DumpConfigData: Usage: DelveDeep.DumpConfigData <AssetName>"));
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Example: DelveDeep.DumpConfigData DA_Character_Warrior"));
			return;
		}

		UDelveDeepConfigurationManager* ConfigManager = 
			World->GetGameInstance()->GetSubsystem<UDelveDeepConfigurationManager>();

		if (!ConfigManager)
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("DelveDeep.DumpConfigData: Configuration Manager not found"));
			return;
		}

		FName AssetName = FName(*Args[0]);
		bool bFound = false;

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Dumping Configuration Data: %s ==="), *AssetName.ToString());

		// Try to find as character data
		const UDelveDeepCharacterData* CharacterData = ConfigManager->GetCharacterData(AssetName);
		if (CharacterData)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("\nType: Character Data"));
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Name: %s"), *CharacterData->CharacterName.ToString());
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Description: %s"), *CharacterData->Description.ToString());
			UE_LOG(LogDelveDeepConfig, Display, TEXT("\nBase Stats:"));
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  BaseHealth: %.2f"), CharacterData->BaseHealth);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  BaseDamage: %.2f"), CharacterData->BaseDamage);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  MoveSpeed: %.2f"), CharacterData->MoveSpeed);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  BaseArmor: %.2f"), CharacterData->BaseArmor);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("\nResource System:"));
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  MaxResource: %.2f"), CharacterData->MaxResource);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  ResourceRegenRate: %.2f"), CharacterData->ResourceRegenRate);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("\nCombat Parameters:"));
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  BaseAttackSpeed: %.2f"), CharacterData->BaseAttackSpeed);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  AttackRange: %.2f"), CharacterData->AttackRange);
			UE_LOG(LogDelveDeepConfig, Display, TEXT("\nEquipment:"));
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  StartingWeapon: %s"), 
				CharacterData->StartingWeapon.IsNull() ? TEXT("(none)") : *CharacterData->StartingWeapon.ToString());
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  StartingAbilities: %d"), CharacterData->StartingAbilities.Num());
			bFound = true;
		}

		// Try to find as upgrade data
		if (!bFound)
		{
			const UDelveDeepUpgradeData* UpgradeData = ConfigManager->GetUpgradeData(AssetName);
			if (UpgradeData)
			{
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nType: Upgrade Data"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Name: %s"), *UpgradeData->UpgradeName.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Description: %s"), *UpgradeData->Description.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nCost Parameters:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  BaseCost: %d"), UpgradeData->BaseCost);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  CostScalingFactor: %.2f"), UpgradeData->CostScalingFactor);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  MaxLevel: %d"), UpgradeData->MaxLevel);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nStat Modifications:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  HealthModifier: %.2f"), UpgradeData->HealthModifier);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  DamageModifier: %.2f"), UpgradeData->DamageModifier);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  MoveSpeedModifier: %.2f"), UpgradeData->MoveSpeedModifier);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  ArmorModifier: %.2f"), UpgradeData->ArmorModifier);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nDependencies:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  RequiredUpgrades: %d"), UpgradeData->RequiredUpgrades.Num());
				bFound = true;
			}
		}

		// Try to find as weapon data
		if (!bFound)
		{
			const UDelveDeepWeaponData* WeaponData = ConfigManager->GetWeaponData(AssetName);
			if (WeaponData)
			{
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nType: Weapon Data"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Name: %s"), *WeaponData->WeaponName.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Description: %s"), *WeaponData->Description.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nCombat Stats:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  BaseDamage: %.2f"), WeaponData->BaseDamage);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  AttackSpeed: %.2f"), WeaponData->AttackSpeed);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Range: %.2f"), WeaponData->Range);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  DamageType: %s"), *WeaponData->DamageType.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nProjectile Parameters:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  ProjectileSpeed: %.2f"), WeaponData->ProjectileSpeed);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  bPiercing: %s"), WeaponData->bPiercing ? TEXT("true") : TEXT("false"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  MaxPierceTargets: %d"), WeaponData->MaxPierceTargets);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nSpecial Abilities:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  SpecialAbility: %s"), 
					WeaponData->SpecialAbility.IsNull() ? TEXT("(none)") : *WeaponData->SpecialAbility.ToString());
				bFound = true;
			}
		}

		// Try to find as ability data
		if (!bFound)
		{
			const UDelveDeepAbilityData* AbilityData = ConfigManager->GetAbilityData(AssetName);
			if (AbilityData)
			{
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nType: Ability Data"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Name: %s"), *AbilityData->AbilityName.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Description: %s"), *AbilityData->Description.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nTiming Parameters:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Cooldown: %.2f"), AbilityData->Cooldown);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  CastTime: %.2f"), AbilityData->CastTime);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Duration: %.2f"), AbilityData->Duration);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nResource Cost:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  ResourceCost: %.2f"), AbilityData->ResourceCost);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nDamage Parameters:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  DamageMultiplier: %.2f"), AbilityData->DamageMultiplier);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  DamageType: %s"), *AbilityData->DamageType.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nArea of Effect:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  AoERadius: %.2f"), AbilityData->AoERadius);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  bAffectsAllies: %s"), AbilityData->bAffectsAllies ? TEXT("true") : TEXT("false"));
				bFound = true;
			}
		}

		// Try to find as monster config
		if (!bFound)
		{
			const FDelveDeepMonsterConfig* MonsterConfig = ConfigManager->GetMonsterConfig(AssetName);
			if (MonsterConfig)
			{
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nType: Monster Config"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Name: %s"), *MonsterConfig->MonsterName.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("Description: %s"), *MonsterConfig->Description.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nBase Stats:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Health: %.2f"), MonsterConfig->Health);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Damage: %.2f"), MonsterConfig->Damage);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  MoveSpeed: %.2f"), MonsterConfig->MoveSpeed);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Armor: %.2f"), MonsterConfig->Armor);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nAI Behavior:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  DetectionRange: %.2f"), MonsterConfig->DetectionRange);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  AttackRange: %.2f"), MonsterConfig->AttackRange);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  AIBehaviorType: %s"), *MonsterConfig->AIBehaviorType.ToString());
				UE_LOG(LogDelveDeepConfig, Display, TEXT("\nLoot and Rewards:"));
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  CoinDropMin: %d"), MonsterConfig->CoinDropMin);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  CoinDropMax: %d"), MonsterConfig->CoinDropMax);
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  ExperienceReward: %d"), MonsterConfig->ExperienceReward);
				bFound = true;
			}
		}

		if (!bFound)
		{
			UE_LOG(LogDelveDeepConfig, Warning, TEXT("Asset not found: %s"), *AssetName.ToString());
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Use DelveDeep.ListLoadedAssets to see available assets"));
		}

		UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Dump Complete ==="));
	}

	// Register console commands
	FAutoConsoleCommandWithWorldAndArgs ValidateAllDataCmd(
		TEXT("DelveDeep.ValidateAllData"),
		TEXT("Validates all loaded configuration data and logs the report"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ValidateAllDataCommand)
	);

	FAutoConsoleCommandWithWorldAndArgs ShowConfigStatsCmd(
		TEXT("DelveDeep.ShowConfigStats"),
		TEXT("Displays performance statistics for the configuration system"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ShowConfigStatsCommand)
	);

	FAutoConsoleCommandWithWorldAndArgs ListLoadedAssetsCmd(
		TEXT("DelveDeep.ListLoadedAssets"),
		TEXT("Lists all cached configuration assets by type"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ListLoadedAssetsCommand)
	);

	FAutoConsoleCommandWithWorldAndArgs ReloadConfigDataCmd(
		TEXT("DelveDeep.ReloadConfigData"),
		TEXT("Forces a full reload of all configuration data"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ReloadConfigDataCommand)
	);

	FAutoConsoleCommandWithWorldAndArgs DumpConfigDataCmd(
		TEXT("DelveDeep.DumpConfigData"),
		TEXT("Dumps all properties of a specified configuration asset. Usage: DelveDeep.DumpConfigData <AssetName>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&DumpConfigDataCommand)
	);
}
