// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepMonsterConfig.h"
#include "Engine/DataTable.h"

#if !UE_BUILD_SHIPPING
#include "AssetRegistry/AssetRegistryModule.h"
#endif
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"

void UDelveDeepConfigurationManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration Manager initializing..."));

	// Initialize performance tracking
	CacheHits = 0;
	CacheMisses = 0;
	TotalQueryTime = 0.0;
	QueryCount = 0;

	// Measure initialization time
	double StartTime = FPlatformTime::Seconds();

	// Load all data assets and tables
	LoadCharacterData();
	LoadUpgradeData();
	LoadWeaponData();
	LoadAbilityData();
	LoadDataTables();

	double EndTime = FPlatformTime::Seconds();
	double InitTime = (EndTime - StartTime) * 1000.0; // Convert to milliseconds

	// Calculate total assets loaded
	int32 TotalAssets = CharacterDataCache.Num() + UpgradeDataCache.Num() + 
		WeaponDataCache.Num() + AbilityDataCache.Num();

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration Manager initialized: %d assets loaded in %.2f ms"), 
		TotalAssets, InitTime);

	// Validate loaded data
	FString ValidationReport;
	if (!ValidateAllData(ValidationReport))
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Configuration validation found issues:\n%s"), *ValidationReport);
	}

#if !UE_BUILD_SHIPPING
	// Setup hot reload for development builds
	SetupHotReload();
#endif
}

void UDelveDeepConfigurationManager::Deinitialize()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration Manager shutting down..."));

	// Cleanup caches
	CharacterDataCache.Empty();
	UpgradeDataCache.Empty();
	WeaponDataCache.Empty();
	AbilityDataCache.Empty();
	MonsterConfigTable = nullptr;
	ItemConfigTable = nullptr;

#if !UE_BUILD_SHIPPING
	// Unregister hot-reload callbacks
	if (AssetReloadHandle.IsValid())
	{
		FAssetRegistryModule& AssetRegistryModule = 
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().OnAssetUpdated().Remove(AssetReloadHandle);
	}
#endif

	Super::Deinitialize();
}

const UDelveDeepCharacterData* UDelveDeepConfigurationManager::GetCharacterData(FName CharacterName) const
{
	double StartTime = FPlatformTime::Seconds();

	// Check cache
	const UDelveDeepCharacterData* const* CachedData = CharacterDataCache.Find(CharacterName);
	
	if (CachedData)
	{
		CacheHits++;
		double EndTime = FPlatformTime::Seconds();
		TotalQueryTime += (EndTime - StartTime);
		QueryCount++;
		return *CachedData;
	}

	CacheMisses++;
	double EndTime = FPlatformTime::Seconds();
	TotalQueryTime += (EndTime - StartTime);
	QueryCount++;

	UE_LOG(LogDelveDeepConfig, Warning, TEXT("Character data not found: %s"), *CharacterName.ToString());
	return nullptr;
}

const FDelveDeepMonsterConfig* UDelveDeepConfigurationManager::GetMonsterConfig(FName MonsterName) const
{
	double StartTime = FPlatformTime::Seconds();

	if (!MonsterConfigTable)
	{
		CacheMisses++;
		double EndTime = FPlatformTime::Seconds();
		TotalQueryTime += (EndTime - StartTime);
		QueryCount++;
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Monster config table not loaded"));
		return nullptr;
	}

	// Lookup row in data table
	const FDelveDeepMonsterConfig* Config = MonsterConfigTable->FindRow<FDelveDeepMonsterConfig>(MonsterName, TEXT("GetMonsterConfig"));

	if (Config)
	{
		CacheHits++;
	}
	else
	{
		CacheMisses++;
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Monster config not found: %s"), *MonsterName.ToString());
	}

	double EndTime = FPlatformTime::Seconds();
	TotalQueryTime += (EndTime - StartTime);
	QueryCount++;

	return Config;
}

