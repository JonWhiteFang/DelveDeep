// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestReport.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

FDelveDeepTestReport FTestReportGenerator::GenerateReport(const FString& ReportPath)
{
	FDelveDeepTestReport Report;
	Report.GenerationTime = FDateTime::Now();

	// Check if report file exists
	if (!FPaths::FileExists(ReportPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Test report file not found: %s"), *ReportPath);
		return Report;
	}

	// Load report file
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *ReportPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load test report file: %s"), *ReportPath);
		return Report;
	}

	// Parse file content line by line
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);

	TArray<FDelveDeepTestResult> Results;
	for (const FString& Line : Lines)
	{
		FDelveDeepTestResult Result;
		if (ParseTestResultLine(Line, Result))
		{
			Results.Add(Result);
		}
	}

	// Generate report from parsed results
	return GenerateReportFromResults(Results);
}

FDelveDeepTestReport FTestReportGenerator::GenerateReportFromResults(
	const TArray<FDelveDeepTestResult>& Results,
	const FString& BuildVersion)
{
	FDelveDeepTestReport Report;
	Report.GenerationTime = FDateTime::Now();
	Report.BuildVersion = BuildVersion.IsEmpty() ? TEXT("Unknown") : BuildVersion;
	Report.Results = Results;
	Report.TotalTests = Results.Num();

	// Calculate statistics
	for (const FDelveDeepTestResult& Result : Results)
	{
		// Count passed/failed tests
		if (Result.bPassed)
		{
			Report.PassedTests++;
		}
		else
		{
			Report.FailedTests++;
		}

		// Accumulate execution time
		Report.TotalExecutionTime += Result.ExecutionTime;

		// Accumulate memory statistics
		Report.TotalMemoryAllocated += Result.MemoryAllocated;
		Report.TotalAllocations += Result.AllocationCount;

		// Track tests by suite
		FString Suite = Result.TestSuite;
		if (Suite.IsEmpty())
		{
			Suite = ExtractTestSuite(Result.TestPath);
		}

		if (!Suite.IsEmpty())
		{
			int32* TestCount = Report.TestsBySuite.Find(Suite);
			if (TestCount)
			{
				(*TestCount)++;
			}
			else
			{
				Report.TestsBySuite.Add(Suite, 1);
			}

			Report.ExecutionTimeBySuite.FindOrAdd(Suite) += Result.ExecutionTime;
		}
	}

	return Report;
}

bool FTestReportGenerator::ExportToMarkdown(const FDelveDeepTestReport& Report, const FString& OutputPath)
{
	FString MarkdownContent;

	// Title
	MarkdownContent += TEXT("# DelveDeep Test Report\n\n");

	// Generation info
	MarkdownContent += FString::Printf(TEXT("**Generated:** %s\n\n"),
		*Report.GenerationTime.ToString());
	MarkdownContent += FString::Printf(TEXT("**Build Version:** %s\n\n"),
		*Report.BuildVersion);

	// Summary section
	MarkdownContent += GenerateMarkdownSummary(Report);

	// Suite breakdown
	MarkdownContent += GenerateMarkdownSuiteBreakdown(Report);

	// Test results table
	MarkdownContent += GenerateMarkdownResultsTable(Report);

	// Failed tests details
	if (Report.FailedTests > 0)
	{
		MarkdownContent += TEXT("\n## Failed Tests Details\n\n");
		for (const FDelveDeepTestResult& Result : Report.Results)
		{
			if (!Result.bPassed)
			{
				MarkdownContent += FString::Printf(TEXT("### %s\n\n"), *Result.TestName);
				MarkdownContent += FString::Printf(TEXT("**Path:** `%s`\n\n"), *Result.TestPath);
				MarkdownContent += FString::Printf(TEXT("**Execution Time:** %s\n\n"),
					*FormatExecutionTime(Result.ExecutionTime));

				if (Result.Errors.Num() > 0)
				{
					MarkdownContent += TEXT("**Errors:**\n\n");
					for (const FString& Error : Result.Errors)
					{
						MarkdownContent += FString::Printf(TEXT("- %s\n"), *Error);
					}
					MarkdownContent += TEXT("\n");
				}

				if (Result.Warnings.Num() > 0)
				{
					MarkdownContent += TEXT("**Warnings:**\n\n");
					for (const FString& Warning : Result.Warnings)
					{
						MarkdownContent += FString::Printf(TEXT("- %s\n"), *Warning);
					}
					MarkdownContent += TEXT("\n");
				}
			}
		}
	}

	// Write to file
	if (FFileHelper::SaveStringToFile(MarkdownContent, *OutputPath))
	{
		UE_LOG(LogTemp, Display, TEXT("Test report exported to Markdown: %s"), *OutputPath);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export test report to Markdown: %s"), *OutputPath);
		return false;
	}
}

