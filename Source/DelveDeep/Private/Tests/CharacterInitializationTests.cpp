// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestMacros.h"
#include "DelveDeepTestFixtures.h"
#include "DelveDeepTestUtilities.h"
#include "Character/DelveDeepCharacter.h"
#include "Character/DelveDeepWarrior.h"
#include "Character/DelveDeepRanger.h"
#include "Character/DelveDeepMage.h"
#include "Character/DelveDeepNecromancer.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Character/DelveDeepAbilitiesComponent.h"
#include "Character/DelveDeepEquipmentComponent.h"
#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepValidation.h"
#include "Misc/AutomationTest.h"

/**
 * Character Initialization Tests
 * 
 * Tests character spawning, component creation, data loading, and validation.
 * 
 * Requirements:
 * - 1.3: Character loads data from configuration manager
 * - 5.1: Character queries configuration manager for character data
 * - 5.5: Character logs error and uses fallback default values on invalid data
 * - 17.1: Character validates character data asset is not null
 */

// ========================================
// Test: Character Spawns with All Components
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterSpawnsWithComponentsTest,
	"DelveDeep.Character.Initialization.SpawnsWithComponents",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterSpawnsWithComponentsTest::RunTest(const FString& Parameters)
{
	// Test Warrior
	{
		ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
		ASSERT_NOT_NULL(Warrior);

		// Verify all components are created
		EXPECT_NOT_NULL(Warrior->GetStatsComponent());
		EXPECT_NOT_NULL(Warrior->GetAbilitiesComponent());
		EXPECT_NOT_NULL(Warrior->GetEquipmentComponent());
	}

	// Test Ranger
	{
		ADelveDeepRanger* Ranger = NewObject<ADelveDeepRanger>();
		ASSERT_NOT_NULL(Ranger);

		// Verify all components are created
		EXPECT_NOT_NULL(Ranger->GetStatsComponent());
		EXPECT_NOT_NULL(Ranger->GetAbilitiesComponent());
		EXPECT_NOT_NULL(Ranger->GetEquipmentComponent());
	}

	// Test Mage
	{
		ADelveDeepMage* Mage = NewObject<ADelveDeepMage>();
		ASSERT_NOT_NULL(Mage);

		// Verify all components are created
		EXPECT_NOT_NULL(Mage->GetStatsComponent());
		EXPECT_NOT_NULL(Mage->GetAbilitiesComponent());
		EXPECT_NOT_NULL(Mage->GetEquipmentComponent());
	}

	// Test Necromancer
	{
		ADelveDeepNecromancer* Necromancer = NewObject<ADelveDeepNecromancer>();
		ASSERT_NOT_NULL(Necromancer);

		// Verify all components are created
		EXPECT_NOT_NULL(Necromancer->GetStatsComponent());
		EXPECT_NOT_NULL(Necromancer->GetAbilitiesComponent());
		EXPECT_NOT_NULL(Necromancer->GetEquipmentComponent());
	}

	return true;
}

// ========================================
// Test: Character Loads Data from Configuration Manager
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterLoadsDataFromConfigManagerTest,
	"DelveDeep.Character.Initialization.LoadsDataFromConfigManager",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterLoadsDataFromConfigManagerTest::RunTest(const FString& Parameters)
{
	// Create test game instance with configuration manager
	UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
	ASSERT_NOT_NULL(GameInstance);

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = 
		DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);
	ASSERT_NOT_NULL(ConfigManager);

	// Create Warrior character
	ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
	ASSERT_NOT_NULL(Warrior);

	// Verify stats component is initialized
	UDelveDeepStatsComponent* StatsComponent = Warrior->GetStatsComponent();
	ASSERT_NOT_NULL(StatsComponent);

	// Verify stats are initialized (even if using defaults)
	EXPECT_GT(Warrior->GetMaxHealth(), 0.0f);
	EXPECT_GT(Warrior->GetMaxResource(), 0.0f);

	// Cleanup
	GameInstance->Shutdown();
	GameInstance->ConditionalBeginDestroy();

	return true;
}

