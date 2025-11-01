// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"
#include "DelveDeepValidationSubsystem.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test FDelveDeepValidationContext error tracking functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextErrorTest, 
	"DelveDeep.Validation.ErrorTracking", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextErrorTest::RunTest(const FString& Parameters)
{
	// Create validation context
	FDelveDeepValidationContext Context;
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
 * Test FDelveDeepValidationContext report generation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextReportTest, 
	"DelveDeep.Validation.ReportGeneration", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextReportTest::RunTest(const FString& Parameters)
{
	// Create validation context with errors and warnings
	FDelveDeepValidationContext Context;
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
 * Test FDelveDeepValidationContext reset functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextResetTest, 
	"DelveDeep.Validation.Reset", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextResetTest::RunTest(const FString& Parameters)
{
	// Create validation context with data
	FDelveDeepValidationContext Context;
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
 * Test FDelveDeepValidationContext with multiple errors and warnings
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextMultipleIssuesTest, 
	"DelveDeep.Validation.MultipleIssues", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextMultipleIssuesTest::RunTest(const FString& Parameters)
{
	// Create validation context
	FDelveDeepValidationContext Context;
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
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepCharacterDataValidationTest::RunTest(const FString& Parameters)
{
	// Create character data with invalid health
	UDelveDeepCharacterData* CharacterData = NewObject<UDelveDeepCharacterData>();
	CharacterData->BaseHealth = -10.0f; // Invalid: must be positive
	CharacterData->BaseDamage = 15.0f;  // Valid
	CharacterData->MoveSpeed = 300.0f;  // Valid
	
	FDelveDeepValidationContext Context;
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
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepMonsterConfigValidationTest::RunTest(const FString& Parameters)
{
	// Create monster config with negative damage
	FDelveDeepMonsterConfig MonsterConfig;
	MonsterConfig.Health = 50.0f;    // Valid
	MonsterConfig.Damage = -5.0f;    // Invalid: cannot be negative
	MonsterConfig.MoveSpeed = 200.0f; // Valid
	MonsterConfig.DetectionRange = 500.0f; // Valid
	MonsterConfig.AttackRange = 100.0f;    // Valid
	
	FDelveDeepValidationContext Context;
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
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepUpgradeDataValidationTest::RunTest(const FString& Parameters)
{
	// Create upgrade data with out-of-range scaling factor
	UDelveDeepUpgradeData* UpgradeData = NewObject<UDelveDeepUpgradeData>();
	UpgradeData->BaseCost = 100;              // Valid
	UpgradeData->CostScalingFactor = 15.0f;   // Invalid: exceeds max of 10.0
	UpgradeData->MaxLevel = 10;               // Valid
	UpgradeData->HealthModifier = 10.0f;      // Valid
	
	FDelveDeepValidationContext Context;
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
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepWeaponDataValidationTest::RunTest(const FString& Parameters)
{
	// Create weapon data with invalid attack speed (acts as cooldown)
	UDelveDeepWeaponData* WeaponData = NewObject<UDelveDeepWeaponData>();
	WeaponData->BaseDamage = 10.0f;      // Valid
	WeaponData->AttackSpeed = 0.05f;     // Invalid: below min of 0.1
	WeaponData->Range = 100.0f;          // Valid
	
	FDelveDeepValidationContext Context;
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
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepAbilityDataValidationTest::RunTest(const FString& Parameters)
{
	// Create ability data with invalid resource cost
	UDelveDeepAbilityData* AbilityData = NewObject<UDelveDeepAbilityData>();
	AbilityData->Cooldown = 5.0f;           // Valid
	AbilityData->ResourceCost = -10.0f;     // Invalid: cannot be negative
	AbilityData->DamageMultiplier = 1.5f;   // Valid
	
	FDelveDeepValidationContext Context;
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

/**
 * Test validation context nesting functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextNestingTest, 
	"DelveDeep.Validation.ContextNesting", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextNestingTest::RunTest(const FString& Parameters)
{
	// Create parent context
	FDelveDeepValidationContext ParentContext;
	ParentContext.SystemName = TEXT("ParentSystem");
	ParentContext.OperationName = TEXT("ParentOperation");
	ParentContext.AddError(TEXT("Parent error"));
	
	// Create child context
	FDelveDeepValidationContext ChildContext;
	ChildContext.SystemName = TEXT("ChildSystem");
	ChildContext.OperationName = TEXT("ChildOperation");
	ChildContext.AddError(TEXT("Child error"));
	ChildContext.AddWarning(TEXT("Child warning"));
	
	// Add child to parent
	ParentContext.AddChildContext(ChildContext);
	
	// Verify child was added
	TestEqual(TEXT("Parent should have one child context"), ParentContext.ChildContexts.Num(), 1);
	TestEqual(TEXT("Child context should have correct system name"), 
		ParentContext.ChildContexts[0].SystemName, TEXT("ChildSystem"));
	
	// Verify parent report includes child issues
	FString Report = ParentContext.GetReport();
	TestTrue(TEXT("Report should contain parent error"), Report.Contains(TEXT("Parent error")));
	TestTrue(TEXT("Report should contain child error"), Report.Contains(TEXT("Child error")));
	TestTrue(TEXT("Report should contain child warning"), Report.Contains(TEXT("Child warning")));
	TestTrue(TEXT("Report should show nested contexts section"), Report.Contains(TEXT("NESTED CONTEXTS")));
	
	return true;
}

/**
 * Test validation context merging functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextMergingTest, 
	"DelveDeep.Validation.ContextMerging", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextMergingTest::RunTest(const FString& Parameters)
{
	// Create first context
	FDelveDeepValidationContext Context1;
	Context1.SystemName = TEXT("System1");
	Context1.OperationName = TEXT("Operation1");
	Context1.AddError(TEXT("Error from context 1"));
	Context1.AddWarning(TEXT("Warning from context 1"));
	
	// Create second context
	FDelveDeepValidationContext Context2;
	Context2.SystemName = TEXT("System2");
	Context2.OperationName = TEXT("Operation2");
	Context2.AddError(TEXT("Error from context 2"));
	Context2.AddWarning(TEXT("Warning from context 2"));
	
	// Store initial counts
	int32 InitialErrors = Context1.ValidationErrors.Num();
	int32 InitialWarnings = Context1.ValidationWarnings.Num();
	int32 InitialIssues = Context1.Issues.Num();
	
	// Merge context2 into context1
	Context1.MergeContext(Context2);
	
	// Verify merge results
	TestEqual(TEXT("Should have combined errors"), 
		Context1.ValidationErrors.Num(), InitialErrors + Context2.ValidationErrors.Num());
	TestEqual(TEXT("Should have combined warnings"), 
		Context1.ValidationWarnings.Num(), InitialWarnings + Context2.ValidationWarnings.Num());
	TestEqual(TEXT("Should have combined issues"), 
		Context1.Issues.Num(), InitialIssues + Context2.Issues.Num());
	
	// Verify merged content
	FString Report = Context1.GetReport();
	TestTrue(TEXT("Report should contain error from context 1"), 
		Report.Contains(TEXT("Error from context 1")));
	TestTrue(TEXT("Report should contain error from context 2"), 
		Report.Contains(TEXT("Error from context 2")));
	TestTrue(TEXT("Report should contain warning from context 1"), 
		Report.Contains(TEXT("Warning from context 1")));
	TestTrue(TEXT("Report should contain warning from context 2"), 
		Report.Contains(TEXT("Warning from context 2")));
	
	return true;
}

/**
 * Test validation context metadata attachment
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextMetadataTest, 
	"DelveDeep.Validation.MetadataAttachment", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextMetadataTest::RunTest(const FString& Parameters)
{
	// Create context and add issue
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("TestSystem");
	Context.OperationName = TEXT("TestOperation");
	Context.AddError(TEXT("Test error with metadata"));
	
	// Attach metadata to the issue
	Context.AttachMetadata(TEXT("AssetPath"), TEXT("/Game/Data/TestAsset"));
	Context.AttachMetadata(TEXT("PropertyName"), TEXT("BaseHealth"));
	
	// Verify metadata was attached
	TestEqual(TEXT("Should have one issue"), Context.Issues.Num(), 1);
	TestEqual(TEXT("Issue should have two metadata entries"), Context.Issues[0].Metadata.Num(), 2);
	TestTrue(TEXT("Should have AssetPath metadata"), 
		Context.Issues[0].Metadata.Contains(TEXT("AssetPath")));
	TestTrue(TEXT("Should have PropertyName metadata"), 
		Context.Issues[0].Metadata.Contains(TEXT("PropertyName")));
	TestEqual(TEXT("AssetPath value should match"), 
		Context.Issues[0].Metadata[TEXT("AssetPath")], TEXT("/Game/Data/TestAsset"));
	TestEqual(TEXT("PropertyName value should match"), 
		Context.Issues[0].Metadata[TEXT("PropertyName")], TEXT("BaseHealth"));
	
	// Verify metadata appears in report
	FString Report = Context.GetReport();
	TestTrue(TEXT("Report should contain metadata"), Report.Contains(TEXT("Metadata:")));
	TestTrue(TEXT("Report should contain AssetPath"), Report.Contains(TEXT("AssetPath")));
	TestTrue(TEXT("Report should contain PropertyName"), Report.Contains(TEXT("PropertyName")));
	
	return true;
}

/**
 * Test validation duration calculation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationContextDurationTest, 
	"DelveDeep.Validation.DurationCalculation", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationContextDurationTest::RunTest(const FString& Parameters)
{
	// Create context
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("TestSystem");
	Context.OperationName = TEXT("TestOperation");
	
	// Record creation time
	FDateTime StartTime = Context.CreationTime;
	
	// Simulate some work
	FPlatformProcess::Sleep(0.01f); // Sleep for 10ms
	
	// Get duration before completion
	FTimespan DurationInProgress = Context.GetValidationDuration();
	TestTrue(TEXT("Duration should be positive while in progress"), 
		DurationInProgress.GetTotalMilliseconds() > 0);
	
	// Mark as completed
	Context.CompletionTime = FDateTime::Now();
	
	// Get final duration
	FTimespan FinalDuration = Context.GetValidationDuration();
	TestTrue(TEXT("Final duration should be positive"), 
		FinalDuration.GetTotalMilliseconds() > 0);
	TestTrue(TEXT("Final duration should be at least 10ms"), 
		FinalDuration.GetTotalMilliseconds() >= 10.0);
	
	// Verify duration appears in report
	FString Report = Context.GetReport();
	TestTrue(TEXT("Report should contain duration"), Report.Contains(TEXT("Duration:")));
	
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

/**
 * Test validation subsystem initialization and cleanup
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationSubsystemInitTest, 
	"DelveDeep.Validation.SubsystemInitialization", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationSubsystemInitTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	
	// Get validation subsystem (auto-initializes)
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	TestTrue(TEXT("Validation subsystem should be created"), ValidationSubsystem != nullptr);
	
	if (ValidationSubsystem)
	{
		// Verify initial state
		TestEqual(TEXT("Should have no rules initially"), 
			ValidationSubsystem->GetAllRules().Num(), 0);
	}
	
	return true;
}

/**
 * Test validation rule registration
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationRuleRegistrationTest, 
	"DelveDeep.Validation.RuleRegistration", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationRuleRegistrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Create a simple validation rule
	FValidationRuleDelegate TestRule;
	TestRule.BindLambda([](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		Context.AddInfo(TEXT("Test rule executed"));
		return true;
	});
	
	// Register the rule
	ValidationSubsystem->RegisterValidationRule(
		TEXT("TestRule"),
		UObject::StaticClass(),
		TestRule,
		100,
		TEXT("Test validation rule")
	);
	
	// Verify rule was registered
	TestEqual(TEXT("Should have one rule registered"), 
		ValidationSubsystem->GetRuleCountForClass(UObject::StaticClass()), 1);
	
	TArray<FValidationRuleDefinition> Rules = 
		ValidationSubsystem->GetRulesForClass(UObject::StaticClass());
	
	TestEqual(TEXT("Should return one rule"), Rules.Num(), 1);
	TestEqual(TEXT("Rule name should match"), Rules[0].RuleName, FName(TEXT("TestRule")));
	TestEqual(TEXT("Rule priority should match"), Rules[0].Priority, 100);
	TestEqual(TEXT("Rule description should match"), 
		Rules[0].Description, TEXT("Test validation rule"));
	
	return true;
}

/**
 * Test validation rule execution
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationRuleExecutionTest, 
	"DelveDeep.Validation.RuleExecution", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationRuleExecutionTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Create a validation rule that adds an error
	FValidationRuleDelegate FailingRule;
	FailingRule.BindLambda([](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		Context.AddError(TEXT("Validation failed"));
		return false;
	});
	
	// Register the rule
	ValidationSubsystem->RegisterValidationRule(
		TEXT("FailingRule"),
		UObject::StaticClass(),
		FailingRule,
		0,
		TEXT("Rule that always fails")
	);
	
	// Create test object
	UObject* TestObject = NewObject<UObject>();
	
	// Validate object
	FDelveDeepValidationContext Context;
	bool bResult = ValidationSubsystem->ValidateObject(TestObject, Context);
	
	// Verify rule was executed
	TestFalse(TEXT("Validation should fail"), bResult);
	TestTrue(TEXT("Context should have child contexts"), Context.ChildContexts.Num() > 0);
	
	// Find the child context for our rule
	bool bFoundRuleContext = false;
	for (const FDelveDeepValidationContext& ChildContext : Context.ChildContexts)
	{
		if (ChildContext.OperationName == TEXT("FailingRule"))
		{
			bFoundRuleContext = true;
			TestTrue(TEXT("Rule context should have errors"), 
				ChildContext.ValidationErrors.Num() > 0);
			TestEqual(TEXT("Error message should match"), 
				ChildContext.ValidationErrors[0], TEXT("Validation failed"));
		}
	}
	
	TestTrue(TEXT("Should find rule context in child contexts"), bFoundRuleContext);
	
	return true;
}

/**
 * Test validation rule priority ordering
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationRulePriorityTest, 
	"DelveDeep.Validation.RulePriority", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationRulePriorityTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Register rules with different priorities
	TArray<FString> ExecutionOrder;
	
	FValidationRuleDelegate LowPriorityRule;
	LowPriorityRule.BindLambda([&ExecutionOrder](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		ExecutionOrder.Add(TEXT("LowPriority"));
		return true;
	});
	
	FValidationRuleDelegate HighPriorityRule;
	HighPriorityRule.BindLambda([&ExecutionOrder](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		ExecutionOrder.Add(TEXT("HighPriority"));
		return true;
	});
	
	FValidationRuleDelegate MediumPriorityRule;
	MediumPriorityRule.BindLambda([&ExecutionOrder](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		ExecutionOrder.Add(TEXT("MediumPriority"));
		return true;
	});
	
	// Register in random order
	ValidationSubsystem->RegisterValidationRule(TEXT("LowPriority"), UObject::StaticClass(), LowPriorityRule, 10);
	ValidationSubsystem->RegisterValidationRule(TEXT("HighPriority"), UObject::StaticClass(), HighPriorityRule, 100);
	ValidationSubsystem->RegisterValidationRule(TEXT("MediumPriority"), UObject::StaticClass(), MediumPriorityRule, 50);
	
	// Validate object
	UObject* TestObject = NewObject<UObject>();
	FDelveDeepValidationContext Context;
	ValidationSubsystem->ValidateObject(TestObject, Context);
	
	// Verify execution order (higher priority first)
	TestEqual(TEXT("Should execute three rules"), ExecutionOrder.Num(), 3);
	TestEqual(TEXT("First rule should be HighPriority"), ExecutionOrder[0], TEXT("HighPriority"));
	TestEqual(TEXT("Second rule should be MediumPriority"), ExecutionOrder[1], TEXT("MediumPriority"));
	TestEqual(TEXT("Third rule should be LowPriority"), ExecutionOrder[2], TEXT("LowPriority"));
	
	return true;
}

/**
 * Test validation caching functionality
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationCachingTest, 
	"DelveDeep.Validation.Caching", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationCachingTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Track rule execution count
	int32 ExecutionCount = 0;
	
	FValidationRuleDelegate CountingRule;
	CountingRule.BindLambda([&ExecutionCount](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		ExecutionCount++;
		Context.AddInfo(FString::Printf(TEXT("Execution %d"), ExecutionCount));
		return true;
	});
	
	ValidationSubsystem->RegisterValidationRule(TEXT("CountingRule"), UObject::StaticClass(), CountingRule);
	
	// Create test object
	UObject* TestObject = NewObject<UObject>();
	
	// First validation - should execute rule
	FDelveDeepValidationContext Context1;
	ValidationSubsystem->ValidateObjectWithCache(TestObject, Context1);
	TestEqual(TEXT("Rule should execute once"), ExecutionCount, 1);
	
	// Second validation with cache - should NOT execute rule again
	FDelveDeepValidationContext Context2;
	ValidationSubsystem->ValidateObjectWithCache(TestObject, Context2);
	TestEqual(TEXT("Rule should not execute again (cached)"), ExecutionCount, 1);
	
	// Force revalidation - should execute rule again
	FDelveDeepValidationContext Context3;
	ValidationSubsystem->ValidateObjectWithCache(TestObject, Context3, true);
	TestEqual(TEXT("Rule should execute again (forced)"), ExecutionCount, 2);
	
	return true;
}

/**
 * Test validation cache invalidation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationCacheInvalidationTest, 
	"DelveDeep.Validation.CacheInvalidation", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationCacheInvalidationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Track rule execution count
	int32 ExecutionCount = 0;
	
	FValidationRuleDelegate CountingRule;
	CountingRule.BindLambda([&ExecutionCount](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		ExecutionCount++;
		return true;
	});
	
	ValidationSubsystem->RegisterValidationRule(TEXT("CountingRule"), UObject::StaticClass(), CountingRule);
	
	// Create test object
	UObject* TestObject = NewObject<UObject>();
	
	// First validation - should execute rule
	FDelveDeepValidationContext Context1;
	ValidationSubsystem->ValidateObjectWithCache(TestObject, Context1);
	TestEqual(TEXT("Rule should execute once"), ExecutionCount, 1);
	
	// Invalidate cache
	ValidationSubsystem->InvalidateCache(TestObject);
	
	// Second validation - should execute rule again (cache invalidated)
	FDelveDeepValidationContext Context2;
	ValidationSubsystem->ValidateObjectWithCache(TestObject, Context2);
	TestEqual(TEXT("Rule should execute again after invalidation"), ExecutionCount, 2);
	
	return true;
}

/**
 * Test validation cache clearing
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationCacheClearTest, 
	"DelveDeep.Validation.CacheClear", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationCacheClearTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Track rule execution count
	int32 ExecutionCount = 0;
	
	FValidationRuleDelegate CountingRule;
	CountingRule.BindLambda([&ExecutionCount](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
	{
		ExecutionCount++;
		return true;
	});
	
	ValidationSubsystem->RegisterValidationRule(TEXT("CountingRule"), UObject::StaticClass(), CountingRule);
	
	// Create multiple test objects
	UObject* TestObject1 = NewObject<UObject>();
	UObject* TestObject2 = NewObject<UObject>();
	
	// Validate both objects
	FDelveDeepValidationContext Context1;
	ValidationSubsystem->ValidateObjectWithCache(TestObject1, Context1);
	FDelveDeepValidationContext Context2;
	ValidationSubsystem->ValidateObjectWithCache(TestObject2, Context2);
	TestEqual(TEXT("Rule should execute twice"), ExecutionCount, 2);
	
	// Clear all cache
	ValidationSubsystem->ClearValidationCache();
	
	// Validate both objects again - should execute rules again
	FDelveDeepValidationContext Context3;
	ValidationSubsystem->ValidateObjectWithCache(TestObject1, Context3);
	FDelveDeepValidationContext Context4;
	ValidationSubsystem->ValidateObjectWithCache(TestObject2, Context4);
	TestEqual(TEXT("Rule should execute twice more after cache clear"), ExecutionCount, 4);
	
	return true;
}

/**
 * Test rule unregistration
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepValidationRuleUnregistrationTest, 
	"DelveDeep.Validation.RuleUnregistration", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepValidationRuleUnregistrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();
	
	if (!ValidationSubsystem)
	{
		AddError(TEXT("Failed to create validation subsystem"));
		return false;
	}
	
	// Register multiple rules
	FValidationRuleDelegate Rule1;
	Rule1.BindLambda([](const UObject* Object, FDelveDeepValidationContext& Context) -> bool { return true; });
	
	FValidationRuleDelegate Rule2;
	Rule2.BindLambda([](const UObject* Object, FDelveDeepValidationContext& Context) -> bool { return true; });
	
	ValidationSubsystem->RegisterValidationRule(TEXT("Rule1"), UObject::StaticClass(), Rule1);
	ValidationSubsystem->RegisterValidationRule(TEXT("Rule2"), UObject::StaticClass(), Rule2);
	
	TestEqual(TEXT("Should have two rules"), 
		ValidationSubsystem->GetRuleCountForClass(UObject::StaticClass()), 2);
	
	// Unregister one rule
	ValidationSubsystem->UnregisterValidationRule(TEXT("Rule1"), UObject::StaticClass());
	
	TestEqual(TEXT("Should have one rule after unregistration"), 
		ValidationSubsystem->GetRuleCountForClass(UObject::StaticClass()), 1);
	
	// Verify remaining rule is Rule2
	TArray<FValidationRuleDefinition> Rules = 
		ValidationSubsystem->GetRulesForClass(UObject::StaticClass());
	TestEqual(TEXT("Remaining rule should be Rule2"), Rules[0].RuleName, FName(TEXT("Rule2")));
	
	// Unregister all rules for class
	ValidationSubsystem->UnregisterAllRulesForClass(UObject::StaticClass());
	
	TestEqual(TEXT("Should have no rules after unregistering all"), 
		ValidationSubsystem->GetRuleCountForClass(UObject::StaticClass()), 0);
	
	return true;
}
