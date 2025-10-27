// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestDocumentation.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"

FTestDocumentation FTestDocumentationGenerator::GenerateDocumentation()
{
	FTestDocumentation Documentation;
	Documentation.GenerationTime = FDateTime::Now();
	Documentation.ProjectVersion = TEXT("1.0.0");  // TODO: Get from project settings

	// Get all registered automation tests
	TArray<FAutomationTestBase*> Tests;
	FAutomationTestFramework::Get().GetValidTestNames(Tests);

	// Extract metadata from each test
	TArray<FTestMetadata> AllTests;
	for (FAutomationTestBase* Test : Tests)
	{
		if (Test)
		{
			FTestMetadata Metadata = ExtractTestMetadata(Test);
			AllTests.Add(Metadata);
		}
	}

	// Organize tests into suites
	Documentation.Suites = OrganizeIntoSuites(AllTests);

	// Build requirement traceability
	Documentation.TestsByRequirement = BuildRequirementTraceability(AllTests);

	// Calculate totals
	Documentation.TotalTests = AllTests.Num();
	Documentation.TotalRequirements = Documentation.TestsByRequirement.Num();

	// Organize suites by system
	for (const FTestSuiteDocumentation& Suite : Documentation.Suites)
	{
		if (!Documentation.SuitesBySystem.Contains(Suite.SystemName))
		{
			Documentation.SuitesBySystem.Add(Suite.SystemName, TArray<FTestSuiteDocumentation>());
		}
		Documentation.SuitesBySystem[Suite.SystemName].Add(Suite);
	}

	return Documentation;
}

FTestMetadata FTestDocumentationGenerator::ExtractTestMetadata(const FAutomationTestBase* Test)
{
	FTestMetadata Metadata;

	if (!Test)
	{
		return Metadata;
	}

	// Get test name
	Metadata.TestName = Test->GetTestFullName();

	// Parse test name to extract system and suite
	FString ShortName;
	ParseTestName(Metadata.TestName, Metadata.SystemName, Metadata.TestSuite, ShortName);

	// Get test description (if available)
	// Note: Unreal's automation system doesn't store descriptions by default
	// We'll use the test name as description for now
	Metadata.Description = ShortName;

	// Extract requirements from test name
	Metadata.Requirements = ExtractRequirements(Metadata.TestName, Metadata.Description);

	// Get test flags
	Metadata.TestFlags = Test->GetTestFlags();

	// Extract tags from flags
	Metadata.Tags = ExtractTags(Metadata.TestFlags, Metadata.TestName);

	// Determine if complex test
	// Complex tests typically have "Fixture" in their name or use IMPLEMENT_COMPLEX_AUTOMATION_TEST
	Metadata.bIsComplexTest = Metadata.TestName.Contains(TEXT("Fixture")) || 
	                           Metadata.TestName.Contains(TEXT("Integration"));

	return Metadata;
}

void FTestDocumentationGenerator::ParseTestName(
	const FString& TestName,
	FString& OutSystemName,
	FString& OutSuiteName,
	FString& OutShortName)
{
	// Expected format: "DelveDeep.System.TestName" or "DelveDeep.System.Suite.TestName"
	TArray<FString> Parts;
	TestName.ParseIntoArray(Parts, TEXT("."));

	if (Parts.Num() >= 3)
	{
		// Skip "DelveDeep" prefix
		OutSystemName = Parts[1];
		
		if (Parts.Num() == 3)
		{
			// Format: DelveDeep.System.TestName
			OutSuiteName = OutSystemName;
			OutShortName = Parts[2];
		}
		else
		{
			// Format: DelveDeep.System.Suite.TestName
			OutSuiteName = Parts[2];
			OutShortName = Parts[Parts.Num() - 1];
		}
	}
	else
	{
		// Fallback for non-standard names
		OutSystemName = TEXT("Other");
		OutSuiteName = TEXT("Other");
		OutShortName = TestName;
	}
}

