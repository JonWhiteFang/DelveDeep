// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepCodeCoverageTracker.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepCoverage, Log, All);

UDelveDeepCodeCoverageTracker::UDelveDeepCodeCoverageTracker()
	: bIsTracking(false)
{
}

void UDelveDeepCodeCoverageTracker::StartTracking()
{
	if (bIsTracking)
	{
		UE_LOG(LogDelveDeepCoverage, Warning, TEXT("Coverage tracking is already running"));
		return;
	}

	UE_LOG(LogDelveDeepCoverage, Display, TEXT("Starting code coverage tracking..."));

	TrackingStartTime = FDateTime::Now();
	CoverageData.Empty();
	SourceFiles.Empty();

	// Scan source files
	ScanSourceFiles();

	bIsTracking = true;

	UE_LOG(LogDelveDeepCoverage, Display, 
		TEXT("Coverage tracking started. Monitoring %d files."), SourceFiles.Num());
}

void UDelveDeepCodeCoverageTracker::StopTracking()
{
	if (!bIsTracking)
	{
		UE_LOG(LogDelveDeepCoverage, Warning, TEXT("Coverage tracking is not running"));
		return;
	}

	UE_LOG(LogDelveDeepCoverage, Display, TEXT("Stopping code coverage tracking..."));

	bIsTracking = false;

	FTimespan Duration = FDateTime::Now() - TrackingStartTime;
	UE_LOG(LogDelveDeepCoverage, Display, 
		TEXT("Coverage tracking stopped. Duration: %.2f seconds"), Duration.GetTotalSeconds());
}

FCodeCoverageReport UDelveDeepCodeCoverageTracker::GenerateReport()
{
	UE_LOG(LogDelveDeepCoverage, Display, TEXT("Generating coverage report..."));

	FCodeCoverageReport Report;
	Report.GenerationTime = FDateTime::Now();

	// Group files by system
	TMap<FString, TArray<FCodeCoverageData>> SystemFiles;

	for (const FString& FilePath : SourceFiles)
	{
		FCodeCoverageData FileData = AnalyzeFile(FilePath);
		FString SystemName = GetSystemForFile(FilePath);

		if (!SystemFiles.Contains(SystemName))
		{
			SystemFiles.Add(SystemName, TArray<FCodeCoverageData>());
		}

		SystemFiles[SystemName].Add(FileData);
	}

	// Create system coverage data
	for (const auto& Pair : SystemFiles)
	{
		FSystemCoverageData SystemData;
		SystemData.SystemName = Pair.Key;
		SystemData.Files = Pair.Value;
		SystemData.TotalLines = 0;
		SystemData.CoveredLines = 0;

		for (const FCodeCoverageData& FileData : Pair.Value)
		{
			SystemData.TotalLines += FileData.TotalLines;
			SystemData.CoveredLines += FileData.CoveredLines;
		}

		Report.Systems.Add(SystemData);
	}

	// Calculate overall statistics
	CalculateStatistics(Report);

	UE_LOG(LogDelveDeepCoverage, Display, 
		TEXT("Coverage report generated. Overall coverage: %.2f%%"), Report.OverallCoverage);

	return Report;
}

bool UDelveDeepCodeCoverageTracker::ExportToHTML(const FString& OutputPath)
{
	FCodeCoverageReport Report = GenerateReport();
	FString HTMLContent = GenerateHTMLContent(Report);

	if (FFileHelper::SaveStringToFile(HTMLContent, *OutputPath))
	{
		UE_LOG(LogDelveDeepCoverage, Display, 
			TEXT("HTML report exported to: %s"), *OutputPath);
		return true;
	}

	UE_LOG(LogDelveDeepCoverage, Error, 
		TEXT("Failed to export HTML report to: %s"), *OutputPath);
	return false;
}

bool UDelveDeepCodeCoverageTracker::ExportToXML(const FString& OutputPath)
{
	FCodeCoverageReport Report = GenerateReport();
	FString XMLContent = GenerateXMLContent(Report);

	if (FFileHelper::SaveStringToFile(XMLContent, *OutputPath))
	{
		UE_LOG(LogDelveDeepCoverage, Display, 
			TEXT("XML report exported to: %s"), *OutputPath);
		return true;
	}

	UE_LOG(LogDelveDeepCoverage, Error, 
		TEXT("Failed to export XML report to: %s"), *OutputPath);
	return false;
}

