// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestMacros.h"
#include "DelveDeepTestUtilities.h"
#include "DelveDeepTestReport.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test report generation from test results
 * Verifies that test reports can be generated from an array of test results
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestReportGenerationTest,
	"DelveDeep.Testing.ReportGeneration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestReportGenerationTest::RunTest(const FString& Parameters)
{
	// Create sample test results
	TArray<FDelveDeepTestResult> Results;

	// Add passed test
	FDelveDeepTestResult PassedTest;
	PassedTest.TestName = TEXT("SamplePassedTest");
	PassedTest.TestPath = TEXT("DelveDeep.Configuration.SamplePassedTest");
	PassedTest.bPassed = true;
	PassedTest.ExecutionTime = 0.123f;
	PassedTest.TestSuite = TEXT("Configuration");
	Results.Add(PassedTest);

	// Add failed test
	FDelveDeepTestResult FailedTest;
	FailedTest.TestName = TEXT("SampleFailedTest");
	FailedTest.TestPath = TEXT("DelveDeep.Events.SampleFailedTest");
	FailedTest.bPassed = false;
	FailedTest.ExecutionTime = 0.456f;
	FailedTest.TestSuite = TEXT("Events");
	FailedTest.Errors.Add(TEXT("Expected value to be 10, but was 5"));
	FailedTest.Warnings.Add(TEXT("Test took longer than expected"));
	Results.Add(FailedTest);

	// Add another passed test
	FDelveDeepTestResult AnotherPassedTest;
	AnotherPassedTest.TestName = TEXT("AnotherPassedTest");
	AnotherPassedTest.TestPath = TEXT("DelveDeep.Configuration.AnotherPassedTest");
	AnotherPassedTest.bPassed = true;
	AnotherPassedTest.ExecutionTime = 0.089f;
	AnotherPassedTest.TestSuite = TEXT("Configuration");
	Results.Add(AnotherPassedTest);

	// Generate report
	FDelveDeepTestReport Report = FTestReportGenerator::GenerateReportFromResults(Results, TEXT("1.0.0"));

	// Verify report statistics
	EXPECT_EQ(Report.TotalTests, 3);
	EXPECT_EQ(Report.PassedTests, 2);
	EXPECT_EQ(Report.FailedTests, 1);
	EXPECT_NEAR(Report.TotalExecutionTime, 0.668f, 0.001f);
	EXPECT_NEAR(Report.GetPassRate(), 66.67f, 0.1f);
	EXPECT_NEAR(Report.GetAverageExecutionTime(), 0.223f, 0.001f);

	// Verify suite breakdown
	EXPECT_EQ(Report.TestsBySuite.Num(), 2);
	EXPECT_EQ(*Report.TestsBySuite.Find(TEXT("Configuration")), 2);
	EXPECT_EQ(*Report.TestsBySuite.Find(TEXT("Events")), 1);

	// Verify execution time by suite
	EXPECT_NEAR(*Report.ExecutionTimeBySuite.Find(TEXT("Configuration")), 0.212f, 0.001f);
	EXPECT_NEAR(*Report.ExecutionTimeBySuite.Find(TEXT("Events")), 0.456f, 0.001f);

	// Verify results are stored
	EXPECT_EQ(Report.Results.Num(), 3);

	return true;
}

