// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * DelveDeep Test Documentation Generator
 * 
 * Utilities for extracting test metadata and generating documentation.
 * Provides automatic documentation generation organized by system and suite,
 * with requirement traceability showing which tests verify which requirements.
 */

/**
 * Metadata for a single test case.
 * Contains information about the test's purpose, requirements, and categorization.
 */
struct DELVEDEEP_API FTestMetadata
{
	/** Full name of the test (e.g., "DelveDeep.Configuration.CacheHitRate") */
	FString TestName;

	/** Test suite this test belongs to (e.g., "Configuration", "Events") */
	FString TestSuite;

	/** System being tested (e.g., "Configuration", "Events", "Telemetry") */
	FString SystemName;

	/** Human-readable description of what the test verifies */
	FString Description;

	/** Array of requirement IDs this test verifies (e.g., ["1.1", "3.3", "7.1"]) */
	TArray<FString> Requirements;

	/** Tags for categorization (e.g., "unit", "integration", "performance") */
	TArray<FString> Tags;

	/** Expected execution time in seconds (0 if not specified) */
	float ExpectedDuration;

	/** Test flags (ProductFilter, PerfFilter, etc.) */
	uint32 TestFlags;

	/** Whether this is a complex test (has fixture) or simple test */
	bool bIsComplexTest;

	FTestMetadata()
		: ExpectedDuration(0.0f)
		, TestFlags(0)
		, bIsComplexTest(false)
	{
	}
};

/**
 * Documentation for a test suite.
 * Groups related tests together with summary information.
 */
struct DELVEDEEP_API FTestSuiteDocumentation
{
	/** Name of the test suite (e.g., "Configuration Tests") */
	FString SuiteName;

	/** System this suite belongs to (e.g., "Configuration") */
	FString SystemName;

	/** Description of what this suite tests */
	FString Description;

	/** Array of tests in this suite */
	TArray<FTestMetadata> Tests;

	/** Total number of tests in suite */
	int32 TotalTests;

	/** Number of unit tests */
	int32 UnitTests;

	/** Number of integration tests */
	int32 IntegrationTests;

	/** Number of performance tests */
	int32 PerformanceTests;

	FTestSuiteDocumentation()
		: TotalTests(0)
		, UnitTests(0)
		, IntegrationTests(0)
		, PerformanceTests(0)
	{
	}
};

/**
 * Complete test documentation for the project.
 * Organizes all tests by system and suite with requirement traceability.
 */
struct DELVEDEEP_API FTestDocumentation
{
	/** Generation timestamp */
	FDateTime GenerationTime;

	/** Project version */
	FString ProjectVersion;

	/** Array of all test suites */
	TArray<FTestSuiteDocumentation> Suites;

	/** Map of system name to test suites */
	TMap<FString, TArray<FTestSuiteDocumentation>> SuitesBySystem;

	/** Map of requirement ID to tests that verify it */
	TMap<FString, TArray<FTestMetadata>> TestsByRequirement;

	/** Total number of tests across all suites */
	int32 TotalTests;

	/** Total number of requirements covered */
	int32 TotalRequirements;

	FTestDocumentation()
		: TotalTests(0)
		, TotalRequirements(0)
	{
	}
};

/**
 * Test documentation generator.
 * Extracts test metadata from registered automation tests and generates documentation.
 */
class DELVEDEEP_API FTestDocumentationGenerator
{
public:
	/**
	 * Generates complete test documentation by scanning all registered tests.
	 * 
	 * @return Complete test documentation structure
	 */
	static FTestDocumentation GenerateDocumentation();

	/**
	 * Extracts metadata from a single automation test.
	 * 
	 * @param Test The automation test to extract metadata from
	 * @return Test metadata structure
	 */
	static FTestMetadata ExtractTestMetadata(const FAutomationTestBase* Test);