TArray<FString> FTestDocumentationGenerator::ExtractRequirements(
	const FString& TestName,
	const FString& Description)
{
	TArray<FString> Requirements;

	// Look for requirement patterns in test name and description
	// Patterns: "Req_1.1", "Requirements: 1.1, 3.3", "[1.1]", etc.
	
	FString SearchText = TestName + TEXT(" ") + Description;

	// Pattern 1: Req_X.Y
	{
		FRegexPattern Pattern(TEXT("Req_([0-9]+\\.[0-9]+)"));
		FRegexMatcher Matcher(Pattern, SearchText);
		while (Matcher.FindNext())
		{
			FString Requirement = Matcher.GetCaptureGroup(1);
			Requirements.AddUnique(Requirement);
		}
	}

	// Pattern 2: [X.Y]
	{
		FRegexPattern Pattern(TEXT("\\[([0-9]+\\.[0-9]+)\\]"));
		FRegexMatcher Matcher(Pattern, SearchText);
		while (Matcher.FindNext())
		{
			FString Requirement = Matcher.GetCaptureGroup(1);
			Requirements.AddUnique(Requirement);
		}
	}

	// Pattern 3: Requirements: X.Y, Z.W
	{
		FRegexPattern Pattern(TEXT("Requirements?:\\s*([0-9]+\\.[0-9]+(?:,\\s*[0-9]+\\.[0-9]+)*)"));
		FRegexMatcher Matcher(Pattern, SearchText);
		if (Matcher.FindNext())
		{
			FString ReqList = Matcher.GetCaptureGroup(1);
			TArray<FString> ReqParts;
			ReqList.ParseIntoArray(ReqParts, TEXT(","));
			for (FString& Req : ReqParts)
			{
				Req.TrimStartAndEndInline();
				Requirements.AddUnique(Req);
			}
		}
	}

	return Requirements;
}

TArray<FString> FTestDocumentationGenerator::ExtractTags(uint32 TestFlags, const FString& TestName)
{
	TArray<FString> Tags;

	// Extract tags from test flags
	if (TestFlags & EAutomationTestFlags::ProductFilter)
	{
		Tags.Add(TEXT("unit"));
	}

	if (TestFlags & EAutomationTestFlags::PerfFilter)
	{
		Tags.Add(TEXT("performance"));
	}

	if (TestFlags & EAutomationTestFlags::StressFilter)
	{
		Tags.Add(TEXT("stress"));
	}

	// Extract tags from test name
	if (TestName.Contains(TEXT("Integration")))
	{
		Tags.Add(TEXT("integration"));
	}

	if (TestName.Contains(TEXT("Performance")))
	{
		Tags.Add(TEXT("performance"));
	}

	if (TestName.Contains(TEXT("Memory")))
	{
		Tags.Add(TEXT("memory"));
	}

	if (TestName.Contains(TEXT("Validation")))
	{
		Tags.Add(TEXT("validation"));
	}

	return Tags;
}

TArray<FTestSuiteDocumentation> FTestDocumentationGenerator::OrganizeIntoSuites(
	const TArray<FTestMetadata>& Tests)
{
	// Group tests by suite name
	TMap<FString, TArray<FTestMetadata>> TestsBySuite;
	
	for (const FTestMetadata& Test : Tests)
	{
		FString SuiteKey = Test.SystemName + TEXT(".") + Test.TestSuite;
		if (!TestsBySuite.Contains(SuiteKey))
		{
			TestsBySuite.Add(SuiteKey, TArray<FTestMetadata>());
		}
		TestsBySuite[SuiteKey].Add(Test);
	}

	// Create suite documentation
	TArray<FTestSuiteDocumentation> Suites;
	for (const auto& Pair : TestsBySuite)
	{
		FTestSuiteDocumentation Suite;
		Suite.SuiteName = Pair.Key;
		Suite.Tests = Pair.Value;
		Suite.TotalTests = Pair.Value.Num();

		// Extract system name from first test
		if (Pair.Value.Num() > 0)
		{
			Suite.SystemName = Pair.Value[0].SystemName;
			Suite.Description = FString::Printf(TEXT("Tests for %s system"), *Suite.SystemName);
		}

		// Count test types
		for (const FTestMetadata& Test : Pair.Value)
		{
			if (Test.Tags.Contains(TEXT("unit")))
			{
				Suite.UnitTests++;
			}
			if (Test.Tags.Contains(TEXT("integration")))
			{
				Suite.IntegrationTests++;
			}
			if (Test.Tags.Contains(TEXT("performance")))
			{
				Suite.PerformanceTests++;
			}
		}

		Suites.Add(Suite);
	}

	return Suites;
}

TMap<FString, TArray<FTestMetadata>> FTestDocumentationGenerator::BuildRequirementTraceability(
	const TArray<FTestMetadata>& Tests)
{
	TMap<FString, TArray<FTestMetadata>> TraceabilityMap;

	for (const FTestMetadata& Test : Tests)
	{
		for (const FString& Requirement : Test.Requirements)
		{
			if (!TraceabilityMap.Contains(Requirement))
			{
				TraceabilityMap.Add(Requirement, TArray<FTestMetadata>());
			}
			TraceabilityMap[Requirement].Add(Test);
		}
	}

	return TraceabilityMap;
}