const UDelveDeepUpgradeData* UDelveDeepConfigurationManager::GetUpgradeData(FName UpgradeName) const
{
	double StartTime = FPlatformTime::Seconds();

	// Check cache
	const UDelveDeepUpgradeData* const* CachedData = UpgradeDataCache.Find(UpgradeName);
	
	if (CachedData)
	{
		CacheHits++;
		double EndTime = FPlatformTime::Seconds();
		TotalQueryTime += (EndTime - StartTime);
		QueryCount++;
		return *CachedData;
	}

	CacheMisses++;
	double EndTime = FPlatformTime::Seconds();
	TotalQueryTime += (EndTime - StartTime);
	QueryCount++;

	UE_LOG(LogDelveDeepConfig, Warning, TEXT("Upgrade data not found: %s"), *UpgradeName.ToString());
	return nullptr;
}

const UDelveDeepWeaponData* UDelveDeepConfigurationManager::GetWeaponData(FName WeaponName) const
{
	double StartTime = FPlatformTime::Seconds();

	// Check cache
	const UDelveDeepWeaponData* const* CachedData = WeaponDataCache.Find(WeaponName);
	
	if (CachedData)
	{
		CacheHits++;
		double EndTime = FPlatformTime::Seconds();
		TotalQueryTime += (EndTime - StartTime);
		QueryCount++;
		return *CachedData;
	}

	CacheMisses++;
	double EndTime = FPlatformTime::Seconds();
	TotalQueryTime += (EndTime - StartTime);
	QueryCount++;

	UE_LOG(LogDelveDeepConfig, Warning, TEXT("Weapon data not found: %s"), *WeaponName.ToString());
	return nullptr;
}

const UDelveDeepAbilityData* UDelveDeepConfigurationManager::GetAbilityData(FName AbilityName) const
{
	double StartTime = FPlatformTime::Seconds();

	// Check cache
	const UDelveDeepAbilityData* const* CachedData = AbilityDataCache.Find(AbilityName);
	
	if (CachedData)
	{
		CacheHits++;
		double EndTime = FPlatformTime::Seconds();
		TotalQueryTime += (EndTime - StartTime);
		QueryCount++;
		return *CachedData;
	}

	CacheMisses++;
	double EndTime = FPlatformTime::Seconds();
	TotalQueryTime += (EndTime - StartTime);
	QueryCount++;

	UE_LOG(LogDelveDeepConfig, Warning, TEXT("Ability data not found: %s"), *AbilityName.ToString());
	return nullptr;
}

bool UDelveDeepConfigurationManager::ValidateAllData(FString& OutReport)
{
	FValidationContext Context;
	Context.SystemName = TEXT("ConfigurationManager");
	Context.OperationName = TEXT("ValidateAllData");

	bool bAllValid = true;

	// Validate all character data
	for (const auto& Pair : CharacterDataCache)
	{
		if (Pair.Value)
		{
			if (!ValidateCharacterData(Pair.Value, Context))
			{
				bAllValid = false;
			}
		}
	}

	// Validate all monster configs
	if (MonsterConfigTable)
	{
		TArray<FName> RowNames = MonsterConfigTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			const FDelveDeepMonsterConfig* Config = MonsterConfigTable->FindRow<FDelveDeepMonsterConfig>(RowName, TEXT("ValidateAllData"));
			if (Config)
			{
				if (!ValidateMonsterConfig(Config, Context))
				{
					bAllValid = false;
				}
			}
		}
	}

	// Validate all upgrade data
	for (const auto& Pair : UpgradeDataCache)
	{
		if (Pair.Value)
		{
			if (!ValidateUpgradeData(Pair.Value, Context))
			{
				bAllValid = false;
			}
		}
	}

	// Validate all weapon data
	for (const auto& Pair : WeaponDataCache)
	{
		if (Pair.Value)
		{
			if (!ValidateWeaponData(Pair.Value, Context))
			{
				bAllValid = false;
			}
		}
	}

	// Validate all ability data
	for (const auto& Pair : AbilityDataCache)
	{
		if (Pair.Value)
		{
			if (!ValidateAbilityData(Pair.Value, Context))
			{
				bAllValid = false;
			}
		}
	}

	// Generate report
	OutReport = Context.GetReport();

	return bAllValid;
}

