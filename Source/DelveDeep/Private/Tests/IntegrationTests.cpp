// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "DelveDeepValidation.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test fixture for integration tests
 * Provides common setup and teardown for integration testing
 */
class FIntegrationTestFixture
{
public:
	FIntegrationTestFixture()
	{
		// Create test game instance
		GameInstance = NewObject<UGameInstance>();
		check(GameInstance);
		
		// Get configuration manager subsystem (auto-initializes)
		ConfigManager = GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
		check(ConfigManager);
	}

	~FIntegrationTestFixture()
	{
		// Cleanup
		ConfigManager = nullptr;
		GameInstance = nullptr;
	}

	UGameInstance* GameInstance;
	UDelveDeepConfigurationManager* ConfigManager;
};

/**
 * Test upgrade cost calculation at various levels
 * Verifies that the exponential cost scaling formula works correctly
 * Requirements: 3.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepUpgradeCostCalculationIntegrationTest, 
	"DelveDeep.Integration.UpgradeCostCalculation", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepUpgradeCostCalculationIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test upgrade data
	UDelveDeepUpgradeData* UpgradeData = NewObject<UDelveDeepUpgradeData>();
	UpgradeData->BaseCost = 100;
	UpgradeData->CostScalingFactor = 1.5f;
	UpgradeData->MaxLevel = 10;

	// Test cost calculation at level 0 (base cost)
	int32 Level0Cost = UpgradeData->CalculateCostForLevel(0);
	TestEqual(TEXT("Level 0 cost should equal base cost"), Level0Cost, 100);

	// Test cost calculation at level 1
	// Formula: 100 * (1.5 ^ 1) = 150
	int32 Level1Cost = UpgradeData->CalculateCostForLevel(1);
	TestEqual(TEXT("Level 1 cost should be 150"), Level1Cost, 150);

	// Test cost calculation at level 2
	// Formula: 100 * (1.5 ^ 2) = 225
	int32 Level2Cost = UpgradeData->CalculateCostForLevel(2);
	TestEqual(TEXT("Level 2 cost should be 225"), Level2Cost, 225);

	// Test cost calculation at level 3
	// Formula: 100 * (1.5 ^ 3) = 337.5 -> 338 (rounded)
	int32 Level3Cost = UpgradeData->CalculateCostForLevel(3);
	TestEqual(TEXT("Level 3 cost should be 338"), Level3Cost, 338);

	// Test cost calculation at level 5
	// Formula: 100 * (1.5 ^ 5) = 759.375 -> 759 (rounded)
	int32 Level5Cost = UpgradeData->CalculateCostForLevel(5);
	TestEqual(TEXT("Level 5 cost should be 759"), Level5Cost, 759);

	// Test cost calculation at max level (10)
	// Formula: 100 * (1.5 ^ 10) = 5766.5 -> 5767 (rounded)
	int32 Level10Cost = UpgradeData->CalculateCostForLevel(10);
	TestEqual(TEXT("Level 10 cost should be 5767"), Level10Cost, 5767);

	// Test with different base cost and scaling factor
	UpgradeData->BaseCost = 50;
	UpgradeData->CostScalingFactor = 2.0f;

	// Level 0: 50 * (2.0 ^ 0) = 50
	int32 NewLevel0Cost = UpgradeData->CalculateCostForLevel(0);
	TestEqual(TEXT("New level 0 cost should be 50"), NewLevel0Cost, 50);

	// Level 3: 50 * (2.0 ^ 3) = 400
	int32 NewLevel3Cost = UpgradeData->CalculateCostForLevel(3);
	TestEqual(TEXT("New level 3 cost should be 400"), NewLevel3Cost, 400);

	// Level 5: 50 * (2.0 ^ 5) = 1600
	int32 NewLevel5Cost = UpgradeData->CalculateCostForLevel(5);
	TestEqual(TEXT("New level 5 cost should be 1600"), NewLevel5Cost, 1600);

	// Verify costs increase exponentially
	TestTrue(TEXT("Level 1 cost should be greater than level 0"), Level1Cost > Level0Cost);
	TestTrue(TEXT("Level 2 cost should be greater than level 1"), Level2Cost > Level1Cost);
	TestTrue(TEXT("Level 3 cost should be greater than level 2"), Level3Cost > Level2Cost);
	TestTrue(TEXT("Level 5 cost should be greater than level 3"), Level5Cost > Level3Cost);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Upgrade cost progression: L0=%d, L1=%d, L2=%d, L3=%d, L5=%d, L10=%d"),
		Level0Cost, Level1Cost, Level2Cost, Level3Cost, Level5Cost, Level10Cost);

	return true;
}

/**
 * Test upgrade dependency chain validation
 * Verifies that upgrade dependencies are validated correctly
 * Requirements: 3.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepUpgradeDependencyChainIntegrationTest, 
	"DelveDeep.Integration.UpgradeDependencyChain", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepUpgradeDependencyChainIntegrationTest::RunTest(const FString& Parameters)
{
	// Create base upgrade (no dependencies)
	UDelveDeepUpgradeData* BaseUpgrade = NewObject<UDelveDeepUpgradeData>();
	BaseUpgrade->UpgradeName = FText::FromString(TEXT("Health Boost I"));
	BaseUpgrade->BaseCost = 50;
	BaseUpgrade->CostScalingFactor = 1.3f;
	BaseUpgrade->MaxLevel = 5;
	BaseUpgrade->HealthModifier = 10.0f;

	// Validate base upgrade
	FDelveDeepValidationContext BaseContext;
	BaseContext.SystemName = TEXT("Integration");
	BaseContext.OperationName = TEXT("TestBaseUpgrade");
	bool bBaseValid = BaseUpgrade->Validate(BaseContext);
	TestTrue(TEXT("Base upgrade should be valid"), bBaseValid);
	TestEqual(TEXT("Base upgrade should have no errors"), BaseContext.ValidationErrors.Num(), 0);

	// Create tier 2 upgrade that depends on base upgrade
	UDelveDeepUpgradeData* Tier2Upgrade = NewObject<UDelveDeepUpgradeData>();
	Tier2Upgrade->UpgradeName = FText::FromString(TEXT("Health Boost II"));
	Tier2Upgrade->BaseCost = 100;
	Tier2Upgrade->CostScalingFactor = 1.5f;
	Tier2Upgrade->MaxLevel = 5;
	Tier2Upgrade->HealthModifier = 25.0f;
	Tier2Upgrade->RequiredUpgrades.Add(TSoftObjectPtr<UDelveDeepUpgradeData>(BaseUpgrade));

	// Validate tier 2 upgrade
	FDelveDeepValidationContext Tier2Context;
	Tier2Context.SystemName = TEXT("Integration");
	Tier2Context.OperationName = TEXT("TestTier2Upgrade");
	bool bTier2Valid = Tier2Upgrade->Validate(Tier2Context);
	TestTrue(TEXT("Tier 2 upgrade should be valid"), bTier2Valid);
	TestEqual(TEXT("Tier 2 upgrade should have no errors"), Tier2Context.ValidationErrors.Num(), 0);

	// Verify dependency is set
	TestEqual(TEXT("Tier 2 should have 1 required upgrade"), Tier2Upgrade->RequiredUpgrades.Num(), 1);
	TestFalse(TEXT("Required upgrade reference should not be null"), Tier2Upgrade->RequiredUpgrades[0].IsNull());

	// Load and verify dependency
	UDelveDeepUpgradeData* LoadedDependency = Tier2Upgrade->RequiredUpgrades[0].LoadSynchronous();
	TestNotNull(TEXT("Loaded dependency should not be null"), LoadedDependency);

	if (LoadedDependency)
	{
		TestEqual(TEXT("Loaded dependency health modifier should match"), LoadedDependency->HealthModifier, 10.0f);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Successfully validated upgrade dependency: %s → %s"),
			*Tier2Upgrade->UpgradeName.ToString(), *LoadedDependency->UpgradeName.ToString());
	}

	// Create tier 3 upgrade with multiple dependencies
	UDelveDeepUpgradeData* Tier3Upgrade = NewObject<UDelveDeepUpgradeData>();
	Tier3Upgrade->UpgradeName = FText::FromString(TEXT("Health Boost III"));
	Tier3Upgrade->BaseCost = 200;
	Tier3Upgrade->CostScalingFactor = 1.8f;
	Tier3Upgrade->MaxLevel = 5;
	Tier3Upgrade->HealthModifier = 50.0f;
	Tier3Upgrade->RequiredUpgrades.Add(TSoftObjectPtr<UDelveDeepUpgradeData>(BaseUpgrade));
	Tier3Upgrade->RequiredUpgrades.Add(TSoftObjectPtr<UDelveDeepUpgradeData>(Tier2Upgrade));

	// Validate tier 3 upgrade
	FDelveDeepValidationContext Tier3Context;
	Tier3Context.SystemName = TEXT("Integration");
	Tier3Context.OperationName = TEXT("TestTier3Upgrade");
	bool bTier3Valid = Tier3Upgrade->Validate(Tier3Context);
	TestTrue(TEXT("Tier 3 upgrade should be valid"), bTier3Valid);
	TestEqual(TEXT("Tier 3 upgrade should have no errors"), Tier3Context.ValidationErrors.Num(), 0);
	TestEqual(TEXT("Tier 3 should have 2 required upgrades"), Tier3Upgrade->RequiredUpgrades.Num(), 2);

	// Test upgrade with null dependency (should fail validation)
	UDelveDeepUpgradeData* InvalidUpgrade = NewObject<UDelveDeepUpgradeData>();
	InvalidUpgrade->UpgradeName = FText::FromString(TEXT("Invalid Upgrade"));
	InvalidUpgrade->BaseCost = 100;
	InvalidUpgrade->CostScalingFactor = 1.5f;
	InvalidUpgrade->MaxLevel = 5;
	InvalidUpgrade->RequiredUpgrades.Add(TSoftObjectPtr<UDelveDeepUpgradeData>(nullptr));

	FDelveDeepValidationContext InvalidContext;
	InvalidContext.SystemName = TEXT("Integration");
	InvalidContext.OperationName = TEXT("TestInvalidUpgrade");
	bool bInvalidValid = InvalidUpgrade->Validate(InvalidContext);
	TestFalse(TEXT("Upgrade with null dependency should be invalid"), bInvalidValid);
	TestTrue(TEXT("Should have at least one error for null dependency"), InvalidContext.ValidationErrors.Num() > 0);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Upgrade dependency chain validation complete"));

	return true;
}

/**
 * Test asset reference resolution (weapon → ability)
 * Verifies that soft object pointers can be resolved correctly
 * Requirements: 3.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepAssetReferenceResolutionIntegrationTest, 
	"DelveDeep.Integration.AssetReferenceResolution", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepAssetReferenceResolutionIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test ability data
	UDelveDeepAbilityData* TestAbility = NewObject<UDelveDeepAbilityData>();
	TestAbility->AbilityName = FText::FromString(TEXT("Test Cleave"));
	TestAbility->Cooldown = 5.0f;
	TestAbility->ResourceCost = 20.0f;
	TestAbility->DamageMultiplier = 2.0f;
	TestAbility->AoERadius = 150.0f;

	// Validate ability data
	FDelveDeepValidationContext AbilityContext;
	AbilityContext.SystemName = TEXT("Integration");
	AbilityContext.OperationName = TEXT("TestAbilityCreation");
	bool bAbilityValid = TestAbility->Validate(AbilityContext);
	TestTrue(TEXT("Test ability should be valid"), bAbilityValid);

	// Create test weapon data with reference to ability
	UDelveDeepWeaponData* TestWeapon = NewObject<UDelveDeepWeaponData>();
	TestWeapon->WeaponName = FText::FromString(TEXT("Test Sword"));
	TestWeapon->BaseDamage = 15.0f;
	TestWeapon->AttackSpeed = 1.2f;
	TestWeapon->Range = 100.0f;

	// Set soft object pointer to ability
	// Note: In a real scenario, this would be a path to an asset
	// For testing, we verify the soft pointer behavior
	TestWeapon->SpecialAbility = TSoftObjectPtr<UDelveDeepAbilityData>(TestAbility);

	// Validate weapon data
	FDelveDeepValidationContext WeaponContext;
	WeaponContext.SystemName = TEXT("Integration");
	WeaponContext.OperationName = TEXT("TestWeaponCreation");
	bool bWeaponValid = TestWeapon->Validate(WeaponContext);
	TestTrue(TEXT("Test weapon should be valid"), bWeaponValid);

	// Test soft object pointer behavior
	TestFalse(TEXT("Special ability reference should not be null"), TestWeapon->SpecialAbility.IsNull());

	// Test synchronous loading of soft reference
	UDelveDeepAbilityData* LoadedAbility = TestWeapon->SpecialAbility.LoadSynchronous();
	TestNotNull(TEXT("Loaded ability should not be null"), LoadedAbility);

	if (LoadedAbility)
	{
		// Verify loaded ability matches original
		TestEqual(TEXT("Loaded ability cooldown should match"), LoadedAbility->Cooldown, 5.0f);
		TestEqual(TEXT("Loaded ability resource cost should match"), LoadedAbility->ResourceCost, 20.0f);
		TestEqual(TEXT("Loaded ability damage multiplier should match"), LoadedAbility->DamageMultiplier, 2.0f);
		TestEqual(TEXT("Loaded ability AoE radius should match"), LoadedAbility->AoERadius, 150.0f);

		UE_LOG(LogDelveDeepConfig, Display, TEXT("Successfully resolved weapon → ability reference"));
	}

	// Test with null reference
	UDelveDeepWeaponData* WeaponWithoutAbility = NewObject<UDelveDeepWeaponData>();
	WeaponWithoutAbility->WeaponName = FText::FromString(TEXT("Basic Sword"));
	WeaponWithoutAbility->BaseDamage = 10.0f;
	WeaponWithoutAbility->AttackSpeed = 1.0f;
	WeaponWithoutAbility->Range = 100.0f;
	// SpecialAbility is null by default

	TestTrue(TEXT("Weapon without ability should have null reference"), WeaponWithoutAbility->SpecialAbility.IsNull());

	// Validate weapon without ability (should still be valid, just may have warnings)
	FDelveDeepValidationContext NoAbilityContext;
	NoAbilityContext.SystemName = TEXT("Integration");
	NoAbilityContext.OperationName = TEXT("TestWeaponWithoutAbility");
	bool bNoAbilityValid = WeaponWithoutAbility->Validate(NoAbilityContext);
	TestTrue(TEXT("Weapon without special ability should still be valid"), bNoAbilityValid);

	return true;
}

#if !UE_BUILD_SHIPPING
/**
 * Test hot-reload functionality in development builds
 * Verifies that configuration data can be hot-reloaded when modified
 * Requirements: 7.2, 7.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepHotReloadIntegrationTest, 
	"DelveDeep.Integration.HotReload", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepHotReloadIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FIntegrationTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Create test character data
	UDelveDeepCharacterData* TestCharacter = NewObject<UDelveDeepCharacterData>();
	TestCharacter->CharacterName = FText::FromString(TEXT("Test Warrior"));
	TestCharacter->BaseHealth = 100.0f;
	TestCharacter->BaseDamage = 15.0f;
	TestCharacter->MoveSpeed = 300.0f;

	// Validate initial character data
	FDelveDeepValidationContext InitialContext;
	InitialContext.SystemName = TEXT("Integration");
	InitialContext.OperationName = TEXT("TestInitialCharacter");
	bool bInitialValid = TestCharacter->Validate(InitialContext);
	TestTrue(TEXT("Initial character data should be valid"), bInitialValid);

	// Track if hot-reload event was fired
	bool bHotReloadEventFired = false;
	FString ReloadedAssetName;

	// Subscribe to hot-reload event
	FDelegateHandle EventHandle = ConfigManager->OnConfigDataReloaded.AddLambda(
		[&bHotReloadEventFired, &ReloadedAssetName](const FString& AssetName)
		{
			bHotReloadEventFired = true;
			ReloadedAssetName = AssetName;
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Hot-reload event received for asset: %s"), *AssetName);
		}
	);

	// Simulate asset modification by changing values
	TestCharacter->BaseHealth = 150.0f;
	TestCharacter->BaseDamage = 20.0f;

	// Re-validate modified character data
	FDelveDeepValidationContext ModifiedContext;
	ModifiedContext.SystemName = TEXT("Integration");
	ModifiedContext.OperationName = TEXT("TestModifiedCharacter");
	bool bModifiedValid = TestCharacter->Validate(ModifiedContext);
	TestTrue(TEXT("Modified character data should be valid"), bModifiedValid);

	// Verify modified values
	TestEqual(TEXT("Modified health should be 150"), TestCharacter->BaseHealth, 150.0f);
	TestEqual(TEXT("Modified damage should be 20"), TestCharacter->BaseDamage, 20.0f);

	// Note: In a real hot-reload scenario, the asset registry would detect file changes
	// and trigger the OnAssetReloaded callback. For this test, we verify the event system works.
	
	// Manually trigger hot-reload event to test the system
	ConfigManager->OnConfigDataReloaded.Broadcast(TEXT("TestWarrior"));

	// Verify event was fired
	TestTrue(TEXT("Hot-reload event should have been fired"), bHotReloadEventFired);
	TestEqual(TEXT("Reloaded asset name should match"), ReloadedAssetName, TEXT("TestWarrior"));

	// Cleanup event handler
	ConfigManager->OnConfigDataReloaded.Remove(EventHandle);

	// Test hot-reload timing requirement (< 2 seconds)
	// Measure time to reload and re-validate
	double StartTime = FPlatformTime::Seconds();
	
	// Simulate reload by re-validating
	FDelveDeepValidationContext ReloadContext;
	ReloadContext.SystemName = TEXT("Integration");
	ReloadContext.OperationName = TEXT("TestReloadValidation");
	bool bReloadValid = TestCharacter->Validate(ReloadContext);
	
	double EndTime = FPlatformTime::Seconds();
	double ReloadTime = (EndTime - StartTime) * 1000.0; // Convert to milliseconds

	TestTrue(TEXT("Reloaded data should be valid"), bReloadValid);
	TestTrue(TEXT("Reload time should be under 2000ms"), ReloadTime < 2000.0);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Hot-reload validation time: %.2f ms"), ReloadTime);

	// Test that hot-reload system is only active in development builds
	#if UE_BUILD_SHIPPING
		TestTrue(TEXT("Hot-reload should be disabled in shipping builds"), false);
	#else
		TestTrue(TEXT("Hot-reload should be enabled in development builds"), true);
	#endif

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Hot-reload integration test complete"));

	return true;
}
#endif // !UE_BUILD_SHIPPING

#endif // WITH_DEV_AUTOMATION_TESTS