bool FTestReportGenerator::ExportToHTML(const FDelveDeepTestReport& Report, const FString& OutputPath)
{
	FString HTMLContent;

	// HTML header with CSS
	HTMLContent += GenerateHTMLHeader();

	// Title
	HTMLContent += TEXT("<h1>DelveDeep Test Report</h1>\n");

	// Generation info
	HTMLContent += TEXT("<div class=\"info\">\n");
	HTMLContent += FString::Printf(TEXT("<p><strong>Generated:</strong> %s</p>\n"),
		*Report.GenerationTime.ToString());
	HTMLContent += FString::Printf(TEXT("<p><strong>Build Version:</strong> %s</p>\n"),
		*Report.BuildVersion);
	HTMLContent += TEXT("</div>\n\n");

	// Summary section
	HTMLContent += TEXT("<div class=\"summary\">\n");
	HTMLContent += TEXT("<h2>Summary</h2>\n");
	HTMLContent += TEXT("<table>\n");
	HTMLContent += TEXT("<tr><th>Metric</th><th>Value</th></tr>\n");
	HTMLContent += FString::Printf(TEXT("<tr><td>Total Tests</td><td>%d</td></tr>\n"), Report.TotalTests);
	HTMLContent += FString::Printf(TEXT("<tr><td>Passed</td><td class=\"passed\">%d</td></tr>\n"), Report.PassedTests);
	HTMLContent += FString::Printf(TEXT("<tr><td>Failed</td><td class=\"failed\">%d</td></tr>\n"), Report.FailedTests);
	HTMLContent += FString::Printf(TEXT("<tr><td>Pass Rate</td><td>%.1f%%</td></tr>\n"), Report.GetPassRate());
	HTMLContent += FString::Printf(TEXT("<tr><td>Total Execution Time</td><td>%s</td></tr>\n"),
		*FormatExecutionTime(Report.TotalExecutionTime));
	HTMLContent += FString::Printf(TEXT("<tr><td>Average Execution Time</td><td>%s</td></tr>\n"),
		*FormatExecutionTime(Report.GetAverageExecutionTime()));
	HTMLContent += TEXT("</table>\n");
	HTMLContent += TEXT("</div>\n\n");

	// Suite breakdown
	if (Report.TestsBySuite.Num() > 0)
	{
		HTMLContent += TEXT("<div class=\"suite-breakdown\">\n");
		HTMLContent += TEXT("<h2>Test Suites</h2>\n");
		HTMLContent += TEXT("<table>\n");
		HTMLContent += TEXT("<tr><th>Suite</th><th>Tests</th><th>Execution Time</th></tr>\n");

		for (const auto& Pair : Report.TestsBySuite)
		{
			const FString& Suite = Pair.Key;
			int32 TestCount = Pair.Value;
			const float* SuiteTime = Report.ExecutionTimeBySuite.Find(Suite);
			float ExecutionTime = SuiteTime ? *SuiteTime : 0.0f;

			HTMLContent += FString::Printf(TEXT("<tr><td>%s</td><td>%d</td><td>%s</td></tr>\n"),
				*Suite, TestCount, *FormatExecutionTime(ExecutionTime));
		}

		HTMLContent += TEXT("</table>\n");
		HTMLContent += TEXT("</div>\n\n");
	}

	// Test results table
	HTMLContent += TEXT("<div class=\"test-results\">\n");
	HTMLContent += TEXT("<h2>Test Results</h2>\n");
	HTMLContent += TEXT("<table>\n");
	HTMLContent += TEXT("<tr><th>Test Name</th><th>Suite</th><th>Status</th><th>Execution Time</th></tr>\n");

	for (const FDelveDeepTestResult& Result : Report.Results)
	{
		FString StatusClass = Result.bPassed ? TEXT("passed") : TEXT("failed");
		FString StatusText = Result.bPassed ? TEXT("✓ PASSED") : TEXT("✗ FAILED");
		FString Suite = Result.TestSuite.IsEmpty() ? ExtractTestSuite(Result.TestPath) : Result.TestSuite;

		HTMLContent += FString::Printf(TEXT("<tr><td>%s</td><td>%s</td><td class=\"%s\">%s</td><td>%s</td></tr>\n"),
			*Result.TestName, *Suite, *StatusClass, *StatusText, *FormatExecutionTime(Result.ExecutionTime));
	}

	HTMLContent += TEXT("</table>\n");
	HTMLContent += TEXT("</div>\n\n");

	// Failed tests details
	if (Report.FailedTests > 0)
	{
		HTMLContent += TEXT("<div class=\"failed-details\">\n");
		HTMLContent += TEXT("<h2>Failed Tests Details</h2>\n");

		for (const FDelveDeepTestResult& Result : Report.Results)
		{
			if (!Result.bPassed)
			{
				HTMLContent += TEXT("<div class=\"failed-test\">\n");
				HTMLContent += FString::Printf(TEXT("<h3>%s</h3>\n"), *Result.TestName);
				HTMLContent += FString::Printf(TEXT("<p><strong>Path:</strong> <code>%s</code></p>\n"), *Result.TestPath);
				HTMLContent += FString::Printf(TEXT("<p><strong>Execution Time:</strong> %s</p>\n"),
					*FormatExecutionTime(Result.ExecutionTime));

				if (Result.Errors.Num() > 0)
				{
					HTMLContent += TEXT("<p><strong>Errors:</strong></p>\n<ul>\n");
					for (const FString& Error : Result.Errors)
					{
						HTMLContent += FString::Printf(TEXT("<li>%s</li>\n"), *Error);
					}
					HTMLContent += TEXT("</ul>\n");
				}

				if (Result.Warnings.Num() > 0)
				{
					HTMLContent += TEXT("<p><strong>Warnings:</strong></p>\n<ul>\n");
					for (const FString& Warning : Result.Warnings)
					{
						HTMLContent += FString::Printf(TEXT("<li>%s</li>\n"), *Warning);
					}
					HTMLContent += TEXT("</ul>\n");
				}

				HTMLContent += TEXT("</div>\n");
			}
		}

		HTMLContent += TEXT("</div>\n");
	}

	// HTML footer
	HTMLContent += GenerateHTMLFooter();

	// Write to file
	if (FFileHelper::SaveStringToFile(HTMLContent, *OutputPath))
	{
		UE_LOG(LogTemp, Display, TEXT("Test report exported to HTML: %s"), *OutputPath);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export test report to HTML: %s"), *OutputPath);
		return false;
	}
}

