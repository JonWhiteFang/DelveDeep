// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepRegressionDetector.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepRegression, Log, All);

UDelveDeepRegressionDetector::UDelveDeepRegressionDetector()
{
	// Set default thresholds
	Thresholds.PerformanceThreshold = 10.0f;
	Thresholds.MemoryThreshold = 15.0f;
	Thresholds.FlakyTestThreshold = 0.2f;
	Thresholds.MinExecutionTime = 1.0f;
}

bool UDelveDeepRegressionDetector::CompareTestResults(
	const FDelveDeepTestReport& Baseline,
	const FDelveDeepTestReport& Current)
{
	UE_LOG(LogDelveDeepRegression, Display, TEXT("Comparing test results for regression detection..."));

	Regressions.Empty();

	// Detect different types of regressions
	DetectPerformanceRegressions(Baseline, Current);
	DetectMemoryRegressions(Baseline, Current);
	DetectTestFailures(Baseline, Current);
	DetectFlakyTests(Baseline, Current);

	UE_LOG(LogDelveDeepRegression, Display, 
		TEXT("Regression detection complete. Found %d regressions."), Regressions.Num());

	return Regressions.Num() > 0;
}

TArray<FRegressionReport> UDelveDeepRegressionDetector::GetRegressionsByType(ERegressionType Type) const
{
	TArray<FRegressionReport> FilteredRegressions;

	for (const FRegressionReport& Regression : Regressions)
	{
		if (Regression.RegressionType == Type)
		{
			FilteredRegressions.Add(Regression);
		}
	}

	return FilteredRegressions;
}

TArray<FRegressionReport> UDelveDeepRegressionDetector::GetCriticalRegressions() const
{
	TArray<FRegressionReport> CriticalRegressions;

	for (const FRegressionReport& Regression : Regressions)
	{
		if (Regression.PercentageChange > 50.0f)
		{
			CriticalRegressions.Add(Regression);
		}
	}

	return CriticalRegressions;
}

void UDelveDeepRegressionDetector::SetThresholds(const FRegressionThresholds& NewThresholds)
{
	Thresholds = NewThresholds;
	UE_LOG(LogDelveDeepRegression, Display, TEXT("Regression thresholds updated"));
}

FString UDelveDeepRegressionDetector::GenerateRegressionReport() const
{
	return GenerateMarkdownReport();
}

bool UDelveDeepRegressionDetector::ExportRegressionReport(const FString& OutputPath) const
{
	FString Report = GenerateHTMLReport();

	if (FFileHelper::SaveStringToFile(Report, *OutputPath))
	{
		UE_LOG(LogDelveDeepRegression, Display, 
			TEXT("Regression report exported to: %s"), *OutputPath);
		return true;
	}

	UE_LOG(LogDelveDeepRegression, Error, 
		TEXT("Failed to export regression report to: %s"), *OutputPath);
	return false;
}

bool UDelveDeepRegressionDetector::LoadBaselineReport(const FString& FilePath)
{
	// In a real implementation, this would deserialize from JSON
	UE_LOG(LogDelveDeepRegression, Display, 
		TEXT("Loading baseline report from: %s"), *FilePath);

	// For now, just log
	return true;
}

bool UDelveDeepRegressionDetector::SaveBaselineReport(
	const FDelveDeepTestReport& Report,
	const FString& FilePath)
{
	// In a real implementation, this would serialize to JSON
	UE_LOG(LogDelveDeepRegression, Display, 
		TEXT("Saving baseline report to: %s"), *FilePath);

	BaselineReport = Report;
	return true;
}

void UDelveDeepRegressionDetector::DetectPerformanceRegressions(
	const FDelveDeepTestReport& Baseline,
	const FDelveDeepTestReport& Current)
{
	for (const FDelveDeepTestResult& CurrentResult : Current.Results)
	{
		const FDelveDeepTestResult* BaselineResult = FindTestResult(Baseline, CurrentResult.TestName);

		if (!BaselineResult)
		{
			continue;  // New test, not a regression
		}

		// Skip tests that are too fast to measure reliably
		if (BaselineResult->ExecutionTime < Thresholds.MinExecutionTime)
		{
			continue;
		}

		float PercentageChange = CalculatePercentageChange(
			BaselineResult->ExecutionTime,
			CurrentResult.ExecutionTime);

		if (PercentageChange > Thresholds.PerformanceThreshold)
		{
			FRegressionReport Regression;
			Regression.TestName = CurrentResult.TestName;
			Regression.RegressionType = ERegressionType::Performance;
			Regression.Description = FString::Printf(
				TEXT("Execution time increased from %.2fms to %.2fms"),
				BaselineResult->ExecutionTime,
				CurrentResult.ExecutionTime);
			Regression.BaselineValue = BaselineResult->ExecutionTime;
			Regression.CurrentValue = CurrentResult.ExecutionTime;
			Regression.PercentageChange = PercentageChange;
			Regression.DetectionTime = FDateTime::Now();

			Regressions.Add(Regression);

			UE_LOG(LogDelveDeepRegression, Warning,
				TEXT("Performance regression detected: %s (%.1f%% slower)"),
				*CurrentResult.TestName, PercentageChange);
		}
	}
}