void UDelveDeepConfigurationManager::GetPerformanceStats(int32& OutCacheHits, int32& OutCacheMisses, float& OutAvgQueryTime) const
{
	OutCacheHits = CacheHits;
	OutCacheMisses = CacheMisses;
	
	// Calculate average query time in milliseconds
	if (QueryCount > 0)
	{
		OutAvgQueryTime = static_cast<float>((TotalQueryTime / QueryCount) * 1000.0);
	}
	else
	{
		OutAvgQueryTime = 0.0f;
	}

	// Log performance warning if queries exceed threshold
	if (QueryCount > 1000)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("High query count detected: %d queries (Cache Hits: %d, Misses: %d, Avg Time: %.4f ms)"),
			QueryCount, CacheHits, CacheMisses, OutAvgQueryTime);
	}
}

#if !UE_BUILD_SHIPPING
void UDelveDeepConfigurationManager::SetupHotReload()
{
	// TODO: Implement hot-reload support for development builds
}

void UDelveDeepConfigurationManager::OnAssetReloaded(const FAssetData& AssetData)
{
	// TODO: Implement asset reload handling
}
#endif

void UDelveDeepConfigurationManager::LoadCharacterData()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all character data assets
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UDelveDeepCharacterData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add("/Game/Data/Characters");
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loading %d character data assets..."), AssetDataList.Num());

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDelveDeepCharacterData* CharacterData = Cast<UDelveDeepCharacterData>(AssetData.GetAsset());
		if (CharacterData)
		{
			FName AssetName = FName(*AssetData.AssetName.ToString());
			CharacterDataCache.Add(AssetName, CharacterData);
			UE_LOG(LogDelveDeepConfig, Verbose, TEXT("  Loaded character data: %s"), *AssetName.ToString());
		}
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loaded %d character data assets"), CharacterDataCache.Num());
}

void UDelveDeepConfigurationManager::LoadUpgradeData()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all upgrade data assets
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UDelveDeepUpgradeData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add("/Game/Data/Upgrades");
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loading %d upgrade data assets..."), AssetDataList.Num());

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDelveDeepUpgradeData* UpgradeData = Cast<UDelveDeepUpgradeData>(AssetData.GetAsset());
		if (UpgradeData)
		{
			FName AssetName = FName(*AssetData.AssetName.ToString());
			UpgradeDataCache.Add(AssetName, UpgradeData);
			UE_LOG(LogDelveDeepConfig, Verbose, TEXT("  Loaded upgrade data: %s"), *AssetName.ToString());
		}
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loaded %d upgrade data assets"), UpgradeDataCache.Num());
}

void UDelveDeepConfigurationManager::LoadWeaponData()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all weapon data assets
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UDelveDeepWeaponData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add("/Game/Data/Weapons");
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loading %d weapon data assets..."), AssetDataList.Num());

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDelveDeepWeaponData* WeaponData = Cast<UDelveDeepWeaponData>(AssetData.GetAsset());
		if (WeaponData)
		{
			FName AssetName = FName(*AssetData.AssetName.ToString());
			WeaponDataCache.Add(AssetName, WeaponData);
			UE_LOG(LogDelveDeepConfig, Verbose, TEXT("  Loaded weapon data: %s"), *AssetName.ToString());
		}
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loaded %d weapon data assets"), WeaponDataCache.Num());
}

void UDelveDeepConfigurationManager::LoadAbilityData()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for all ability data assets
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UDelveDeepAbilityData::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add("/Game/Data/Abilities");
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loading %d ability data assets..."), AssetDataList.Num());

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDelveDeepAbilityData* AbilityData = Cast<UDelveDeepAbilityData>(AssetData.GetAsset());
		if (AbilityData)
		{
			FName AssetName = FName(*AssetData.AssetName.ToString());
			AbilityDataCache.Add(AssetName, AbilityData);
			UE_LOG(LogDelveDeepConfig, Verbose, TEXT("  Loaded ability data: %s"), *AssetName.ToString());
		}
	}

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loaded %d ability data assets"), AbilityDataCache.Num());
}

