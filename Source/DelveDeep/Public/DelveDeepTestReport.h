// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepTestReport.generated.h"

/**
 * Individual test result information.
 * Contains detailed information about a single test execution.
 */
USTRUCT()
struct DELVEDEEP_API FDelveDeepTestResult
{
	GENERATED_BODY()

	/** Name of the test */
	FString TestName;

	/** Full path of the test (e.g., "DelveDeep.Configuration.AssetCaching") */
	FString TestPath;

	/** Whether the test passed */
	bool bPassed = false;

	/** Execution time in seconds */
	float ExecutionTime = 0.0f;

	/** Error messages from test failures */
	TArray<FString> Errors;

	/** Warning messages from test execution */
	TArray<FString> Warnings;

	/** Timestamp when test was executed */
	FDateTime ExecutionTimestamp;

	/** Test suite this test belongs to (e.g., "Configuration", "Events") */
	FString TestSuite;

	/** Memory allocated during test execution (bytes) */
	uint64 MemoryAllocated = 0;

	/** Number of allocations during test execution */
	int32 AllocationCount = 0;

	FDelveDeepTestResult()
		: bPassed(false)
		, ExecutionTime(0.0f)
		, ExecutionTimestamp(FDateTime::Now())
		, MemoryAllocated(0)
		, AllocationCount(0)
	{
	}
};

/**
 * Comprehensive test report with statistics and results.
 * Contains aggregated information about all test executions.
 */
USTRUCT()
struct DELVEDEEP_API FDelveDeepTestReport
{
	GENERATED_BODY()

	/** Timestamp when report was generated */
	FDateTime GenerationTime;

	/** Build version or commit hash */
	FString BuildVersion;

	/** Total number of tests executed */
	int32 TotalTests = 0;

	/** Number of tests that passed */
	int32 PassedTests = 0;

	/** Number of tests that failed */
	int32 FailedTests = 0;

	/** Number of tests that were skipped */
	int32 SkippedTests = 0;

	/** Total execution time for all tests in seconds */
	float TotalExecutionTime = 0.0f;

	/** Individual test results */
	TArray<FDelveDeepTestResult> Results;

	/** Number of tests per suite */
	TMap<FString, int32> TestsBySuite;

	/** Execution time per suite in seconds */
	TMap<FString, float> ExecutionTimeBySuite;

	/** Total memory allocated across all tests (bytes) */
	uint64 TotalMemoryAllocated = 0;

	/** Total number of allocations across all tests */
	int32 TotalAllocations = 0;

	FDelveDeepTestReport()
		: GenerationTime(FDateTime::Now())
		, TotalTests(0)
		, PassedTests(0)
		, FailedTests(0)
		, SkippedTests(0)
		, TotalExecutionTime(0.0f)
		, TotalMemoryAllocated(0)
		, TotalAllocations(0)
	{
	}

	/**
	 * Gets the pass rate as a percentage.
	 * 
	 * @return Pass rate (0.0 to 100.0)
	 */
	float GetPassRate() const
	{
		if (TotalTests == 0)
		{
			return 0.0f;
		}
		return (static_cast<float>(PassedTests) / static_cast<float>(TotalTests)) * 100.0f;
	}

	/**
	 * Gets the average execution time per test.
	 * 
	 * @return Average execution time in seconds
	 */
	float GetAverageExecutionTime() const
	{
		if (TotalTests == 0)
		{
			return 0.0f;
		}
		return TotalExecutionTime / static_cast<float>(TotalTests);
	}

	/**
	 * Checks if all tests passed.
	 * 
	 * @return True if all tests passed, false otherwise
	 */
	bool AllTestsPassed() const
	{
		return FailedTests == 0 && TotalTests > 0;
	}
};

/**
 * Test report generator for parsing Unreal's test output and generating reports.
 * Supports multiple output formats including Markdown and HTML.
 */