bool FTestDocumentationGenerator::ExportToMarkdown(
	const FTestDocumentation& Documentation,
	const FString& OutputPath)
{
	FString Markdown;

	// Header
	Markdown += TEXT("# DelveDeep Test Documentation\n\n");
	Markdown += FString::Printf(TEXT("Generated: %s\n\n"), *Documentation.GenerationTime.ToString());
	Markdown += FString::Printf(TEXT("Project Version: %s\n\n"), *Documentation.ProjectVersion);
	Markdown += FString::Printf(TEXT("Total Tests: %d\n\n"), Documentation.TotalTests);
	Markdown += FString::Printf(TEXT("Total Requirements Covered: %d\n\n"), Documentation.TotalRequirements);

	// Table of contents
	Markdown += GenerateMarkdownTOC(Documentation);

	// Test suites by system
	Markdown += TEXT("## Test Suites\n\n");

	for (const auto& SystemPair : Documentation.SuitesBySystem)
	{
		Markdown += FString::Printf(TEXT("### %s System\n\n"), *SystemPair.Key);

		for (const FTestSuiteDocumentation& Suite : SystemPair.Value)
		{
			Markdown += GenerateMarkdownSuite(Suite);
		}
	}

	// Write to file
	FString FullPath = FPaths::ProjectDir() / OutputPath;
	return FFileHelper::SaveStringToFile(Markdown, *FullPath);
}

FString FTestDocumentationGenerator::GenerateMarkdownTOC(const FTestDocumentation& Documentation)
{
	FString TOC;
	TOC += TEXT("## Table of Contents\n\n");

	for (const auto& SystemPair : Documentation.SuitesBySystem)
	{
		TOC += FString::Printf(TEXT("- [%s System](#%s-system)\n"),
			*SystemPair.Key,
			*SystemPair.Key.ToLower().Replace(TEXT(" "), TEXT("-")));

		for (const FTestSuiteDocumentation& Suite : SystemPair.Value)
		{
			TOC += FString::Printf(TEXT("  - [%s](#%s)\n"),
				*Suite.SuiteName,
				*Suite.SuiteName.ToLower().Replace(TEXT(" "), TEXT("-")).Replace(TEXT("."), TEXT("")));
		}
	}

	TOC += TEXT("\n");
	return TOC;
}

FString FTestDocumentationGenerator::GenerateMarkdownSuite(const FTestSuiteDocumentation& Suite)
{
	FString Markdown;

	Markdown += FString::Printf(TEXT("#### %s\n\n"), *Suite.SuiteName);
	Markdown += FString::Printf(TEXT("%s\n\n"), *Suite.Description);
	Markdown += FString::Printf(TEXT("**Statistics:**\n"));
	Markdown += FString::Printf(TEXT("- Total Tests: %d\n"), Suite.TotalTests);
	Markdown += FString::Printf(TEXT("- Unit Tests: %d\n"), Suite.UnitTests);
	Markdown += FString::Printf(TEXT("- Integration Tests: %d\n"), Suite.IntegrationTests);
	Markdown += FString::Printf(TEXT("- Performance Tests: %d\n\n"), Suite.PerformanceTests);

	Markdown += TEXT("**Tests:**\n\n");

	for (const FTestMetadata& Test : Suite.Tests)
	{
		Markdown += GenerateMarkdownTest(Test);
	}

	Markdown += TEXT("\n");
	return Markdown;
}

FString FTestDocumentationGenerator::GenerateMarkdownTest(const FTestMetadata& Test)
{
	FString Markdown;

	Markdown += FString::Printf(TEXT("- **%s**\n"), *Test.Description);
	Markdown += FString::Printf(TEXT("  - Full Name: `%s`\n"), *Test.TestName);

	if (Test.Requirements.Num() > 0)
	{
		Markdown += TEXT("  - Requirements: ");
		for (int32 i = 0; i < Test.Requirements.Num(); ++i)
		{
			Markdown += Test.Requirements[i];
			if (i < Test.Requirements.Num() - 1)
			{
				Markdown += TEXT(", ");
			}
		}
		Markdown += TEXT("\n");
	}

	if (Test.Tags.Num() > 0)
	{
		Markdown += TEXT("  - Tags: ");
		for (int32 i = 0; i < Test.Tags.Num(); ++i)
		{
			Markdown += FString::Printf(TEXT("`%s`"), *Test.Tags[i]);
			if (i < Test.Tags.Num() - 1)
			{
				Markdown += TEXT(", ");
			}
		}
		Markdown += TEXT("\n");
	}

	return Markdown;
}