void UDelveDeepConfigurationManager::LoadDataTables()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Search for monster config data table
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add("/Game/Data/Monsters");
	Filter.bRecursivePaths = true;

	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Loading data tables..."));

	for (const FAssetData& AssetData : AssetDataList)
	{
		UDataTable* DataTable = Cast<UDataTable>(AssetData.GetAsset());
		if (DataTable)
		{
			// Check if this is a monster config table
			if (DataTable->GetRowStruct() && DataTable->GetRowStruct()->IsChildOf(FDelveDeepMonsterConfig::StaticStruct()))
			{
				MonsterConfigTable = DataTable;
				UE_LOG(LogDelveDeepConfig, Display, TEXT("  Loaded monster config table: %s (%d rows)"), 
					*AssetData.AssetName.ToString(), DataTable->GetRowNames().Num());
			}
		}
	}

	if (!MonsterConfigTable)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Monster config data table not found in /Game/Data/Monsters"));
	}
}

bool UDelveDeepConfigurationManager::ValidateCharacterData(const UDelveDeepCharacterData* Data, FValidationContext& Context) const
{
	if (!Data)
	{
		Context.AddError(TEXT("Character data is null"));
		return false;
	}

	bool bIsValid = true;

	// Validate base health
	if (Data->BaseHealth <= 0.0f || Data->BaseHealth > 10000.0f)
	{
		Context.AddError(FString::Printf(TEXT("Character '%s': BaseHealth out of range: %.2f (expected 1-10000)"), 
			*Data->GetName(), Data->BaseHealth));
		bIsValid = false;
	}

	// Validate base damage
	if (Data->BaseDamage <= 0.0f || Data->BaseDamage > 1000.0f)
	{
		Context.AddError(FString::Printf(TEXT("Character '%s': BaseDamage out of range: %.2f (expected 1-1000)"), 
			*Data->GetName(), Data->BaseDamage));
		bIsValid = false;
	}

	// Validate move speed
	if (Data->MoveSpeed < 50.0f || Data->MoveSpeed > 1000.0f)
	{
		Context.AddError(FString::Printf(TEXT("Character '%s': MoveSpeed out of range: %.2f (expected 50-1000)"), 
			*Data->GetName(), Data->MoveSpeed));
		bIsValid = false;
	}

	// Validate armor
	if (Data->BaseArmor < 0.0f || Data->BaseArmor > 100.0f)
	{
		Context.AddError(FString::Printf(TEXT("Character '%s': BaseArmor out of range: %.2f (expected 0-100)"), 
			*Data->GetName(), Data->BaseArmor));
		bIsValid = false;
	}

	// Validate starting weapon reference
	if (Data->StartingWeapon.IsNull())
	{
		Context.AddWarning(FString::Printf(TEXT("Character '%s': No starting weapon assigned"), 
			*Data->GetName()));
	}

	return bIsValid;
}

bool UDelveDeepConfigurationManager::ValidateMonsterConfig(const FDelveDeepMonsterConfig* Config, FValidationContext& Context) const
{
	if (!Config)
	{
		Context.AddError(TEXT("Monster config is null"));
		return false;
	}

	bool bIsValid = true;

	// Validate health (must be positive)
	if (Config->Health <= 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Monster '%s': Health must be positive (current: %.2f)"), 
			*Config->MonsterName.ToString(), Config->Health));
		bIsValid = false;
	}

	// Validate damage (must be non-negative)
	if (Config->Damage < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Monster '%s': Damage cannot be negative (current: %.2f)"), 
			*Config->MonsterName.ToString(), Config->Damage));
		bIsValid = false;
	}

	// Validate move speed (must be non-negative)
	if (Config->MoveSpeed < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Monster '%s': MoveSpeed cannot be negative (current: %.2f)"), 
			*Config->MonsterName.ToString(), Config->MoveSpeed));
		bIsValid = false;
	}

	// Validate detection range
	if (Config->DetectionRange < 100.0f)
	{
		Context.AddWarning(FString::Printf(TEXT("Monster '%s': DetectionRange is very low (current: %.2f)"), 
			*Config->MonsterName.ToString(), Config->DetectionRange));
	}

	return bIsValid;
}

