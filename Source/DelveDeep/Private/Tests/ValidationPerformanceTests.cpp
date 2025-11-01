// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DelveDeepValidation.h"
#include "DelveDeepValidationTemplates.h"
#include "HAL/PlatformTime.h"

/**
 * Performance test for validation template operations.
 * Verifies that validation operations meet performance targets.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FValidationTemplatePerformanceTest, "DelveDeep.Validation.Performance.Templates", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FValidationTemplatePerformanceTest::RunTest(const FString& Parameters)
{
	const int32 IterationCount = 10000;
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Performance");
	Context.OperationName = TEXT("TemplateTest");

	// Test range validation performance
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < IterationCount; ++i)
		{
			DelveDeepValidation::ValidateRange(50.0f, 0.0f, 100.0f, TEXT("TestValue"), Context);
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0; // Convert to ms
		double AvgTime = TotalTime / IterationCount;

		UE_LOG(LogTemp, Display, TEXT("Range validation: %d iterations in %.2f ms (avg: %.4f ms)"), 
			IterationCount, TotalTime, AvgTime);
		
		TestTrue(TEXT("Range validation should be fast"), AvgTime < 0.01); // Less than 0.01ms per validation
	}

	// Test pointer validation performance
	{
		UObject* TestObject = NewObject<UObject>();
		Context.Reset();
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < IterationCount; ++i)
		{
			DelveDeepValidation::ValidatePointer(TestObject, TEXT("TestObject"), Context, false);
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / IterationCount;

		UE_LOG(LogTemp, Display, TEXT("Pointer validation: %d iterations in %.2f ms (avg: %.4f ms)"), 
			IterationCount, TotalTime, AvgTime);
		
		TestTrue(TEXT("Pointer validation should be fast"), AvgTime < 0.01);
	}

	// Test string validation performance
	{
		FString TestString = TEXT("TestString");
		Context.Reset();
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < IterationCount; ++i)
		{
			DelveDeepValidation::ValidateString(TestString, TEXT("TestString"), Context, 1, 100, false);
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / IterationCount;

		UE_LOG(LogTemp, Display, TEXT("String validation: %d iterations in %.2f ms (avg: %.4f ms)"), 
			IterationCount, TotalTime, AvgTime);
		
		TestTrue(TEXT("String validation should be fast"), AvgTime < 0.01);
	}

	// Test array validation performance
	{
		TArray<int32> TestArray = {1, 2, 3, 4, 5};
		Context.Reset();
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < IterationCount; ++i)
		{
			DelveDeepValidation::ValidateArraySize(TestArray, TEXT("TestArray"), Context, 1, 10);
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / IterationCount;

		UE_LOG(LogTemp, Display, TEXT("Array validation: %d iterations in %.2f ms (avg: %.4f ms)"), 
			IterationCount, TotalTime, AvgTime);
		
		TestTrue(TEXT("Array validation should be fast"), AvgTime < 0.01);
	}

	return true;
}

/**
 * Performance test for validation context operations.
 * Verifies that context operations meet performance targets.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FValidationContextPerformanceTest, "DelveDeep.Validation.Performance.Context", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FValidationContextPerformanceTest::RunTest(const FString& Parameters)
{
	const int32 IterationCount = 10000;

	// Test issue addition performance
	{
		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("Performance");
		Context.OperationName = TEXT("IssueAddition");
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < IterationCount; ++i)
		{
			Context.AddError(FString::Printf(TEXT("Error %d"), i));
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / IterationCount;

		UE_LOG(LogTemp, Display, TEXT("Issue addition: %d iterations in %.2f ms (avg: %.4f ms)"), 
			IterationCount, TotalTime, AvgTime);
		
		TestTrue(TEXT("Issue addition should be fast"), AvgTime < 0.01);
	}

	// Test context reset performance
	{
		FDelveDeepValidationContext Context;
		Context.SystemName = TEXT("Performance");
		Context.OperationName = TEXT("ContextReset");
		
		// Add some issues first
		for (int32 i = 0; i < 100; ++i)
		{
			Context.AddError(FString::Printf(TEXT("Error %d"), i));
		}
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < IterationCount; ++i)
		{
			Context.Reset();
			Context.AddError(TEXT("Test error"));
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / IterationCount;

		UE_LOG(LogTemp, Display, TEXT("Context reset: %d iterations in %.2f ms (avg: %.4f ms)"), 
			IterationCount, TotalTime, AvgTime);
		
		TestTrue(TEXT("Context reset should be fast"), AvgTime < 0.01);
	}

	// Test context merging performance
	{
		FDelveDeepValidationContext Context1;
		Context1.SystemName = TEXT("System1");
		for (int32 i = 0; i < 50; ++i)
		{
			Context1.AddError(FString::Printf(TEXT("Error %d"), i));
		}
		
		FDelveDeepValidationContext Context2;
		Context2.SystemName = TEXT("System2");
		for (int32 i = 0; i < 50; ++i)
		{
			Context2.AddError(FString::Printf(TEXT("Error %d"), i));
		}
		
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < 1000; ++i) // Fewer iterations for merge
		{
			FDelveDeepValidationContext TempContext = Context1;
			TempContext.MergeContext(Context2);
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / 1000;

		UE_LOG(LogTemp, Display, TEXT("Context merging: 1000 iterations in %.2f ms (avg: %.4f ms)"), 
			TotalTime, AvgTime);
		
		TestTrue(TEXT("Context merging should be reasonably fast"), AvgTime < 1.0); // Less than 1ms per merge
	}

	return true;
}

/**
 * Performance test for report generation.
 * Verifies that report generation meets performance targets.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FValidationReportPerformanceTest, "DelveDeep.Validation.Performance.Reports", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FValidationReportPerformanceTest::RunTest(const FString& Parameters)
{
	// Create a context with many issues
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Performance");
	Context.OperationName = TEXT("ReportGeneration");
	
	for (int32 i = 0; i < 100; ++i)
	{
		Context.AddCritical(FString::Printf(TEXT("Critical %d"), i));
		Context.AddError(FString::Printf(TEXT("Error %d"), i));
		Context.AddWarning(FString::Printf(TEXT("Warning %d"), i));
		Context.AddInfo(FString::Printf(TEXT("Info %d"), i));
	}

	// Test console report generation
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < 100; ++i)
		{
			FString Report = Context.GetReport();
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / 100;

		UE_LOG(LogTemp, Display, TEXT("Console report generation: 100 iterations in %.2f ms (avg: %.4f ms)"), 
			TotalTime, AvgTime);
		
		TestTrue(TEXT("Console report generation should be fast"), AvgTime < 10.0); // Less than 10ms per report
	}

	// Test JSON report generation
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < 100; ++i)
		{
			FString Report = Context.GetReportJSON();
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / 100;

		UE_LOG(LogTemp, Display, TEXT("JSON report generation: 100 iterations in %.2f ms (avg: %.4f ms)"), 
			TotalTime, AvgTime);
		
		TestTrue(TEXT("JSON report generation should be fast"), AvgTime < 10.0);
	}

	// Test CSV report generation
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < 100; ++i)
		{
			FString Report = Context.GetReportCSV();
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / 100;

		UE_LOG(LogTemp, Display, TEXT("CSV report generation: 100 iterations in %.2f ms (avg: %.4f ms)"), 
			TotalTime, AvgTime);
		
		TestTrue(TEXT("CSV report generation should be fast"), AvgTime < 10.0);
	}

	// Test HTML report generation
	{
		double StartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < 100; ++i)
		{
			FString Report = Context.GetReportHTML();
		}
		double EndTime = FPlatformTime::Seconds();
		double TotalTime = (EndTime - StartTime) * 1000.0;
		double AvgTime = TotalTime / 100;

		UE_LOG(LogTemp, Display, TEXT("HTML report generation: 100 iterations in %.2f ms (avg: %.4f ms)"), 
			TotalTime, AvgTime);
		
		TestTrue(TEXT("HTML report generation should be fast"), AvgTime < 10.0);
	}

	return true;
}

/**
 * Performance test documentation for subsystem operations.
 * These tests require a full game instance and would be implemented as functional tests.
 */

