// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test FValidationContext error tracking functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextErrorTest, 
	"DelveDeep.Validation.ErrorTracking", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextErrorTest::RunTest(const FString& Parameters)
{
	// Create validation context
	FValidationContext Context;
	Context.SystemName = TEXT("TestSystem");
	Context.OperationName = TEXT("TestOperation");

	// Initially should be valid (no errors)
	TestTrue(TEXT("Context should be valid initially"), Context.IsValid());
	TestEqual(TEXT("Should have no errors initially"), Context.ValidationErrors.Num(), 0);
	TestEqual(TEXT("Should have no warnings initially"), Context.ValidationWarnings.Num(), 0);

	// Add an error
	Context.AddError(TEXT("Test error message"));
	TestFalse(TEXT("Context should be invalid after adding error"), Context.IsValid());
	TestEqual(TEXT("Should have one error"), Context.ValidationErrors.Num(), 1);
	TestEqual(TEXT("Error message should match"), Context.ValidationErrors[0], TEXT("Test error message"));

	// Add a warning
	Context.AddWarning(TEXT("Test warning message"));
	TestFalse(TEXT("Context should still be invalid (has error)"), Context.IsValid());
	TestEqual(TEXT("Should have one warning"), Context.ValidationWarnings.Num(), 1);
	TestEqual(TEXT("Warning message should match"), Context.ValidationWarnings[0], TEXT("Test warning message"));

	return true;
}