bool FTestDocumentationGenerator::ExportToHTML(
	const FTestDocumentation& Documentation,
	const FString& OutputPath)
{
	FString HTML;

	// Header
	HTML += GenerateHTMLHeader();

	// Title
	HTML += TEXT("<h1>DelveDeep Test Documentation</h1>\n");
	HTML += FString::Printf(TEXT("<p><strong>Generated:</strong> %s</p>\n"), *Documentation.GenerationTime.ToString());
	HTML += FString::Printf(TEXT("<p><strong>Project Version:</strong> %s</p>\n"), *Documentation.ProjectVersion);
	HTML += FString::Printf(TEXT("<p><strong>Total Tests:</strong> %d</p>\n"), Documentation.TotalTests);
	HTML += FString::Printf(TEXT("<p><strong>Total Requirements Covered:</strong> %d</p>\n"), Documentation.TotalRequirements);

	// Test suites by system
	HTML += TEXT("<h2>Test Suites</h2>\n");

	for (const auto& SystemPair : Documentation.SuitesBySystem)
	{
		HTML += FString::Printf(TEXT("<h3>%s System</h3>\n"), *SystemPair.Key);

		for (const FTestSuiteDocumentation& Suite : SystemPair.Value)
		{
			HTML += GenerateHTMLSuite(Suite);
		}
	}

	// Footer
	HTML += GenerateHTMLFooter();

	// Write to file
	FString FullPath = FPaths::ProjectDir() / OutputPath;
	return FFileHelper::SaveStringToFile(HTML, *FullPath);
}

FString FTestDocumentationGenerator::GenerateHTMLHeader()
{
	FString Header;
	Header += TEXT("<!DOCTYPE html>\n");
	Header += TEXT("<html>\n");
	Header += TEXT("<head>\n");
	Header += TEXT("<meta charset=\"UTF-8\">\n");
	Header += TEXT("<title>DelveDeep Test Documentation</title>\n");
	Header += TEXT("<style>\n");
	Header += TEXT("body { font-family: Arial, sans-serif; margin: 20px; }\n");
	Header += TEXT("h1 { color: #333; }\n");
	Header += TEXT("h2 { color: #555; border-bottom: 2px solid #ddd; padding-bottom: 5px; }\n");
	Header += TEXT("h3 { color: #777; }\n");
	Header += TEXT("h4 { color: #999; }\n");
	Header += TEXT(".suite { margin: 20px 0; padding: 15px; background-color: #f9f9f9; border-left: 4px solid #4CAF50; }\n");
	Header += TEXT(".test { margin: 10px 0; padding: 10px; background-color: #fff; border: 1px solid #ddd; }\n");
	Header += TEXT(".tag { display: inline-block; padding: 2px 8px; margin: 2px; background-color: #e0e0e0; border-radius: 3px; font-size: 0.9em; }\n");
	Header += TEXT(".requirement { color: #2196F3; font-weight: bold; }\n");
	Header += TEXT("</style>\n");
	Header += TEXT("</head>\n");
	Header += TEXT("<body>\n");
	return Header;
}

FString FTestDocumentationGenerator::GenerateHTMLSuite(const FTestSuiteDocumentation& Suite)
{
	FString HTML;

	HTML += TEXT("<div class=\"suite\">\n");
	HTML += FString::Printf(TEXT("<h4>%s</h4>\n"), *Suite.SuiteName);
	HTML += FString::Printf(TEXT("<p>%s</p>\n"), *Suite.Description);
	HTML += TEXT("<p><strong>Statistics:</strong></p>\n");
	HTML += TEXT("<ul>\n");
	HTML += FString::Printf(TEXT("<li>Total Tests: %d</li>\n"), Suite.TotalTests);
	HTML += FString::Printf(TEXT("<li>Unit Tests: %d</li>\n"), Suite.UnitTests);
	HTML += FString::Printf(TEXT("<li>Integration Tests: %d</li>\n"), Suite.IntegrationTests);
	HTML += FString::Printf(TEXT("<li>Performance Tests: %d</li>\n"), Suite.PerformanceTests);
	HTML += TEXT("</ul>\n");

	HTML += TEXT("<p><strong>Tests:</strong></p>\n");

	for (const FTestMetadata& Test : Suite.Tests)
	{
		HTML += TEXT("<div class=\"test\">\n");
		HTML += FString::Printf(TEXT("<strong>%s</strong><br>\n"), *Test.Description);
		HTML += FString::Printf(TEXT("<code>%s</code><br>\n"), *Test.TestName);

		if (Test.Requirements.Num() > 0)
		{
			HTML += TEXT("<span class=\"requirement\">Requirements: ");
			for (int32 i = 0; i < Test.Requirements.Num(); ++i)
			{
				HTML += Test.Requirements[i];
				if (i < Test.Requirements.Num() - 1)
				{
					HTML += TEXT(", ");
				}
			}
			HTML += TEXT("</span><br>\n");
		}

		if (Test.Tags.Num() > 0)
		{
			for (const FString& Tag : Test.Tags)
			{
				HTML += FString::Printf(TEXT("<span class=\"tag\">%s</span>"), *Tag);
			}
			HTML += TEXT("<br>\n");
		}

		HTML += TEXT("</div>\n");
	}

	HTML += TEXT("</div>\n");
	return HTML;
}

