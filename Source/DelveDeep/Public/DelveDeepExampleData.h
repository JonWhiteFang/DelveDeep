#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DelveDeepExampleData.generated.h"

class UDelveDeepCharacterData;
class UDelveDeepWeaponData;
class UDelveDeepAbilityData;
class UDelveDeepUpgradeData;
class UDataTable;

/**
 * Structure to hold a complete set of example data assets for testing
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepExampleDataSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Example Data")
	UDelveDeepCharacterData* WarriorData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Example Data")
	UDelveDeepWeaponData* SwordData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Example Data")
	UDelveDeepAbilityData* CleaveData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Example Data")
	UDelveDeepUpgradeData* HealthBoostData = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Example Data")
	UDataTable* MonsterConfigTable = nullptr;
};

/**
 * Utility class for creating example data assets programmatically for testing purposes.
 * 
 * This class provides factory methods to create example data assets that can be used
 * for testing the configuration system without requiring UE5 Editor access.
 * 
 * Example Usage:
 * @code
 * FDelveDeepExampleDataSet ExampleData;
 * UDelveDeepExampleData::CreateAllExampleData(GetTransientPackage(), ExampleData);
 * 
 * // Use the example data for testing
 * UDelveDeepCharacterData* Warrior = ExampleData.WarriorData;
 * @endcode
 */
UCLASS()
class DELVEDEEP_API UDelveDeepExampleData : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Creates an example Warrior character data asset with valid test values.
	 * 
	 * @param Outer The outer object for the created data asset (typically GetTransientPackage())
	 * @return A new UDelveDeepCharacterData instance configured as a Warrior
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Example Data")
	static UDelveDeepCharacterData* CreateExampleWarriorData(UObject* Outer);

	/**
	 * Creates an example Sword weapon data asset with valid test values.
	 * 
	 * @param Outer The outer object for the created data asset (typically GetTransientPackage())
	 * @return A new UDelveDeepWeaponData instance configured as an Iron Sword
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Example Data")
	static UDelveDeepWeaponData* CreateExampleSwordData(UObject* Outer);

	/**
	 * Creates an example Cleave ability data asset with valid test values.
	 * 
	 * @param Outer The outer object for the created data asset (typically GetTransientPackage())
	 * @return A new UDelveDeepAbilityData instance configured as a Cleave ability
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Example Data")
	static UDelveDeepAbilityData* CreateExampleCleaveData(UObject* Outer);

	/**
	 * Creates an example Health Boost upgrade data asset with valid test values.
	 * 
	 * @param Outer The outer object for the created data asset (typically GetTransientPackage())
	 * @return A new UDelveDeepUpgradeData instance configured as a Health Boost upgrade
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Example Data")
	static UDelveDeepUpgradeData* CreateExampleHealthBoostData(UObject* Outer);

	/**
	 * Creates an example monster configuration data table with 5 test monsters.
	 * 
	 * Includes the following monsters:
	 * - Goblin: Weak melee enemy
	 * - Orc: Tough melee fighter
	 * - Skeleton Archer: Ranged undead
	 * - Troll: High health tank
	 * - Dark Mage: Magical caster
	 * 
	 * @param Outer The outer object for the created data table (typically GetTransientPackage())
	 * @return A new UDataTable instance populated with monster configurations
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Example Data")
	static UDataTable* CreateExampleMonsterConfigTable(UObject* Outer);

	/**
	 * Creates a complete set of example data assets for comprehensive testing.
	 * 
	 * This is a convenience function that creates all example data assets at once.
	 * 
	 * @param Outer The outer object for the created data assets (typically GetTransientPackage())
	 * @param OutDataSet Output structure containing all created example data assets
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Example Data")
	static void CreateAllExampleData(UObject* Outer, FDelveDeepExampleDataSet& OutDataSet);
};