bool UDelveDeepConfigurationManager::ValidateUpgradeData(const UDelveDeepUpgradeData* Data, FValidationContext& Context) const
{
	if (!Data)
	{
		Context.AddError(TEXT("Upgrade data is null"));
		return false;
	}

	bool bIsValid = true;

	// Validate cost scaling factor (must be between 1.0 and 10.0)
	if (Data->CostScalingFactor < 1.0f || Data->CostScalingFactor > 10.0f)
	{
		Context.AddError(FString::Printf(TEXT("Upgrade '%s': CostScalingFactor out of range: %.2f (expected 1.0-10.0)"), 
			*Data->GetName(), Data->CostScalingFactor));
		bIsValid = false;
	}

	// Validate base cost
	if (Data->BaseCost < 1)
	{
		Context.AddError(FString::Printf(TEXT("Upgrade '%s': BaseCost must be at least 1 (current: %d)"), 
			*Data->GetName(), Data->BaseCost));
		bIsValid = false;
	}

	// Validate max level
	if (Data->MaxLevel < 1)
	{
		Context.AddError(FString::Printf(TEXT("Upgrade '%s': MaxLevel must be at least 1 (current: %d)"), 
			*Data->GetName(), Data->MaxLevel));
		bIsValid = false;
	}

	// Validate that at least one stat modifier is non-zero
	if (Data->HealthModifier == 0.0f && Data->DamageModifier == 0.0f && 
		Data->MoveSpeedModifier == 0.0f && Data->ArmorModifier == 0.0f)
	{
		Context.AddWarning(FString::Printf(TEXT("Upgrade '%s': All stat modifiers are zero"), 
			*Data->GetName()));
	}

	return bIsValid;
}

bool UDelveDeepConfigurationManager::ValidateWeaponData(const UDelveDeepWeaponData* Data, FValidationContext& Context) const
{
	if (!Data)
	{
		Context.AddError(TEXT("Weapon data is null"));
		return false;
	}

	bool bIsValid = true;

	// Validate base damage
	if (Data->BaseDamage <= 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Weapon '%s': BaseDamage must be positive (current: %.2f)"), 
			*Data->GetName(), Data->BaseDamage));
		bIsValid = false;
	}

	// Validate attack speed (cooldown)
	if (Data->AttackSpeed < 0.1f)
	{
		Context.AddError(FString::Printf(TEXT("Weapon '%s': AttackSpeed must be at least 0.1 (current: %.2f)"), 
			*Data->GetName(), Data->AttackSpeed));
		bIsValid = false;
	}

	// Validate range
	if (Data->Range < 10.0f)
	{
		Context.AddError(FString::Printf(TEXT("Weapon '%s': Range must be at least 10.0 (current: %.2f)"), 
			*Data->GetName(), Data->Range));
		bIsValid = false;
	}

	// Validate projectile speed for ranged weapons
	if (Data->ProjectileSpeed > 0.0f && Data->ProjectileSpeed < 100.0f)
	{
		Context.AddWarning(FString::Printf(TEXT("Weapon '%s': ProjectileSpeed is very low (current: %.2f)"), 
			*Data->GetName(), Data->ProjectileSpeed));
	}

	return bIsValid;
}

bool UDelveDeepConfigurationManager::ValidateAbilityData(const UDelveDeepAbilityData* Data, FValidationContext& Context) const
{
	if (!Data)
	{
		Context.AddError(TEXT("Ability data is null"));
		return false;
	}

	bool bIsValid = true;

	// Validate cooldown
	if (Data->Cooldown < 0.1f)
	{
		Context.AddError(FString::Printf(TEXT("Ability '%s': Cooldown must be at least 0.1 (current: %.2f)"), 
			*Data->GetName(), Data->Cooldown));
		bIsValid = false;
	}

	// Validate resource cost (must be non-negative)
	if (Data->ResourceCost < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Ability '%s': ResourceCost cannot be negative (current: %.2f)"), 
			*Data->GetName(), Data->ResourceCost));
		bIsValid = false;
	}

	// Validate cast time (must be non-negative)
	if (Data->CastTime < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Ability '%s': CastTime cannot be negative (current: %.2f)"), 
			*Data->GetName(), Data->CastTime));
		bIsValid = false;
	}

	// Validate duration (must be non-negative)
	if (Data->Duration < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Ability '%s': Duration cannot be negative (current: %.2f)"), 
			*Data->GetName(), Data->Duration));
		bIsValid = false;
	}

	// Validate damage multiplier (must be non-negative)
	if (Data->DamageMultiplier < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Ability '%s': DamageMultiplier cannot be negative (current: %.2f)"), 
			*Data->GetName(), Data->DamageMultiplier));
		bIsValid = false;
	}

	return bIsValid;
}