	/**
	 * Parses test name to extract system and suite information.
	 * Expected format: "DelveDeep.System.TestName"
	 * 
	 * @param TestName Full test name
	 * @param OutSystemName Extracted system name
	 * @param OutSuiteName Extracted suite name
	 * @param OutShortName Extracted short test name
	 */
	static void ParseTestName(
		const FString& TestName,
		FString& OutSystemName,
		FString& OutSuiteName,
		FString& OutShortName);

	/**
	 * Extracts requirement IDs from test name or description.
	 * Looks for patterns like "Req_1.1", "Requirements: 1.1, 3.3", etc.
	 * 
	 * @param TestName Test name to search
	 * @param Description Test description to search
	 * @return Array of requirement IDs
	 */
	static TArray<FString> ExtractRequirements(
		const FString& TestName,
		const FString& Description);

	/**
	 * Extracts tags from test flags and name.
	 * 
	 * @param TestFlags Automation test flags
	 * @param TestName Test name (may contain tags in brackets)
	 * @return Array of tags
	 */
	static TArray<FString> ExtractTags(uint32 TestFlags, const FString& TestName);

	/**
	 * Organizes tests into suites by system.
	 * 
	 * @param Tests Array of test metadata
	 * @return Array of test suite documentation
	 */
	static TArray<FTestSuiteDocumentation> OrganizeIntoSuites(
		const TArray<FTestMetadata>& Tests);

	/**
	 * Builds requirement traceability map.
	 * 
	 * @param Tests Array of test metadata
	 * @return Map of requirement ID to tests
	 */
	static TMap<FString, TArray<FTestMetadata>> BuildRequirementTraceability(
		const TArray<FTestMetadata>& Tests);

	/**
	 * Exports test documentation to Markdown format.
	 * 
	 * @param Documentation Test documentation to export
	 * @param OutputPath Path to output file (relative to project directory)
	 * @return True if export succeeded
	 */
	static bool ExportToMarkdown(
		const FTestDocumentation& Documentation,
		const FString& OutputPath);

	/**
	 * Exports test documentation to HTML format.
	 * 
	 * @param Documentation Test documentation to export
	 * @param OutputPath Path to output file (relative to project directory)
	 * @return True if export succeeded
	 */
	static bool ExportToHTML(
		const FTestDocumentation& Documentation,
		const FString& OutputPath);

	/**
	 * Exports requirement traceability matrix to Markdown.
	 * 
	 * @param Documentation Test documentation containing traceability
	 * @param OutputPath Path to output file (relative to project directory)
	 * @return True if export succeeded
	 */
	static bool ExportRequirementTraceability(
		const FTestDocumentation& Documentation,
		const FString& OutputPath);

	/**
	 * Generates documentation and exports to default location.
	 * Exports to Documentation/Testing/TestDocumentation.md
	 * 
	 * @return True if generation and export succeeded
	 */
	static bool GenerateAndExport();

private:
	/**
	 * Generates Markdown table of contents.
	 * 
	 * @param Documentation Test documentation
	 * @return Markdown string for table of contents
	 */
	static FString GenerateMarkdownTOC(const FTestDocumentation& Documentation);

	/**
	 * Generates Markdown for a single test suite.
	 * 
	 * @param Suite Test suite documentation
	 * @return Markdown string for suite
	 */
	static FString GenerateMarkdownSuite(const FTestSuiteDocumentation& Suite);

	/**
	 * Generates Markdown for a single test.
	 * 
	 * @param Test Test metadata
	 * @return Markdown string for test
	 */
	static FString GenerateMarkdownTest(const FTestMetadata& Test);

	/**
	 * Generates HTML header with styles.
	 * 
	 * @return HTML header string
	 */
	static FString GenerateHTMLHeader();

	/**
	 * Generates HTML for a single test suite.
	 * 
	 * @param Suite Test suite documentation
	 * @return HTML string for suite
	 */
	static FString GenerateHTMLSuite(const FTestSuiteDocumentation& Suite);

	/**
	 * Generates HTML footer.
	 * 
	 * @return HTML footer string
	 */
	static FString GenerateHTMLFooter();
};