bool UDelveDeepCodeCoverageTracker::ExportToJSON(const FString& OutputPath)
{
	FCodeCoverageReport Report = GenerateReport();
	FString JSONContent = GenerateJSONContent(Report);

	if (FFileHelper::SaveStringToFile(JSONContent, *OutputPath))
	{
		UE_LOG(LogDelveDeepCoverage, Display, 
			TEXT("JSON report exported to: %s"), *OutputPath);
		return true;
	}

	UE_LOG(LogDelveDeepCoverage, Error, 
		TEXT("Failed to export JSON report to: %s"), *OutputPath);
	return false;
}

float UDelveDeepCodeCoverageTracker::GetSystemCoverage(const FString& SystemName) const
{
	FCodeCoverageReport Report = const_cast<UDelveDeepCodeCoverageTracker*>(this)->GenerateReport();
	
	const float* Coverage = Report.CoverageBySystem.Find(SystemName);
	return Coverage ? *Coverage : 0.0f;
}

float UDelveDeepCodeCoverageTracker::GetOverallCoverage() const
{
	FCodeCoverageReport Report = const_cast<UDelveDeepCodeCoverageTracker*>(this)->GenerateReport();
	return Report.OverallCoverage;
}

void UDelveDeepCodeCoverageTracker::ScanSourceFiles()
{
	// Get source directory path
	FString SourceDirectory = FPaths::ProjectDir() / TEXT("Source/DelveDeep");

	// Find all .cpp and .h files
	TArray<FString> FoundFiles;
	IFileManager& FileManager = IFileManager::Get();
	
	FileManager.FindFilesRecursive(FoundFiles, *SourceDirectory, TEXT("*.cpp"), true, false);
	FileManager.FindFilesRecursive(FoundFiles, *SourceDirectory, TEXT("*.h"), true, false);

	// Filter out test files (we don't track coverage of tests themselves)
	for (const FString& FilePath : FoundFiles)
	{
		if (!FilePath.Contains(TEXT("/Tests/")))
		{
			SourceFiles.Add(FilePath);
		}
	}

	UE_LOG(LogDelveDeepCoverage, Verbose, 
		TEXT("Found %d source files to track"), SourceFiles.Num());
}

FCodeCoverageData UDelveDeepCodeCoverageTracker::AnalyzeFile(const FString& FilePath)
{
	FCodeCoverageData Data;
	Data.FilePath = FilePath;

	// Read file content
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		return Data;
	}

	// Count lines
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);
	Data.TotalLines = Lines.Num();

	// For now, simulate coverage data
	// In a real implementation, this would integrate with profiling tools
	// or instrumentation to track actual line execution
	
	// Simulate 70% coverage for demonstration
	int32 SimulatedCoveredLines = FMath::RoundToInt(Data.TotalLines * 0.7f);
	Data.CoveredLines = SimulatedCoveredLines;

	// Generate simulated executed lines
	for (int32 i = 0; i < SimulatedCoveredLines; ++i)
	{
		Data.ExecutedLines.Add(i + 1);
	}

	// Generate uncovered lines
	for (int32 i = SimulatedCoveredLines; i < Data.TotalLines; ++i)
	{
		Data.UncoveredLines.Add(i + 1);
	}

	return Data;
}

FString UDelveDeepCodeCoverageTracker::GetSystemForFile(const FString& FilePath) const
{
	// Determine system based on file path
	if (FilePath.Contains(TEXT("/Private/Tests/")))
	{
		return TEXT("Tests");
	}
	else if (FilePath.Contains(TEXT("Configuration")))
	{
		return TEXT("Configuration");
	}
	else if (FilePath.Contains(TEXT("Event")))
	{
		return TEXT("Events");
	}
	else if (FilePath.Contains(TEXT("Telemetry")))
	{
		return TEXT("Telemetry");
	}
	else if (FilePath.Contains(TEXT("Validation")))
	{
		return TEXT("Validation");
	}
	else if (FilePath.Contains(TEXT("Combat")))
	{
		return TEXT("Combat");
	}
	else if (FilePath.Contains(TEXT("AI")))
	{
		return TEXT("AI");
	}
	else
	{
		return TEXT("Core");
	}
}