bool FTestReportGenerator::ExportToJUnit(const FDelveDeepTestReport& Report, const FString& OutputPath)
{
	FString XMLContent;

	// XML header
	XMLContent += TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	// Testsuites element
	XMLContent += FString::Printf(TEXT("<testsuites tests=\"%d\" failures=\"%d\" time=\"%.3f\">\n"),
		Report.TotalTests, Report.FailedTests, Report.TotalExecutionTime);

	// Group tests by suite
	TMap<FString, TArray<FDelveDeepTestResult>> TestsBySuite;
	for (const FDelveDeepTestResult& Result : Report.Results)
	{
		FString Suite = Result.TestSuite.IsEmpty() ? ExtractTestSuite(Result.TestPath) : Result.TestSuite;
		if (Suite.IsEmpty())
		{
			Suite = TEXT("Default");
		}

		TArray<FDelveDeepTestResult>* SuiteTests = TestsBySuite.Find(Suite);
		if (SuiteTests)
		{
			SuiteTests->Add(Result);
		}
		else
		{
			TArray<FDelveDeepTestResult> NewSuiteTests;
			NewSuiteTests.Add(Result);
			TestsBySuite.Add(Suite, NewSuiteTests);
		}
	}

	// Generate testsuite elements
	for (const auto& Pair : TestsBySuite)
	{
		const FString& Suite = Pair.Key;
		const TArray<FDelveDeepTestResult>& SuiteTests = Pair.Value;

		int32 SuiteFailures = 0;
		float SuiteTime = 0.0f;
		for (const FDelveDeepTestResult& Result : SuiteTests)
		{
			if (!Result.bPassed)
			{
				SuiteFailures++;
			}
			SuiteTime += Result.ExecutionTime;
		}

		XMLContent += FString::Printf(TEXT("  <testsuite name=\"%s\" tests=\"%d\" failures=\"%d\" time=\"%.3f\">\n"),
			*Suite, SuiteTests.Num(), SuiteFailures, SuiteTime);

		// Generate testcase elements
		for (const FDelveDeepTestResult& Result : SuiteTests)
		{
			XMLContent += FString::Printf(TEXT("    <testcase name=\"%s\" classname=\"%s\" time=\"%.3f\">\n"),
				*Result.TestName, *Result.TestPath, Result.ExecutionTime);

			if (!Result.bPassed)
			{
				XMLContent += TEXT("      <failure message=\"Test failed\">\n");
				for (const FString& Error : Result.Errors)
				{
					XMLContent += FString::Printf(TEXT("        %s\n"), *Error);
				}
				XMLContent += TEXT("      </failure>\n");
			}

			XMLContent += TEXT("    </testcase>\n");
		}

		XMLContent += TEXT("  </testsuite>\n");
	}

	XMLContent += TEXT("</testsuites>\n");

	// Write to file
	if (FFileHelper::SaveStringToFile(XMLContent, *OutputPath))
	{
		UE_LOG(LogTemp, Display, TEXT("Test report exported to JUnit XML: %s"), *OutputPath);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export test report to JUnit XML: %s"), *OutputPath);
		return false;
	}
}