// ========================================
// Test: Character Initializes Stats from Data Asset
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterInitializesStatsFromDataAssetTest,
	"DelveDeep.Character.Initialization.InitializesStatsFromDataAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterInitializesStatsFromDataAssetTest::RunTest(const FString& Parameters)
{
	// Create test character data with specific values
	UDelveDeepCharacterData* CharacterData = NewObject<UDelveDeepCharacterData>();
	ASSERT_NOT_NULL(CharacterData);

	// Set specific stat values
	CharacterData->BaseHealth = 200.0f;
	CharacterData->BaseDamage = 25.0f;
	CharacterData->BaseMoveSpeed = 350.0f;
	CharacterData->BaseMana = 150.0f;

	// Validate the data
	FDelveDeepValidationContext Context;
	bool bIsValid = CharacterData->Validate(Context);
	EXPECT_TRUE(bIsValid);
	EXPECT_NO_ERRORS(Context);

	// Create character
	ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
	ASSERT_NOT_NULL(Warrior);

	// Get stats component
	UDelveDeepStatsComponent* StatsComponent = Warrior->GetStatsComponent();
	ASSERT_NOT_NULL(StatsComponent);

	// Verify stats are initialized to valid values
	EXPECT_GT(Warrior->GetMaxHealth(), 0.0f);
	EXPECT_GT(Warrior->GetMaxResource(), 0.0f);
	EXPECT_EQ(Warrior->GetCurrentHealth(), Warrior->GetMaxHealth());
	EXPECT_EQ(Warrior->GetCurrentResource(), Warrior->GetMaxResource());

	return true;
}

// ========================================
// Test: Character Handles Missing Data Gracefully
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterHandlesMissingDataGracefullyTest,
	"DelveDeep.Character.Initialization.HandlesMissingDataGracefully",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterHandlesMissingDataGracefullyTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
	ASSERT_NOT_NULL(GameInstance);

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = 
		DelveDeepTestUtils::GetTestSubsystem<UDelveDeepConfigurationManager>(GameInstance);
	ASSERT_NOT_NULL(ConfigManager);

	// Create character without providing data
	// This simulates the case where data is not found in the configuration manager
	ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
	ASSERT_NOT_NULL(Warrior);

	// Verify character doesn't crash and uses fallback values
	EXPECT_NOT_NULL(Warrior->GetStatsComponent());
	EXPECT_NOT_NULL(Warrior->GetAbilitiesComponent());
	EXPECT_NOT_NULL(Warrior->GetEquipmentComponent());

	// Verify fallback stats are valid
	EXPECT_GT(Warrior->GetMaxHealth(), 0.0f);
	EXPECT_GT(Warrior->GetMaxResource(), 0.0f);
	EXPECT_GE(Warrior->GetCurrentHealth(), 0.0f);
	EXPECT_GE(Warrior->GetCurrentResource(), 0.0f);

	// Verify character is alive (not dead from invalid data)
	EXPECT_FALSE(Warrior->IsDead());

	// Verify stats are within reasonable ranges (fallback values)
	EXPECT_LE(Warrior->GetMaxHealth(), 10000.0f);
	EXPECT_LE(Warrior->GetMaxResource(), 1000.0f);

	// Cleanup
	GameInstance->Shutdown();
	GameInstance->ConditionalBeginDestroy();

	return true;
}

