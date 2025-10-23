// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DelveDeepValidation.h"
#include "DelveDeepConfigurationManager.generated.h"

// Forward declarations
class UDelveDeepCharacterData;
class UDelveDeepWeaponData;
class UDelveDeepAbilityData;
class UDelveDeepUpgradeData;
struct FDelveDeepMonsterConfig;
class UDataTable;

/**
 * Configuration manager subsystem for DelveDeep.
 * Provides centralized access to all game configuration data with caching and validation.
 * Manages character data, monster configs, upgrade data, weapon data, and ability data.
 */
UCLASS()
class DELVEDEEP_API UDelveDeepConfigurationManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Character data access
	/**
	 * Retrieves character data by name.
	 * 
	 * @param CharacterName The name of the character to retrieve
	 * @return Const pointer to character data, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	const UDelveDeepCharacterData* GetCharacterData(FName CharacterName) const;

	// Monster data access
	/**
	 * Retrieves monster configuration by name from data table.
	 * 
	 * @param MonsterName The name of the monster to retrieve
	 * @return Const pointer to monster config, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	const FDelveDeepMonsterConfig* GetMonsterConfig(FName MonsterName) const;

	// Upgrade data access
	/**
	 * Retrieves upgrade data by name.
	 * 
	 * @param UpgradeName The name of the upgrade to retrieve
	 * @return Const pointer to upgrade data, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	const UDelveDeepUpgradeData* GetUpgradeData(FName UpgradeName) const;

	// Weapon data access
	/**
	 * Retrieves weapon data by name.
	 * 
	 * @param WeaponName The name of the weapon to retrieve
	 * @return Const pointer to weapon data, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	const UDelveDeepWeaponData* GetWeaponData(FName WeaponName) const;

	// Ability data access
	/**
	 * Retrieves ability data by name.
	 * 
	 * @param AbilityName The name of the ability to retrieve
	 * @return Const pointer to ability data, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	const UDelveDeepAbilityData* GetAbilityData(FName AbilityName) const;

	// Validation
	/**
	 * Validates all loaded configuration data.
	 * 
	 * @param OutReport String containing the formatted validation report
	 * @return True if all data is valid (no errors), false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	bool ValidateAllData(FString& OutReport);

	// Performance metrics
	/**
	 * Retrieves performance statistics for the configuration system.
	 * 
	 * @param OutCacheHits Number of cache hits
	 * @param OutCacheMisses Number of cache misses
	 * @param OutAvgQueryTime Average query time in milliseconds
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	void GetPerformanceStats(int32& OutCacheHits, int32& OutCacheMisses, float& OutAvgQueryTime) const;

#if !UE_BUILD_SHIPPING
	// Hot-reload notification delegate
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnConfigDataReloaded, const FString& /* AssetName */);
	
	/**
	 * Event broadcast when configuration data is hot-reloaded in development builds.
	 * Provides the name of the reloaded asset.
	 */
	FOnConfigDataReloaded OnConfigDataReloaded;
#endif

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