bool FTestReportGenerator::ParseTestResultLine(const FString& Line, FDelveDeepTestResult& OutResult)
{
	// Parse Unreal's automation test output format
	// Example: "LogAutomationTest: Display: Test Completed. Result={Passed} Name={DelveDeep.Configuration.AssetCaching} Path={DelveDeep.Configuration.AssetCaching} Time={0.123}"
	
	if (!Line.Contains(TEXT("LogAutomationTest")) || !Line.Contains(TEXT("Test Completed")))
	{
		return false;
	}

	// Extract test result (Passed/Failed)
	if (Line.Contains(TEXT("Result={Passed}")))
	{
		OutResult.bPassed = true;
	}
	else if (Line.Contains(TEXT("Result={Failed}")))
	{
		OutResult.bPassed = false;
	}
	else
	{
		return false;
	}

	// Extract test name
	int32 NameStart = Line.Find(TEXT("Name={"));
	if (NameStart != INDEX_NONE)
	{
		NameStart += 6; // Length of "Name={"
		int32 NameEnd = Line.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, NameStart);
		if (NameEnd != INDEX_NONE)
		{
			OutResult.TestName = Line.Mid(NameStart, NameEnd - NameStart);
		}
	}

	// Extract test path
	int32 PathStart = Line.Find(TEXT("Path={"));
	if (PathStart != INDEX_NONE)
	{
		PathStart += 6; // Length of "Path={"
		int32 PathEnd = Line.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, PathStart);
		if (PathEnd != INDEX_NONE)
		{
			OutResult.TestPath = Line.Mid(PathStart, PathEnd - PathStart);
		}
	}

	// Extract execution time
	int32 TimeStart = Line.Find(TEXT("Time={"));
	if (TimeStart != INDEX_NONE)
	{
		TimeStart += 6; // Length of "Time={"
		int32 TimeEnd = Line.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromStart, TimeStart);
		if (TimeEnd != INDEX_NONE)
		{
			FString TimeStr = Line.Mid(TimeStart, TimeEnd - TimeStart);
			OutResult.ExecutionTime = FCString::Atof(*TimeStr);
		}
	}

	// Extract test suite from path
	OutResult.TestSuite = ExtractTestSuite(OutResult.TestPath);
	OutResult.ExecutionTimestamp = FDateTime::Now();

	return true;
}