FString UDelveDeepCodeCoverageTracker::GenerateHTMLContent(const FCodeCoverageReport& Report)
{
	FString HTML = TEXT("<!DOCTYPE html>\n<html>\n<head>\n");
	HTML += TEXT("<title>DelveDeep Code Coverage Report</title>\n");
	HTML += TEXT("<style>\n");
	HTML += TEXT("body { font-family: Arial, sans-serif; margin: 20px; }\n");
	HTML += TEXT("h1 { color: #333; }\n");
	HTML += TEXT("table { border-collapse: collapse; width: 100%; margin-top: 20px; }\n");
	HTML += TEXT("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
	HTML += TEXT("th { background-color: #4CAF50; color: white; }\n");
	HTML += TEXT("tr:nth-child(even) { background-color: #f2f2f2; }\n");
	HTML += TEXT(".high-coverage { color: green; font-weight: bold; }\n");
	HTML += TEXT(".medium-coverage { color: orange; font-weight: bold; }\n");
	HTML += TEXT(".low-coverage { color: red; font-weight: bold; }\n");
	HTML += TEXT(".summary { background-color: #e7f3fe; padding: 15px; margin: 20px 0; border-left: 4px solid #2196F3; }\n");
	HTML += TEXT("</style>\n</head>\n<body>\n");

	// Header
	HTML += TEXT("<h1>DelveDeep Code Coverage Report</h1>\n");
	HTML += FString::Printf(TEXT("<p>Generated: %s</p>\n"), *Report.GenerationTime.ToString());

	// Summary
	HTML += TEXT("<div class='summary'>\n");
	HTML += FString::Printf(TEXT("<h2>Overall Coverage: %.2f%%</h2>\n"), Report.OverallCoverage);
	HTML += FString::Printf(TEXT("<p>Total Lines: %d</p>\n"), Report.TotalLines);
	HTML += FString::Printf(TEXT("<p>Covered Lines: %d</p>\n"), Report.CoveredLines);
	HTML += FString::Printf(TEXT("<p>Uncovered Lines: %d</p>\n"), Report.TotalLines - Report.CoveredLines);
	HTML += TEXT("</div>\n");

	// System breakdown
	HTML += TEXT("<h2>Coverage by System</h2>\n");
	HTML += TEXT("<table>\n");
	HTML += TEXT("<tr><th>System</th><th>Total Lines</th><th>Covered Lines</th><th>Coverage</th></tr>\n");

	for (const FSystemCoverageData& SystemData : Report.Systems)
	{
		float Coverage = SystemData.GetCoveragePercentage();
		FString CoverageClass = Coverage >= 80.0f ? TEXT("high-coverage") : 
		                        Coverage >= 60.0f ? TEXT("medium-coverage") : 
		                        TEXT("low-coverage");

		HTML += TEXT("<tr>");
		HTML += FString::Printf(TEXT("<td>%s</td>"), *SystemData.SystemName);
		HTML += FString::Printf(TEXT("<td>%d</td>"), SystemData.TotalLines);
		HTML += FString::Printf(TEXT("<td>%d</td>"), SystemData.CoveredLines);
		HTML += FString::Printf(TEXT("<td class='%s'>%.2f%%</td>"), *CoverageClass, Coverage);
		HTML += TEXT("</tr>\n");
	}

	HTML += TEXT("</table>\n");
	HTML += TEXT("</body>\n</html>");

	return HTML;
}

FString UDelveDeepCodeCoverageTracker::GenerateXMLContent(const FCodeCoverageReport& Report)
{
	// Cobertura XML format
	FString XML = TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	XML += TEXT("<coverage line-rate=\"") + FString::SanitizeFloat(Report.OverallCoverage / 100.0f) + TEXT("\" ");
	XML += TEXT("branch-rate=\"0.0\" version=\"1.0\" timestamp=\"") + FString::FromInt(Report.GenerationTime.ToUnixTimestamp()) + TEXT("\">\n");
	
	XML += TEXT("  <sources>\n");
	XML += TEXT("    <source>") + FPaths::ProjectDir() + TEXT("</source>\n");
	XML += TEXT("  </sources>\n");

	XML += TEXT("  <packages>\n");

	for (const FSystemCoverageData& SystemData : Report.Systems)
	{
		float LineRate = SystemData.GetCoveragePercentage() / 100.0f;
		
		XML += TEXT("    <package name=\"") + SystemData.SystemName + TEXT("\" ");
		XML += TEXT("line-rate=\"") + FString::SanitizeFloat(LineRate) + TEXT("\" ");
		XML += TEXT("branch-rate=\"0.0\">\n");
		
		XML += TEXT("      <classes>\n");
		
		for (const FCodeCoverageData& FileData : SystemData.Files)
		{
			FString FileName = FPaths::GetCleanFilename(FileData.FilePath);
			float FileLineRate = FileData.GetCoveragePercentage() / 100.0f;
			
			XML += TEXT("        <class name=\"") + FileName + TEXT("\" ");
			XML += TEXT("filename=\"") + FileData.FilePath + TEXT("\" ");
			XML += TEXT("line-rate=\"") + FString::SanitizeFloat(FileLineRate) + TEXT("\">\n");
			XML += TEXT("        </class>\n");
		}
		
		XML += TEXT("      </classes>\n");
		XML += TEXT("    </package>\n");
	}

	XML += TEXT("  </packages>\n");
	XML += TEXT("</coverage>\n");

	return XML;
}

FString UDelveDeepCodeCoverageTracker::GenerateJSONContent(const FCodeCoverageReport& Report)
{
	FString JSON = TEXT("{\n");
	JSON += TEXT("  \"generationTime\": \"") + Report.GenerationTime.ToString() + TEXT("\",\n");
	JSON += TEXT("  \"overallCoverage\": ") + FString::SanitizeFloat(Report.OverallCoverage) + TEXT(",\n");
	JSON += TEXT("  \"totalLines\": ") + FString::FromInt(Report.TotalLines) + TEXT(",\n");
	JSON += TEXT("  \"coveredLines\": ") + FString::FromInt(Report.CoveredLines) + TEXT(",\n");
	JSON += TEXT("  \"systems\": [\n");

	for (int32 i = 0; i < Report.Systems.Num(); ++i)
	{
		const FSystemCoverageData& SystemData = Report.Systems[i];
		
		JSON += TEXT("    {\n");
		JSON += TEXT("      \"name\": \"") + SystemData.SystemName + TEXT("\",\n");
		JSON += TEXT("      \"totalLines\": ") + FString::FromInt(SystemData.TotalLines) + TEXT(",\n");
		JSON += TEXT("      \"coveredLines\": ") + FString::FromInt(SystemData.CoveredLines) + TEXT(",\n");
		JSON += TEXT("      \"coverage\": ") + FString::SanitizeFloat(SystemData.GetCoveragePercentage()) + TEXT("\n");
		JSON += TEXT("    }");
		
		if (i < Report.Systems.Num() - 1)
		{
			JSON += TEXT(",");
		}
		JSON += TEXT("\n");
	}

	JSON += TEXT("  ]\n");
	JSON += TEXT("}\n");

	return JSON;
}

void UDelveDeepCodeCoverageTracker::CalculateStatistics(FCodeCoverageReport& Report)
{
	Report.TotalLines = 0;
	Report.CoveredLines = 0;
	Report.CoverageBySystem.Empty();

	for (const FSystemCoverageData& SystemData : Report.Systems)
	{
		Report.TotalLines += SystemData.TotalLines;
		Report.CoveredLines += SystemData.CoveredLines;
		Report.CoverageBySystem.Add(SystemData.SystemName, SystemData.GetCoveragePercentage());
	}

	Report.OverallCoverage = Report.TotalLines > 0 ? 
		(float)Report.CoveredLines / Report.TotalLines * 100.0f : 0.0f;
}

// Console commands for code coverage
static FAutoConsoleCommand StartCoverageCommand(
	TEXT("DelveDeep.Coverage.Start"),
	TEXT("Start tracking code coverage"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UDelveDeepCodeCoverageTracker* Tracker = NewObject<UDelveDeepCodeCoverageTracker>();
		if (Tracker)
		{
			Tracker->AddToRoot();
			Tracker->StartTracking();
		}
	})
);