void UDelveDeepRegressionDetector::DetectMemoryRegressions(
	const FDelveDeepTestReport& Baseline,
	const FDelveDeepTestReport& Current)
{
	for (const FDelveDeepTestResult& CurrentResult : Current.Results)
	{
		const FDelveDeepTestResult* BaselineResult = FindTestResult(Baseline, CurrentResult.TestName);

		if (!BaselineResult)
		{
			continue;
		}

		// Skip if no memory data
		if (BaselineResult->MemoryAllocated == 0)
		{
			continue;
		}

		float PercentageChange = CalculatePercentageChange(
			static_cast<float>(BaselineResult->MemoryAllocated),
			static_cast<float>(CurrentResult.MemoryAllocated));

		if (PercentageChange > Thresholds.MemoryThreshold)
		{
			FRegressionReport Regression;
			Regression.TestName = CurrentResult.TestName;
			Regression.RegressionType = ERegressionType::Memory;
			Regression.Description = FString::Printf(
				TEXT("Memory usage increased from %llu bytes to %llu bytes"),
				BaselineResult->MemoryAllocated,
				CurrentResult.MemoryAllocated);
			Regression.BaselineValue = static_cast<float>(BaselineResult->MemoryAllocated);
			Regression.CurrentValue = static_cast<float>(CurrentResult.MemoryAllocated);
			Regression.PercentageChange = PercentageChange;
			Regression.DetectionTime = FDateTime::Now();

			Regressions.Add(Regression);

			UE_LOG(LogDelveDeepRegression, Warning,
				TEXT("Memory regression detected: %s (%.1f%% more memory)"),
				*CurrentResult.TestName, PercentageChange);
		}
	}
}

void UDelveDeepRegressionDetector::DetectTestFailures(
	const FDelveDeepTestReport& Baseline,
	const FDelveDeepTestReport& Current)
{
	for (const FDelveDeepTestResult& CurrentResult : Current.Results)
	{
		const FDelveDeepTestResult* BaselineResult = FindTestResult(Baseline, CurrentResult.TestName);

		if (!BaselineResult)
		{
			continue;
		}

		// Test was passing in baseline but failing now
		if (BaselineResult->bPassed && !CurrentResult.bPassed)
		{
			FRegressionReport Regression;
			Regression.TestName = CurrentResult.TestName;
			Regression.RegressionType = ERegressionType::Failure;
			Regression.Description = TEXT("Test was passing in baseline but is now failing");
			Regression.BaselineValue = 1.0f;  // Passing
			Regression.CurrentValue = 0.0f;   // Failing
			Regression.PercentageChange = 100.0f;
			Regression.DetectionTime = FDateTime::Now();

			Regressions.Add(Regression);

			UE_LOG(LogDelveDeepRegression, Error,
				TEXT("Test failure regression detected: %s"),
				*CurrentResult.TestName);
		}
	}
}

void UDelveDeepRegressionDetector::DetectFlakyTests(
	const FDelveDeepTestReport& Baseline,
	const FDelveDeepTestReport& Current)
{
	// Flaky test detection requires multiple runs
	// For now, we'll detect tests that have inconsistent results

	for (const FDelveDeepTestResult& CurrentResult : Current.Results)
	{
		const FDelveDeepTestResult* BaselineResult = FindTestResult(Baseline, CurrentResult.TestName);

		if (!BaselineResult)
		{
			continue;
		}

		// Check for large variance in execution time (potential flakiness indicator)
		if (BaselineResult->ExecutionTime > 0.0f && CurrentResult.ExecutionTime > 0.0f)
		{
			float Variance = FMath::Abs(CurrentResult.ExecutionTime - BaselineResult->ExecutionTime) / 
			                 BaselineResult->ExecutionTime;

			if (Variance > 2.0f)  // More than 200% variance
			{
				FRegressionReport Regression;
				Regression.TestName = CurrentResult.TestName;
				Regression.RegressionType = ERegressionType::Flaky;
				Regression.Description = FString::Printf(
					TEXT("Test shows high variance in execution time (%.2fms vs %.2fms)"),
					BaselineResult->ExecutionTime,
					CurrentResult.ExecutionTime);
				Regression.BaselineValue = BaselineResult->ExecutionTime;
				Regression.CurrentValue = CurrentResult.ExecutionTime;
				Regression.PercentageChange = Variance * 100.0f;
				Regression.DetectionTime = FDateTime::Now();

				Regressions.Add(Regression);

				UE_LOG(LogDelveDeepRegression, Warning,
					TEXT("Potentially flaky test detected: %s"),
					*CurrentResult.TestName);
			}
		}
	}
}