/*
 * Test: Validation Subsystem Initialization Performance
 * 
 * Purpose: Verify that validation subsystem initializes within performance target.
 * 
 * Steps:
 * 1. Measure time to initialize validation subsystem
 * 2. Verify initialization time is <50ms
 * 
 * Expected Results:
 * - Initialization should complete in <50ms
 * - All internal structures should be ready for use
 */

/*
 * Test: Validation Rule Execution Performance
 * 
 * Purpose: Verify that validation rules execute within performance target.
 * 
 * Steps:
 * 1. Register a simple validation rule
 * 2. Execute the rule 10,000 times
 * 3. Measure average execution time
 * 4. Verify average time is <1ms
 * 
 * Expected Results:
 * - Average rule execution time should be <1ms
 * - Total time for 10,000 executions should be <10 seconds
 */

/*
 * Test: Validation Cache Performance
 * 
 * Purpose: Verify that validation cache provides significant performance improvement.
 * 
 * Steps:
 * 1. Validate 100 objects without cache
 * 2. Measure total time
 * 3. Validate same 100 objects with cache
 * 4. Measure total time
 * 5. Calculate performance improvement
 * 
 * Expected Results:
 * - Cached validation should be at least 10x faster
 * - Cache hit rate should be 100% on second pass
 * - Cache lookup time should be <0.1ms
 */

/*
 * Test: Batch Validation Performance
 * 
 * Purpose: Verify that batch validation meets performance target.
 * 
 * Steps:
 * 1. Create 100 test objects
 * 2. Validate all objects using batch validation
 * 3. Measure total time
 * 4. Verify time is <200ms
 * 
 * Expected Results:
 * - Total validation time should be <200ms for 100 objects
 * - Average time per object should be <2ms
 * - Parallel execution should show performance improvement over sequential
 */

/*
 * Test: Metrics Tracking Overhead
 * 
 * Purpose: Verify that metrics tracking has minimal performance impact.
 * 
 * Steps:
 * 1. Validate 1000 objects with metrics tracking enabled
 * 2. Measure total time
 * 3. Validate 1000 objects with metrics tracking disabled
 * 4. Measure total time
 * 5. Calculate overhead percentage
 * 
 * Expected Results:
 * - Metrics tracking overhead should be <5%
 * - Validation should still meet performance targets with metrics enabled
 */