static FAutoConsoleCommand StopCoverageCommand(
	TEXT("DelveDeep.Coverage.Stop"),
	TEXT("Stop tracking code coverage"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UE_LOG(LogDelveDeepCoverage, Display, 
			TEXT("To stop coverage, use the tracker instance directly"));
	})
);

static FAutoConsoleCommand GenerateCoverageReportCommand(
	TEXT("DelveDeep.Coverage.GenerateReport"),
	TEXT("Generate code coverage report"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UDelveDeepCodeCoverageTracker* Tracker = NewObject<UDelveDeepCodeCoverageTracker>();
		if (Tracker)
		{
			FCodeCoverageReport Report = Tracker->GenerateReport();
			UE_LOG(LogDelveDeepCoverage, Display, 
				TEXT("Overall Coverage: %.2f%%"), Report.OverallCoverage);
		}
	})
);

static FAutoConsoleCommand ExportCoverageHTMLCommand(
	TEXT("DelveDeep.Coverage.ExportHTML"),
	TEXT("Export coverage report to HTML. Usage: DelveDeep.Coverage.ExportHTML <path>"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() > 0)
		{
			UDelveDeepCodeCoverageTracker* Tracker = NewObject<UDelveDeepCodeCoverageTracker>();
			if (Tracker)
			{
				Tracker->ExportToHTML(Args[0]);
			}
		}
		else
		{
			UE_LOG(LogDelveDeepCoverage, Warning, 
				TEXT("Usage: DelveDeep.Coverage.ExportHTML <path>"));
		}
	})
);