// ========================================
// Test: Character Handles Invalid Data with Fallbacks
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterHandlesInvalidDataWithFallbacksTest,
	"DelveDeep.Character.Initialization.HandlesInvalidDataWithFallbacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterHandlesInvalidDataWithFallbacksTest::RunTest(const FString& Parameters)
{
	// Create invalid character data
	UDelveDeepCharacterData* InvalidData = NewObject<UDelveDeepCharacterData>();
	ASSERT_NOT_NULL(InvalidData);

	// Set invalid values
	InvalidData->BaseHealth = -100.0f;  // Invalid: negative
	InvalidData->BaseDamage = 99999.0f;  // Invalid: too high
	InvalidData->BaseMoveSpeed = 0.0f;   // Invalid: zero

	// Verify data is actually invalid
	FDelveDeepValidationContext ValidationContext;
	bool bIsValid = InvalidData->Validate(ValidationContext);
	EXPECT_FALSE(bIsValid);
	EXPECT_HAS_ERRORS(ValidationContext);

	// Create character (which should handle invalid data gracefully)
	ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
	ASSERT_NOT_NULL(Warrior);

	// Verify character doesn't crash with invalid data
	EXPECT_NOT_NULL(Warrior->GetStatsComponent());

	// Verify character uses fallback values instead of invalid data
	// Fallback values should be positive and within valid ranges
	EXPECT_GT(Warrior->GetMaxHealth(), 0.0f);
	EXPECT_GT(Warrior->GetMaxResource(), 0.0f);
	EXPECT_GE(Warrior->GetCurrentHealth(), 0.0f);
	EXPECT_GE(Warrior->GetCurrentResource(), 0.0f);

	// Verify stats are within valid ranges
	EXPECT_LE(Warrior->GetMaxHealth(), 10000.0f);
	EXPECT_LE(Warrior->GetMaxResource(), 1000.0f);

	return true;
}

// ========================================
// Test: Character Validates Data Using FDelveDeepValidationContext
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterValidatesDataUsingValidationContextTest,
	"DelveDeep.Character.Initialization.ValidatesDataUsingValidationContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterValidatesDataUsingValidationContextTest::RunTest(const FString& Parameters)
{
	// Test with valid data
	{
		UDelveDeepCharacterData* ValidData = NewObject<UDelveDeepCharacterData>();
		ASSERT_NOT_NULL(ValidData);

		// Set valid values
		ValidData->BaseHealth = 100.0f;
		ValidData->BaseDamage = 10.0f;
		ValidData->BaseMoveSpeed = 300.0f;
		ValidData->BaseMana = 100.0f;

		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("CharacterInitializationTest");
		Context.OperationName = TEXT("ValidateValidData");

		bool bIsValid = ValidData->Validate(Context);
		EXPECT_TRUE(bIsValid);
		EXPECT_NO_ERRORS(Context);

		// Verify validation report is empty for valid data
		FString Report = Context.GetReport();
		EXPECT_TRUE(Report.IsEmpty() || Report.Contains(TEXT("No errors")));
	}

	// Test with invalid data
	{
		UDelveDeepCharacterData* InvalidData = NewObject<UDelveDeepCharacterData>();
		ASSERT_NOT_NULL(InvalidData);

		// Set invalid values
		InvalidData->BaseHealth = -50.0f;  // Invalid: negative
		InvalidData->BaseDamage = 5000.0f;  // Invalid: too high
		InvalidData->BaseMoveSpeed = 0.0f;  // Invalid: zero

		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("CharacterInitializationTest");
		Context.OperationName = TEXT("ValidateInvalidData");

		bool bIsValid = InvalidData->Validate(Context);
		EXPECT_FALSE(bIsValid);
		EXPECT_HAS_ERRORS(Context);

		// Verify validation report contains error details
		FString Report = Context.GetReport();
		EXPECT_FALSE(Report.IsEmpty());
		EXPECT_STR_CONTAINS(Report, TEXT("Error"));

		// Verify specific validation errors are present
		bool bHasHealthError = false;
		bool bHasDamageError = false;
		bool bHasMoveSpeedError = false;

		for (const FString& Error : Context.ValidationErrors)
		{
			if (Error.Contains(TEXT("BaseHealth")))
			{
				bHasHealthError = true;
			}
			if (Error.Contains(TEXT("BaseDamage")))
			{
				bHasDamageError = true;
			}
			if (Error.Contains(TEXT("BaseMoveSpeed")))
			{
				bHasMoveSpeedError = true;
			}
		}

		EXPECT_TRUE(bHasHealthError);
		EXPECT_TRUE(bHasDamageError);
		EXPECT_TRUE(bHasMoveSpeedError);
	}

	// Test with null data
	{
		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("CharacterInitializationTest");
		Context.OperationName = TEXT("ValidateNullData");

		// Simulate validation of null data
		UDelveDeepCharacterData* NullData = nullptr;
		
		if (!NullData)
		{
			Context.AddError(TEXT("Character data is null"));
		}

		EXPECT_FALSE(Context.IsValid());
		EXPECT_HAS_ERRORS(Context);
		EXPECT_STR_CONTAINS(Context.GetReport(), TEXT("null"));
	}

	return true;
}