/**
 * Test Markdown export functionality
 * Verifies that test reports can be exported to Markdown format
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestReportMarkdownExportTest,
	"DelveDeep.Testing.MarkdownExport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestReportMarkdownExportTest::RunTest(const FString& Parameters)
{
	// Create sample test results
	TArray<FDelveDeepTestResult> Results;

	FDelveDeepTestResult PassedTest;
	PassedTest.TestName = TEXT("MarkdownTest");
	PassedTest.TestPath = TEXT("DelveDeep.Testing.MarkdownTest");
	PassedTest.bPassed = true;
	PassedTest.ExecutionTime = 0.1f;
	PassedTest.TestSuite = TEXT("Testing");
	Results.Add(PassedTest);

	// Generate report
	FDelveDeepTestReport Report = FTestReportGenerator::GenerateReportFromResults(Results, TEXT("1.0.0"));

	// Export to Markdown
	FString OutputPath = FPaths::ProjectSavedDir() / TEXT("TestReports") / TEXT("test_report.md");
	bool bExportSuccess = FTestReportGenerator::ExportToMarkdown(Report, OutputPath);

	EXPECT_TRUE(bExportSuccess);

	// Verify file exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	EXPECT_TRUE(PlatformFile.FileExists(*OutputPath));

	// Clean up
	if (PlatformFile.FileExists(*OutputPath))
	{
		PlatformFile.DeleteFile(*OutputPath);
	}

	return true;
}

/**
 * Test HTML export functionality
 * Verifies that test reports can be exported to HTML format
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestReportHTMLExportTest,
	"DelveDeep.Testing.HTMLExport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestReportHTMLExportTest::RunTest(const FString& Parameters)
{
	// Create sample test results
	TArray<FDelveDeepTestResult> Results;

	FDelveDeepTestResult PassedTest;
	PassedTest.TestName = TEXT("HTMLTest");
	PassedTest.TestPath = TEXT("DelveDeep.Testing.HTMLTest");
	PassedTest.bPassed = true;
	PassedTest.ExecutionTime = 0.1f;
	PassedTest.TestSuite = TEXT("Testing");
	Results.Add(PassedTest);

	FDelveDeepTestResult FailedTest;
	FailedTest.TestName = TEXT("HTMLFailedTest");
	FailedTest.TestPath = TEXT("DelveDeep.Testing.HTMLFailedTest");
	FailedTest.bPassed = false;
	FailedTest.ExecutionTime = 0.2f;
	FailedTest.TestSuite = TEXT("Testing");
	FailedTest.Errors.Add(TEXT("Sample error message"));
	Results.Add(FailedTest);

	// Generate report
	FDelveDeepTestReport Report = FTestReportGenerator::GenerateReportFromResults(Results, TEXT("1.0.0"));

	// Export to HTML
	FString OutputPath = FPaths::ProjectSavedDir() / TEXT("TestReports") / TEXT("test_report.html");
	bool bExportSuccess = FTestReportGenerator::ExportToHTML(Report, OutputPath);

	EXPECT_TRUE(bExportSuccess);

	// Verify file exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	EXPECT_TRUE(PlatformFile.FileExists(*OutputPath));

	// Clean up
	if (PlatformFile.FileExists(*OutputPath))
	{
		PlatformFile.DeleteFile(*OutputPath);
	}

	return true;
}

/**
 * Test JUnit XML export functionality
 * Verifies that test reports can be exported to JUnit XML format for CI/CD integration
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestReportJUnitExportTest,
	"DelveDeep.Testing.JUnitExport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestReportJUnitExportTest::RunTest(const FString& Parameters)
{
	// Create sample test results
	TArray<FDelveDeepTestResult> Results;

	FDelveDeepTestResult PassedTest;
	PassedTest.TestName = TEXT("JUnitTest");
	PassedTest.TestPath = TEXT("DelveDeep.Testing.JUnitTest");
	PassedTest.bPassed = true;
	PassedTest.ExecutionTime = 0.1f;
	PassedTest.TestSuite = TEXT("Testing");
	Results.Add(PassedTest);

	FDelveDeepTestResult FailedTest;
	FailedTest.TestName = TEXT("JUnitFailedTest");
	FailedTest.TestPath = TEXT("DelveDeep.Testing.JUnitFailedTest");
	FailedTest.bPassed = false;
	FailedTest.ExecutionTime = 0.2f;
	FailedTest.TestSuite = TEXT("Testing");
	FailedTest.Errors.Add(TEXT("Sample error for JUnit"));
	Results.Add(FailedTest);

	// Generate report
	FDelveDeepTestReport Report = FTestReportGenerator::GenerateReportFromResults(Results, TEXT("1.0.0"));

	// Export to JUnit XML
	FString OutputPath = FPaths::ProjectSavedDir() / TEXT("TestReports") / TEXT("test_report.xml");
	bool bExportSuccess = FTestReportGenerator::ExportToJUnit(Report, OutputPath);

	EXPECT_TRUE(bExportSuccess);

	// Verify file exists
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	EXPECT_TRUE(PlatformFile.FileExists(*OutputPath));

	// Clean up
	if (PlatformFile.FileExists(*OutputPath))
	{
		PlatformFile.DeleteFile(*OutputPath);
	}

	return true;
}

/**
 * Test suite extraction from test path
 * Verifies that test suite names are correctly extracted from test paths
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestSuiteExtractionTest,
	"DelveDeep.Testing.SuiteExtraction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestSuiteExtractionTest::RunTest(const FString& Parameters)
{
	// Create test results with different path formats
	TArray<FDelveDeepTestResult> Results;

	FDelveDeepTestResult Test1;
	Test1.TestName = TEXT("Test1");
	Test1.TestPath = TEXT("DelveDeep.Configuration.AssetCaching");
	Test1.bPassed = true;
	Test1.ExecutionTime = 0.1f;
	Results.Add(Test1);

	FDelveDeepTestResult Test2;
	Test2.TestName = TEXT("Test2");
	Test2.TestPath = TEXT("DelveDeep.Events.Broadcasting");
	Test2.bPassed = true;
	Test2.ExecutionTime = 0.1f;
	Results.Add(Test2);

	FDelveDeepTestResult Test3;
	Test3.TestName = TEXT("Test3");
	Test3.TestPath = TEXT("DelveDeep.Telemetry.FrameTracking");
	Test3.bPassed = true;
	Test3.ExecutionTime = 0.1f;
	Results.Add(Test3);

	// Generate report
	FDelveDeepTestReport Report = FTestReportGenerator::GenerateReportFromResults(Results);

	// Verify suites were extracted correctly
	EXPECT_EQ(Report.TestsBySuite.Num(), 3);
	EXPECT_NOT_NULL(Report.TestsBySuite.Find(TEXT("Configuration")));
	EXPECT_NOT_NULL(Report.TestsBySuite.Find(TEXT("Events")));
	EXPECT_NOT_NULL(Report.TestsBySuite.Find(TEXT("Telemetry")));

	return true;
}

/**
 * Test report statistics calculation
 * Verifies that report statistics are calculated correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTestReportStatisticsTest,
	"DelveDeep.Testing.ReportStatistics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTestReportStatisticsTest::RunTest(const FString& Parameters)
{
	// Create test results with known values
	TArray<FDelveDeepTestResult> Results;

	// Add 7 passed tests
	for (int32 i = 0; i < 7; ++i)
	{
		FDelveDeepTestResult Test;
		Test.TestName = FString::Printf(TEXT("PassedTest%d"), i);
		Test.TestPath = FString::Printf(TEXT("DelveDeep.Testing.PassedTest%d"), i);
		Test.bPassed = true;
		Test.ExecutionTime = 0.1f;
		Test.TestSuite = TEXT("Testing");
		Results.Add(Test);
	}

	// Add 3 failed tests
	for (int32 i = 0; i < 3; ++i)
	{
		FDelveDeepTestResult Test;
		Test.TestName = FString::Printf(TEXT("FailedTest%d"), i);
		Test.TestPath = FString::Printf(TEXT("DelveDeep.Testing.FailedTest%d"), i);
		Test.bPassed = false;
		Test.ExecutionTime = 0.2f;
		Test.TestSuite = TEXT("Testing");
		Results.Add(Test);
	}

	// Generate report
	FDelveDeepTestReport Report = FTestReportGenerator::GenerateReportFromResults(Results);

	// Verify statistics
	EXPECT_EQ(Report.TotalTests, 10);
	EXPECT_EQ(Report.PassedTests, 7);
	EXPECT_EQ(Report.FailedTests, 3);
	EXPECT_NEAR(Report.GetPassRate(), 70.0f, 0.1f);
	EXPECT_NEAR(Report.TotalExecutionTime, 1.3f, 0.01f); // 7 * 0.1 + 3 * 0.2
	EXPECT_NEAR(Report.GetAverageExecutionTime(), 0.13f, 0.01f);
	EXPECT_FALSE(Report.AllTestsPassed());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