class DELVEDEEP_API FTestReportGenerator
{
public:
	/**
	 * Generates a test report from Unreal's automation test output.
	 * Parses the test results and creates a comprehensive report.
	 * 
	 * @param ReportPath Path to Unreal's test output file
	 * @return Generated test report
	 */
	static FDelveDeepTestReport GenerateReport(const FString& ReportPath);

	/**
	 * Generates a test report from an array of test results.
	 * 
	 * @param Results Array of test results
	 * @param BuildVersion Optional build version string
	 * @return Generated test report
	 */
	static FDelveDeepTestReport GenerateReportFromResults(
		const TArray<FDelveDeepTestResult>& Results,
		const FString& BuildVersion = TEXT(""));

	/**
	 * Exports a test report to Markdown format.
	 * Creates a human-readable report with tables and statistics.
	 * 
	 * @param Report The test report to export
	 * @param OutputPath Path where the Markdown file should be saved
	 * @return True if export succeeded, false otherwise
	 */
	static bool ExportToMarkdown(const FDelveDeepTestReport& Report, const FString& OutputPath);

	/**
	 * Exports a test report to HTML format.
	 * Creates a web-based report with styling and interactive elements.
	 * 
	 * @param Report The test report to export
	 * @param OutputPath Path where the HTML file should be saved
	 * @return True if export succeeded, false otherwise
	 */
	static bool ExportToHTML(const FDelveDeepTestReport& Report, const FString& OutputPath);

	/**
	 * Exports a test report to JUnit XML format.
	 * Creates an XML report compatible with CI/CD systems.
	 * 
	 * @param Report The test report to export
	 * @param OutputPath Path where the XML file should be saved
	 * @return True if export succeeded, false otherwise
	 */
	static bool ExportToJUnit(const FDelveDeepTestReport& Report, const FString& OutputPath);

private:
	/**
	 * Parses a single test result line from Unreal's output.
	 * 
	 * @param Line The line to parse
	 * @param OutResult The parsed test result
	 * @return True if parsing succeeded, false otherwise
	 */
	static bool ParseTestResultLine(const FString& Line, FDelveDeepTestResult& OutResult);

	/**
	 * Extracts the test suite name from a test path.
	 * 
	 * @param TestPath Full test path (e.g., "DelveDeep.Configuration.AssetCaching")
	 * @return Test suite name (e.g., "Configuration")
	 */
	static FString ExtractTestSuite(const FString& TestPath);

	/**
	 * Formats execution time as a human-readable string.
	 * 
	 * @param TimeSeconds Time in seconds
	 * @return Formatted time string (e.g., "1.23s", "456ms")
	 */
	static FString FormatExecutionTime(float TimeSeconds);

	/**
	 * Formats memory size as a human-readable string.
	 * 
	 * @param Bytes Memory size in bytes
	 * @return Formatted memory string (e.g., "1.5 MB", "512 KB")
	 */
	static FString FormatMemorySize(uint64 Bytes);

	/**
	 * Generates HTML header with CSS styling.
	 * 
	 * @return HTML header string
	 */
	static FString GenerateHTMLHeader();

	/**
	 * Generates HTML footer.
	 * 
	 * @return HTML footer string
	 */
	static FString GenerateHTMLFooter();

	/**
	 * Generates summary section for Markdown report.
	 * 
	 * @param Report The test report
	 * @return Markdown summary section
	 */
	static FString GenerateMarkdownSummary(const FDelveDeepTestReport& Report);

	/**
	 * Generates test results table for Markdown report.
	 * 
	 * @param Report The test report
	 * @return Markdown test results table
	 */
	static FString GenerateMarkdownResultsTable(const FDelveDeepTestReport& Report);

	/**
	 * Generates suite breakdown for Markdown report.
	 * 
	 * @param Report The test report
	 * @return Markdown suite breakdown section
	 */
	static FString GenerateMarkdownSuiteBreakdown(const FDelveDeepTestReport& Report);
};