const FDelveDeepTestResult* UDelveDeepRegressionDetector::FindTestResult(
	const FDelveDeepTestReport& Report,
	const FString& TestName) const
{
	for (const FDelveDeepTestResult& Result : Report.Results)
	{
		if (Result.TestName == TestName)
		{
			return &Result;
		}
	}

	return nullptr;
}

float UDelveDeepRegressionDetector::CalculatePercentageChange(float Baseline, float Current) const
{
	if (Baseline == 0.0f)
	{
		return 0.0f;
	}

	return ((Current - Baseline) / Baseline) * 100.0f;
}

FString UDelveDeepRegressionDetector::GenerateHTMLReport() const
{
	FString HTML = TEXT("<!DOCTYPE html>\n<html>\n<head>\n");
	HTML += TEXT("<title>DelveDeep Regression Report</title>\n");
	HTML += TEXT("<style>\n");
	HTML += TEXT("body { font-family: Arial, sans-serif; margin: 20px; }\n");
	HTML += TEXT("h1 { color: #333; }\n");
	HTML += TEXT("table { border-collapse: collapse; width: 100%; margin-top: 20px; }\n");
	HTML += TEXT("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
	HTML += TEXT("th { background-color: #f44336; color: white; }\n");
	HTML += TEXT("tr:nth-child(even) { background-color: #f2f2f2; }\n");
	HTML += TEXT(".critical { background-color: #ffcdd2; }\n");
	HTML += TEXT(".high { background-color: #ffecb3; }\n");
	HTML += TEXT(".medium { background-color: #fff9c4; }\n");
	HTML += TEXT(".low { background-color: #f1f8e9; }\n");
	HTML += TEXT(".summary { background-color: #ffebee; padding: 15px; margin: 20px 0; border-left: 4px solid #f44336; }\n");
	HTML += TEXT("</style>\n</head>\n<body>\n");

	// Header
	HTML += TEXT("<h1>DelveDeep Regression Report</h1>\n");
	HTML += FString::Printf(TEXT("<p>Generated: %s</p>\n"), *FDateTime::Now().ToString());

	// Summary
	HTML += TEXT("<div class='summary'>\n");
	HTML += FString::Printf(TEXT("<h2>Total Regressions: %d</h2>\n"), Regressions.Num());
	
	int32 CriticalCount = GetCriticalRegressions().Num();
	int32 PerformanceCount = GetRegressionsByType(ERegressionType::Performance).Num();
	int32 MemoryCount = GetRegressionsByType(ERegressionType::Memory).Num();
	int32 FailureCount = GetRegressionsByType(ERegressionType::Failure).Num();
	int32 FlakyCount = GetRegressionsByType(ERegressionType::Flaky).Num();

	HTML += FString::Printf(TEXT("<p>Critical: %d</p>\n"), CriticalCount);
	HTML += FString::Printf(TEXT("<p>Performance: %d</p>\n"), PerformanceCount);
	HTML += FString::Printf(TEXT("<p>Memory: %d</p>\n"), MemoryCount);
	HTML += FString::Printf(TEXT("<p>Test Failures: %d</p>\n"), FailureCount);
	HTML += FString::Printf(TEXT("<p>Flaky Tests: %d</p>\n"), FlakyCount);
	HTML += TEXT("</div>\n");

	// Regression details
	if (Regressions.Num() > 0)
	{
		HTML += TEXT("<h2>Regression Details</h2>\n");
		HTML += TEXT("<table>\n");
		HTML += TEXT("<tr><th>Test Name</th><th>Type</th><th>Severity</th><th>Change</th><th>Description</th></tr>\n");

		for (const FRegressionReport& Regression : Regressions)
		{
			FString SeverityClass = Regression.GetSeverity().ToLower();
			FString TypeString;

			switch (Regression.RegressionType)
			{
			case ERegressionType::Performance:
				TypeString = TEXT("Performance");
				break;
			case ERegressionType::Memory:
				TypeString = TEXT("Memory");
				break;
			case ERegressionType::Failure:
				TypeString = TEXT("Failure");
				break;
			case ERegressionType::Flaky:
				TypeString = TEXT("Flaky");
				break;
			default:
				TypeString = TEXT("Unknown");
				break;
			}

			HTML += FString::Printf(TEXT("<tr class='%s'>"), *SeverityClass);
			HTML += FString::Printf(TEXT("<td>%s</td>"), *Regression.TestName);
			HTML += FString::Printf(TEXT("<td>%s</td>"), *TypeString);
			HTML += FString::Printf(TEXT("<td>%s</td>"), *Regression.GetSeverity());
			HTML += FString::Printf(TEXT("<td>+%.1f%%</td>"), Regression.PercentageChange);
			HTML += FString::Printf(TEXT("<td>%s</td>"), *Regression.Description);
			HTML += TEXT("</tr>\n");
		}

		HTML += TEXT("</table>\n");
	}
	else
	{
		HTML += TEXT("<p>No regressions detected.</p>\n");
	}

	HTML += TEXT("</body>\n</html>");

	return HTML;
}

FString UDelveDeepRegressionDetector::GenerateMarkdownReport() const
{
	FString MD = TEXT("# DelveDeep Regression Report\n\n");
	MD += FString::Printf(TEXT("**Generated:** %s\n\n"), *FDateTime::Now().ToString());

	// Summary
	MD += TEXT("## Summary\n\n");
	MD += FString::Printf(TEXT("- **Total Regressions:** %d\n"), Regressions.Num());
	MD += FString::Printf(TEXT("- **Critical:** %d\n"), GetCriticalRegressions().Num());
	MD += FString::Printf(TEXT("- **Performance:** %d\n"), GetRegressionsByType(ERegressionType::Performance).Num());
	MD += FString::Printf(TEXT("- **Memory:** %d\n"), GetRegressionsByType(ERegressionType::Memory).Num());
	MD += FString::Printf(TEXT("- **Test Failures:** %d\n"), GetRegressionsByType(ERegressionType::Failure).Num());
	MD += FString::Printf(TEXT("- **Flaky Tests:** %d\n\n"), GetRegressionsByType(ERegressionType::Flaky).Num());

	// Regression details
	if (Regressions.Num() > 0)
	{
		MD += TEXT("## Regression Details\n\n");
		MD += TEXT("| Test Name | Type | Severity | Change | Description |\n");
		MD += TEXT("|-----------|------|----------|--------|-------------|\n");

		for (const FRegressionReport& Regression : Regressions)
		{
			FString TypeString;
			switch (Regression.RegressionType)
			{
			case ERegressionType::Performance:
				TypeString = TEXT("Performance");
				break;
			case ERegressionType::Memory:
				TypeString = TEXT("Memory");
				break;
			case ERegressionType::Failure:
				TypeString = TEXT("Failure");
				break;
			case ERegressionType::Flaky:
				TypeString = TEXT("Flaky");
				break;
			default:
				TypeString = TEXT("Unknown");
				break;
			}

			MD += FString::Printf(TEXT("| %s | %s | %s | +%.1f%% | %s |\n"),
				*Regression.TestName,
				*TypeString,
				*Regression.GetSeverity(),
				Regression.PercentageChange,
				*Regression.Description);
		}
	}
	else
	{
		MD += TEXT("No regressions detected.\n");
	}

	return MD;
}

// Console commands for regression detection
static FAutoConsoleCommand CompareTestResultsCommand(
	TEXT("DelveDeep.Regression.Compare"),
	TEXT("Compare test results for regression detection"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UDelveDeepRegressionDetector* Detector = NewObject<UDelveDeepRegressionDetector>();
		if (Detector)
		{
			// In a real implementation, this would load baseline and current reports
			UE_LOG(LogDelveDeepRegression, Display, 
				TEXT("To compare results, use the detector instance with actual reports"));
		}
	})
);

static FAutoConsoleCommand GenerateRegressionReportCommand(
	TEXT("DelveDeep.Regression.GenerateReport"),
	TEXT("Generate regression report"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UDelveDeepRegressionDetector* Detector = NewObject<UDelveDeepRegressionDetector>();
		if (Detector)
		{
			FString Report = Detector->GenerateRegressionReport();
			UE_LOG(LogDelveDeepRegression, Display, TEXT("%s"), *Report);
		}
	})
);

static FAutoConsoleCommand ExportRegressionReportCommand(
	TEXT("DelveDeep.Regression.ExportReport"),
	TEXT("Export regression report to HTML. Usage: DelveDeep.Regression.ExportReport <path>"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() > 0)
		{
			UDelveDeepRegressionDetector* Detector = NewObject<UDelveDeepRegressionDetector>();
			if (Detector)
			{
				Detector->ExportRegressionReport(Args[0]);
			}
		}
		else
		{
			UE_LOG(LogDelveDeepRegression, Warning, 
				TEXT("Usage: DelveDeep.Regression.ExportReport <path>"));
		}
	})
);