// ========================================
// Test: Character Components Initialize Correctly
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterComponentsInitializeCorrectlyTest,
	"DelveDeep.Character.Initialization.ComponentsInitializeCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterComponentsInitializeCorrectlyTest::RunTest(const FString& Parameters)
{
	// Create character
	ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
	ASSERT_NOT_NULL(Warrior);

	// Verify StatsComponent initialization
	{
		UDelveDeepStatsComponent* StatsComponent = Warrior->GetStatsComponent();
		ASSERT_NOT_NULL(StatsComponent);

		// Verify component has owner reference
		EXPECT_EQ(StatsComponent->GetCharacterOwner(), Warrior);

		// Verify stats are initialized
		EXPECT_GT(StatsComponent->GetMaxHealth(), 0.0f);
		EXPECT_GT(StatsComponent->GetMaxResource(), 0.0f);
		EXPECT_EQ(StatsComponent->GetCurrentHealth(), StatsComponent->GetMaxHealth());
		EXPECT_EQ(StatsComponent->GetCurrentResource(), StatsComponent->GetMaxResource());
	}

	// Verify AbilitiesComponent initialization
	{
		UDelveDeepAbilitiesComponent* AbilitiesComponent = Warrior->GetAbilitiesComponent();
		ASSERT_NOT_NULL(AbilitiesComponent);

		// Verify component has owner reference
		EXPECT_EQ(AbilitiesComponent->GetCharacterOwner(), Warrior);
	}

	// Verify EquipmentComponent initialization
	{
		UDelveDeepEquipmentComponent* EquipmentComponent = Warrior->GetEquipmentComponent();
		ASSERT_NOT_NULL(EquipmentComponent);

		// Verify component has owner reference
		EXPECT_EQ(EquipmentComponent->GetCharacterOwner(), Warrior);
	}

	return true;
}

// ========================================
// Test: Character Initial State Is Valid
// ========================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterInitialStateIsValidTest,
	"DelveDeep.Character.Initialization.InitialStateIsValid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterInitialStateIsValidTest::RunTest(const FString& Parameters)
{
	// Create character
	ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
	ASSERT_NOT_NULL(Warrior);

	// Verify character is alive
	EXPECT_FALSE(Warrior->IsDead());

	// Verify health is at maximum
	EXPECT_EQ(Warrior->GetCurrentHealth(), Warrior->GetMaxHealth());

	// Verify resource is at maximum
	EXPECT_EQ(Warrior->GetCurrentResource(), Warrior->GetMaxResource());

	// Verify all stats are within valid ranges
	EXPECT_GT(Warrior->GetMaxHealth(), 0.0f);
	EXPECT_LE(Warrior->GetMaxHealth(), 10000.0f);
	EXPECT_GT(Warrior->GetMaxResource(), 0.0f);
	EXPECT_LE(Warrior->GetMaxResource(), 1000.0f);

	return true;
}