/**
 * Test FValidationContext report generation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextReportTest, 
	"DelveDeep.Validation.ReportGeneration", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextReportTest::RunTest(const FString& Parameters)
{
	// Create validation context with errors and warnings
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadCharacterData");
	Context.AddError(TEXT("Invalid health value"));
	Context.AddWarning(TEXT("Missing weapon reference"));

	// Generate report
	FString Report = Context.GetReport();

	// Verify report contains expected content
	TestTrue(TEXT("Report should contain system name"), Report.Contains(TEXT("Configuration")));
	TestTrue(TEXT("Report should contain operation name"), Report.Contains(TEXT("LoadCharacterData")));
	TestTrue(TEXT("Report should contain error"), Report.Contains(TEXT("Invalid health value")));
	TestTrue(TEXT("Report should contain warning"), Report.Contains(TEXT("Missing weapon reference")));
	TestTrue(TEXT("Report should show FAILED status"), Report.Contains(TEXT("FAILED")));
	TestTrue(TEXT("Report should show error count"), Report.Contains(TEXT("Total Errors: 1")));
	TestTrue(TEXT("Report should show warning count"), Report.Contains(TEXT("Total Warnings: 1")));

	return true;
}

/**
 * Test FValidationContext reset functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextResetTest, 
	"DelveDeep.Validation.Reset", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextResetTest::RunTest(const FString& Parameters)
{
	// Create validation context with data
	FValidationContext Context;
	Context.SystemName = TEXT("TestSystem");
	Context.OperationName = TEXT("TestOperation");
	Context.AddError(TEXT("Test error"));
	Context.AddWarning(TEXT("Test warning"));

	// Verify data is present
	TestFalse(TEXT("Context should be invalid before reset"), Context.IsValid());
	TestEqual(TEXT("Should have one error before reset"), Context.ValidationErrors.Num(), 1);
	TestEqual(TEXT("Should have one warning before reset"), Context.ValidationWarnings.Num(), 1);

	// Reset context
	Context.Reset();

	// Verify data is cleared
	TestTrue(TEXT("Context should be valid after reset"), Context.IsValid());
	TestEqual(TEXT("Should have no errors after reset"), Context.ValidationErrors.Num(), 0);
	TestEqual(TEXT("Should have no warnings after reset"), Context.ValidationWarnings.Num(), 0);
	TestTrue(TEXT("System name should be empty after reset"), Context.SystemName.IsEmpty());
	TestTrue(TEXT("Operation name should be empty after reset"), Context.OperationName.IsEmpty());

	return true;
}

/**
 * Test FValidationContext with multiple errors and warnings
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextMultipleIssuesTest, 
	"DelveDeep.Validation.MultipleIssues", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextMultipleIssuesTest::RunTest(const FString& Parameters)
{
	// Create validation context
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("ValidateAllData");

	// Add multiple errors
	Context.AddError(TEXT("Error 1"));
	Context.AddError(TEXT("Error 2"));
	Context.AddError(TEXT("Error 3"));

	// Add multiple warnings
	Context.AddWarning(TEXT("Warning 1"));
	Context.AddWarning(TEXT("Warning 2"));

	// Verify counts
	TestFalse(TEXT("Context should be invalid with errors"), Context.IsValid());
	TestEqual(TEXT("Should have three errors"), Context.ValidationErrors.Num(), 3);
	TestEqual(TEXT("Should have two warnings"), Context.ValidationWarnings.Num(), 2);

	// Verify report includes all issues
	FString Report = Context.GetReport();
	TestTrue(TEXT("Report should contain all errors"), 
		Report.Contains(TEXT("Error 1")) && 
		Report.Contains(TEXT("Error 2")) && 
		Report.Contains(TEXT("Error 3")));
	TestTrue(TEXT("Report should contain all warnings"), 
		Report.Contains(TEXT("Warning 1")) && 
		Report.Contains(TEXT("Warning 2")));

	return true;
}

/**
 * Test character data validation with invalid health value
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepCharacterDataValidationTest, 
	"DelveDeep.Validation.CharacterData", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepCharacterDataValidationTest::RunTest(const FString& Parameters)
{
	// Create character data with invalid health
	UDelveDeepCharacterData* CharacterData = NewObject<UDelveDeepCharacterData>();
	CharacterData->BaseHealth = -10.0f; // Invalid: must be positive
	CharacterData->BaseDamage = 15.0f;  // Valid
	CharacterData->MoveSpeed = 300.0f;  // Valid
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("TestCharacterValidation");
	
	// Validate - should fail due to invalid health
	bool bIsValid = CharacterData->Validate(Context);
	
	TestFalse(TEXT("Character data should be invalid with negative health"), bIsValid);
	TestTrue(TEXT("Should have at least one error"), Context.ValidationErrors.Num() > 0);
	TestTrue(TEXT("Error should mention BaseHealth"), 
		Context.ValidationErrors[0].Contains(TEXT("BaseHealth")));
	
	// Test with health too high
	CharacterData->BaseHealth = 15000.0f; // Invalid: exceeds max
	Context.Reset();
	bIsValid = CharacterData->Validate(Context);
	
	TestFalse(TEXT("Character data should be invalid with excessive health"), bIsValid);
	TestTrue(TEXT("Should have error for out of range health"), Context.ValidationErrors.Num() > 0);
	
	// Test with valid health
	CharacterData->BaseHealth = 100.0f; // Valid
	Context.Reset();
	bIsValid = CharacterData->Validate(Context);
	
	TestTrue(TEXT("Character data should be valid with correct health"), bIsValid);
	TestEqual(TEXT("Should have no errors with valid data"), Context.ValidationErrors.Num(), 0);
	
	return true;
}

/**
 * Test monster config validation with negative damage
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepMonsterConfigValidationTest, 
	"DelveDeep.Validation.MonsterConfig", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepMonsterConfigValidationTest::RunTest(const FString& Parameters)
{
	// Create monster config with negative damage
	FDelveDeepMonsterConfig MonsterConfig;
	MonsterConfig.Health = 50.0f;    // Valid
	MonsterConfig.Damage = -5.0f;    // Invalid: cannot be negative
	MonsterConfig.MoveSpeed = 200.0f; // Valid
	MonsterConfig.DetectionRange = 500.0f; // Valid
	MonsterConfig.AttackRange = 100.0f;    // Valid
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("TestMonsterValidation");
	
	// Validate - should fail due to negative damage
	bool bIsValid = MonsterConfig.Validate(Context);
	
	TestFalse(TEXT("Monster config should be invalid with negative damage"), bIsValid);
	TestTrue(TEXT("Should have at least one error"), Context.ValidationErrors.Num() > 0);
	TestTrue(TEXT("Error should mention Damage"), 
		Context.ValidationErrors[0].Contains(TEXT("Damage")));
	
	// Test with zero health (invalid)
	MonsterConfig.Damage = 5.0f;  // Fix damage
	MonsterConfig.Health = 0.0f;  // Invalid: must be positive
	Context.Reset();
	bIsValid = MonsterConfig.Validate(Context);
	
	TestFalse(TEXT("Monster config should be invalid with zero health"), bIsValid);
	TestTrue(TEXT("Should have error for non-positive health"), Context.ValidationErrors.Num() > 0);
	
	// Test with valid values
	MonsterConfig.Health = 50.0f; // Fix health
	Context.Reset();
	bIsValid = MonsterConfig.Validate(Context);
	
	TestTrue(TEXT("Monster config should be valid with correct values"), bIsValid);
	TestEqual(TEXT("Should have no errors with valid data"), Context.ValidationErrors.Num(), 0);
	
	return true;
}

/**
 * Test upgrade data validation with out-of-range scaling factor
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepUpgradeDataValidationTest, 
	"DelveDeep.Validation.UpgradeData", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepUpgradeDataValidationTest::RunTest(const FString& Parameters)
{
	// Create upgrade data with out-of-range scaling factor
	UDelveDeepUpgradeData* UpgradeData = NewObject<UDelveDeepUpgradeData>();
	UpgradeData->BaseCost = 100;              // Valid
	UpgradeData->CostScalingFactor = 15.0f;   // Invalid: exceeds max of 10.0
	UpgradeData->MaxLevel = 10;               // Valid
	UpgradeData->HealthModifier = 10.0f;      // Valid
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("TestUpgradeValidation");
	
	// Validate - should fail due to out-of-range scaling factor
	bool bIsValid = UpgradeData->Validate(Context);
	
	TestFalse(TEXT("Upgrade data should be invalid with excessive scaling factor"), bIsValid);
	TestTrue(TEXT("Should have at least one error"), Context.ValidationErrors.Num() > 0);
	TestTrue(TEXT("Error should mention CostScalingFactor"), 
		Context.ValidationErrors[0].Contains(TEXT("CostScalingFactor")));
	
	// Test with scaling factor too low
	UpgradeData->CostScalingFactor = 0.5f; // Invalid: below min of 1.0
	Context.Reset();
	bIsValid = UpgradeData->Validate(Context);
	
	TestFalse(TEXT("Upgrade data should be invalid with low scaling factor"), bIsValid);
	TestTrue(TEXT("Should have error for out of range scaling factor"), Context.ValidationErrors.Num() > 0);
	
	// Test with valid scaling factor
	UpgradeData->CostScalingFactor = 1.5f; // Valid: within 1.0-10.0 range
	Context.Reset();
	bIsValid = UpgradeData->Validate(Context);
	
	TestTrue(TEXT("Upgrade data should be valid with correct scaling factor"), bIsValid);
	TestEqual(TEXT("Should have no errors with valid data"), Context.ValidationErrors.Num(), 0);
	
	return true;
}

/**
 * Test weapon data validation with invalid cooldown
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepWeaponDataValidationTest, 
	"DelveDeep.Validation.WeaponData", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepWeaponDataValidationTest::RunTest(const FString& Parameters)
{
	// Create weapon data with invalid attack speed (acts as cooldown)
	UDelveDeepWeaponData* WeaponData = NewObject<UDelveDeepWeaponData>();
	WeaponData->BaseDamage = 10.0f;      // Valid
	WeaponData->AttackSpeed = 0.05f;     // Invalid: below min of 0.1
	WeaponData->Range = 100.0f;          // Valid
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("TestWeaponValidation");
	
	// Validate - should fail due to invalid attack speed
	bool bIsValid = WeaponData->Validate(Context);
	
	TestFalse(TEXT("Weapon data should be invalid with low attack speed"), bIsValid);
	TestTrue(TEXT("Should have at least one error"), Context.ValidationErrors.Num() > 0);
	TestTrue(TEXT("Error should mention AttackSpeed"), 
		Context.ValidationErrors[0].Contains(TEXT("AttackSpeed")));
	
	// Test with invalid base damage
	WeaponData->AttackSpeed = 1.0f;  // Fix attack speed
	WeaponData->BaseDamage = 0.0f;   // Invalid: must be positive
	Context.Reset();
	bIsValid = WeaponData->Validate(Context);
	
	TestFalse(TEXT("Weapon data should be invalid with zero damage"), bIsValid);
	TestTrue(TEXT("Should have error for non-positive damage"), Context.ValidationErrors.Num() > 0);
	
	// Test with valid values
	WeaponData->BaseDamage = 10.0f; // Fix damage
	Context.Reset();
	bIsValid = WeaponData->Validate(Context);
	
	TestTrue(TEXT("Weapon data should be valid with correct values"), bIsValid);
	TestEqual(TEXT("Should have no errors with valid data"), Context.ValidationErrors.Num(), 0);
	
	return true;
}

/**
 * Test ability data validation with invalid resource cost
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepAbilityDataValidationTest, 
	"DelveDeep.Validation.AbilityData", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepAbilityDataValidationTest::RunTest(const FString& Parameters)
{
	// Create ability data with invalid resource cost
	UDelveDeepAbilityData* AbilityData = NewObject<UDelveDeepAbilityData>();
	AbilityData->Cooldown = 5.0f;           // Valid
	AbilityData->ResourceCost = -10.0f;     // Invalid: cannot be negative
	AbilityData->DamageMultiplier = 1.5f;   // Valid
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("TestAbilityValidation");
	
	// Validate - should fail due to negative resource cost
	bool bIsValid = AbilityData->Validate(Context);
	
	TestFalse(TEXT("Ability data should be invalid with negative resource cost"), bIsValid);
	TestTrue(TEXT("Should have at least one error"), Context.ValidationErrors.Num() > 0);
	TestTrue(TEXT("Error should mention ResourceCost"), 
		Context.ValidationErrors[0].Contains(TEXT("ResourceCost")));
	
	// Test with invalid cooldown
	AbilityData->ResourceCost = 10.0f;  // Fix resource cost
	AbilityData->Cooldown = 0.05f;      // Invalid: below min of 0.1
	Context.Reset();
	bIsValid = AbilityData->Validate(Context);
	
	TestFalse(TEXT("Ability data should be invalid with low cooldown"), bIsValid);
	TestTrue(TEXT("Should have error for invalid cooldown"), Context.ValidationErrors.Num() > 0);
	
	// Test with valid values
	AbilityData->Cooldown = 5.0f; // Fix cooldown
	Context.Reset();
	bIsValid = AbilityData->Validate(Context);
	
	TestTrue(TEXT("Ability data should be valid with correct values"), bIsValid);
	TestEqual(TEXT("Should have no errors with valid data"), Context.ValidationErrors.Num(), 0);
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