FString FTestReportGenerator::ExtractTestSuite(const FString& TestPath)
{
	// Extract suite from path like "DelveDeep.Configuration.AssetCaching"
	// Returns "Configuration"
	
	TArray<FString> PathParts;
	TestPath.ParseIntoArray(PathParts, TEXT("."));

	if (PathParts.Num() >= 2)
	{
		return PathParts[1]; // Return second part (suite name)
	}

	return TEXT("");
}

FString FTestReportGenerator::FormatExecutionTime(float TimeSeconds)
{
	if (TimeSeconds < 0.001f)
	{
		return FString::Printf(TEXT("%.0f µs"), TimeSeconds * 1000000.0f);
	}
	else if (TimeSeconds < 1.0f)
	{
		return FString::Printf(TEXT("%.2f ms"), TimeSeconds * 1000.0f);
	}
	else
	{
		return FString::Printf(TEXT("%.3f s"), TimeSeconds);
	}
}

FString FTestReportGenerator::FormatMemorySize(uint64 Bytes)
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%llu B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f KB"), Bytes / 1024.0);
	}
	else if (Bytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f MB"), Bytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.2f GB"), Bytes / (1024.0 * 1024.0 * 1024.0));
	}
}

FString FTestReportGenerator::GenerateHTMLHeader()
{
	FString Header;
	Header += TEXT("<!DOCTYPE html>\n");
	Header += TEXT("<html lang=\"en\">\n");
	Header += TEXT("<head>\n");
	Header += TEXT("  <meta charset=\"UTF-8\">\n");
	Header += TEXT("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
	Header += TEXT("  <title>DelveDeep Test Report</title>\n");
	Header += TEXT("  <style>\n");
	Header += TEXT("    body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }\n");
	Header += TEXT("    h1 { color: #333; }\n");
	Header += TEXT("    h2 { color: #555; margin-top: 30px; }\n");
	Header += TEXT("    h3 { color: #666; }\n");
	Header += TEXT("    .info { background-color: #e8f4f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; }\n");
	Header += TEXT("    .summary { background-color: #fff; padding: 20px; border-radius: 5px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	Header += TEXT("    .suite-breakdown { background-color: #fff; padding: 20px; border-radius: 5px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	Header += TEXT("    .test-results { background-color: #fff; padding: 20px; border-radius: 5px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	Header += TEXT("    .failed-details { background-color: #fff; padding: 20px; border-radius: 5px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	Header += TEXT("    .failed-test { background-color: #fff5f5; padding: 15px; border-left: 4px solid #e74c3c; margin-bottom: 15px; }\n");
	Header += TEXT("    table { width: 100%; border-collapse: collapse; margin-top: 10px; }\n");
	Header += TEXT("    th { background-color: #3498db; color: white; padding: 12px; text-align: left; }\n");
	Header += TEXT("    td { padding: 10px; border-bottom: 1px solid #ddd; }\n");
	Header += TEXT("    tr:hover { background-color: #f5f5f5; }\n");
	Header += TEXT("    .passed { color: #27ae60; font-weight: bold; }\n");
	Header += TEXT("    .failed { color: #e74c3c; font-weight: bold; }\n");
	Header += TEXT("    code { background-color: #f4f4f4; padding: 2px 6px; border-radius: 3px; font-family: monospace; }\n");
	Header += TEXT("    ul { margin: 10px 0; }\n");
	Header += TEXT("    li { margin: 5px 0; }\n");
	Header += TEXT("  </style>\n");
	Header += TEXT("</head>\n");
	Header += TEXT("<body>\n");
	return Header;
}

FString FTestReportGenerator::GenerateHTMLFooter()
{
	FString Footer;
	Footer += TEXT("</body>\n");
	Footer += TEXT("</html>\n");
	return Footer;
}

FString FTestReportGenerator::GenerateMarkdownSummary(const FDelveDeepTestReport& Report)
{
	FString Summary;
	Summary += TEXT("## Summary\n\n");
	Summary += TEXT("| Metric | Value |\n");
	Summary += TEXT("|--------|-------|\n");
	Summary += FString::Printf(TEXT("| Total Tests | %d |\n"), Report.TotalTests);
	Summary += FString::Printf(TEXT("| Passed | %d |\n"), Report.PassedTests);
	Summary += FString::Printf(TEXT("| Failed | %d |\n"), Report.FailedTests);
	Summary += FString::Printf(TEXT("| Pass Rate | %.1f%% |\n"), Report.GetPassRate());
	Summary += FString::Printf(TEXT("| Total Execution Time | %s |\n"),
		*FormatExecutionTime(Report.TotalExecutionTime));
	Summary += FString::Printf(TEXT("| Average Execution Time | %s |\n"),
		*FormatExecutionTime(Report.GetAverageExecutionTime()));
	Summary += TEXT("\n");
	return Summary;
}

FString FTestReportGenerator::GenerateMarkdownResultsTable(const FDelveDeepTestReport& Report)
{
	FString Table;
	Table += TEXT("## Test Results\n\n");
	Table += TEXT("| Test Name | Suite | Status | Execution Time |\n");
	Table += TEXT("|-----------|-------|--------|----------------|\n");

	for (const FDelveDeepTestResult& Result : Report.Results)
	{
		FString StatusText = Result.bPassed ? TEXT("✓ PASSED") : TEXT("✗ FAILED");
		FString Suite = Result.TestSuite.IsEmpty() ? ExtractTestSuite(Result.TestPath) : Result.TestSuite;

		Table += FString::Printf(TEXT("| %s | %s | %s | %s |\n"),
			*Result.TestName, *Suite, *StatusText, *FormatExecutionTime(Result.ExecutionTime));
	}

	Table += TEXT("\n");
	return Table;
}

FString FTestReportGenerator::GenerateMarkdownSuiteBreakdown(const FDelveDeepTestReport& Report)
{
	if (Report.TestsBySuite.Num() == 0)
	{
		return TEXT("");
	}

	FString Breakdown;
	Breakdown += TEXT("## Test Suites\n\n");
	Breakdown += TEXT("| Suite | Tests | Execution Time |\n");
	Breakdown += TEXT("|-------|-------|----------------|\n");

	for (const auto& Pair : Report.TestsBySuite)
	{
		const FString& Suite = Pair.Key;
		int32 TestCount = Pair.Value;
		float* SuiteTime = Report.ExecutionTimeBySuite.Find(Suite);
		float ExecutionTime = SuiteTime ? *SuiteTime : 0.0f;

		Breakdown += FString::Printf(TEXT("| %s | %d | %s |\n"),
			*Suite, TestCount, *FormatExecutionTime(ExecutionTime));
	}

	Breakdown += TEXT("\n");
	return Breakdown;
}
