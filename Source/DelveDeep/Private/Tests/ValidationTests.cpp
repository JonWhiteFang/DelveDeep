// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"
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

#endif // WITH_DEV_AUTOMATION_TESTS