FString FTestDocumentationGenerator::GenerateHTMLFooter()
{
	FString Footer;
	Footer += TEXT("</body>\n");
	Footer += TEXT("</html>\n");
	return Footer;
}

bool FTestDocumentationGenerator::ExportRequirementTraceability(
	const FTestDocumentation& Documentation,
	const FString& OutputPath)
{
	FString Markdown;

	// Header
	Markdown += TEXT("# Requirement Traceability Matrix\n\n");
	Markdown += FString::Printf(TEXT("Generated: %s\n\n"), *Documentation.GenerationTime.ToString());
	Markdown += FString::Printf(TEXT("Total Requirements Covered: %d\n\n"), Documentation.TotalRequirements);

	// Traceability table
	Markdown += TEXT("| Requirement | Test Count | Tests |\n");
	Markdown += TEXT("|-------------|------------|-------|\n");

	// Sort requirements by ID
	TArray<FString> RequirementIDs;
	Documentation.TestsByRequirement.GetKeys(RequirementIDs);
	RequirementIDs.Sort();

	for (const FString& RequirementID : RequirementIDs)
	{
		const TArray<FTestMetadata>& Tests = Documentation.TestsByRequirement[RequirementID];
		
		Markdown += FString::Printf(TEXT("| %s | %d | "), *RequirementID, Tests.Num());

		for (int32 i = 0; i < Tests.Num(); ++i)
		{
			Markdown += FString::Printf(TEXT("`%s`"), *Tests[i].TestName);
			if (i < Tests.Num() - 1)
			{
				Markdown += TEXT("<br>");
			}
		}

		Markdown += TEXT(" |\n");
	}

	// Write to file
	FString FullPath = FPaths::ProjectDir() / OutputPath;
	return FFileHelper::SaveStringToFile(Markdown, *FullPath);
}

bool FTestDocumentationGenerator::GenerateAndExport()
{
	// Generate documentation
	FTestDocumentation Documentation = GenerateDocumentation();

	// Create output directory if it doesn't exist
	FString OutputDir = FPaths::ProjectDir() / TEXT("Documentation/Testing");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*OutputDir))
	{
		PlatformFile.CreateDirectoryTree(*OutputDir);
	}

	// Export to Markdown
	bool bMarkdownSuccess = ExportToMarkdown(Documentation, TEXT("Documentation/Testing/TestDocumentation.md"));
	if (!bMarkdownSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export test documentation to Markdown"));
		return false;
	}

	// Export to HTML
	bool bHTMLSuccess = ExportToHTML(Documentation, TEXT("Documentation/Testing/TestDocumentation.html"));
	if (!bHTMLSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to export test documentation to HTML"));
	}

	// Export requirement traceability
	bool bTraceabilitySuccess = ExportRequirementTraceability(Documentation, TEXT("Documentation/Testing/RequirementTraceability.md"));
	if (!bTraceabilitySuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to export requirement traceability matrix"));
	}

	UE_LOG(LogTemp, Display, TEXT("Test documentation generated successfully"));
	UE_LOG(LogTemp, Display, TEXT("  - Markdown: Documentation/Testing/TestDocumentation.md"));
	UE_LOG(LogTemp, Display, TEXT("  - HTML: Documentation/Testing/TestDocumentation.html"));
	UE_LOG(LogTemp, Display, TEXT("  - Traceability: Documentation/Testing/RequirementTraceability.md"));

	return true;
}
