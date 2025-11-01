// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationCommands.h"
#include "DelveDeepValidationSubsystem.h"
#include "DelveDeepValidation.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"

// Console command instances
static FAutoConsoleCommand ValidateObjectCmd(
	TEXT("DelveDeep.ValidateObject"),
	TEXT("Validates a single object by path. Usage: DelveDeep.ValidateObject <ObjectPath>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ValidateObject)
);

static FAutoConsoleCommand ListValidationRulesCmd(
	TEXT("DelveDeep.ListValidationRules"),
	TEXT("Lists all registered validation rules"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ListValidationRules)
);

static FAutoConsoleCommand ListRulesForClassCmd(
	TEXT("DelveDeep.ListRulesForClass"),
	TEXT("Lists validation rules for a specific class. Usage: DelveDeep.ListRulesForClass <ClassName>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ListRulesForClass)
);

static FAutoConsoleCommand ShowValidationCacheCmd(
	TEXT("DelveDeep.ShowValidationCache"),
	TEXT("Displays validation cache statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ShowValidationCache)
);

static FAutoConsoleCommand ClearValidationCacheCmd(
	TEXT("DelveDeep.ClearValidationCache"),
	TEXT("Clears all cached validation results"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ClearValidationCache)
);

static FAutoConsoleCommand ShowValidationMetricsCmd(
	TEXT("DelveDeep.ShowValidationMetrics"),
	TEXT("Displays validation metrics report"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ShowValidationMetrics)
);

static FAutoConsoleCommand ResetValidationMetricsCmd(
	TEXT("DelveDeep.ResetValidationMetrics"),
	TEXT("Resets all validation metrics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ResetValidationMetrics)
);

static FAutoConsoleCommand ExportValidationMetricsCmd(
	TEXT("DelveDeep.ExportValidationMetrics"),
	TEXT("Exports validation metrics to file. Usage: DelveDeep.ExportValidationMetrics [Format] [FilePath]"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ExportValidationMetrics)
);

static FAutoConsoleCommand TestValidationSeverityCmd(
	TEXT("DelveDeep.TestValidationSeverity"),
	TEXT("Tests validation severity levels"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::TestValidationSeverity)
);

static FAutoConsoleCommand ProfileValidationCmd(
	TEXT("DelveDeep.ProfileValidation"),
	TEXT("Profiles validation performance for an object. Usage: DelveDeep.ProfileValidation <ObjectPath>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepValidationCommands::ProfileValidation)
);

void FDelveDeepValidationCommands::RegisterCommands()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation console commands registered"));
}

void FDelveDeepValidationCommands::UnregisterCommands()
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation console commands unregistered"));
}

UDelveDeepValidationSubsystem* FDelveDeepValidationCommands::GetValidationSubsystem()
{
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->GetWorld())
	{
		UWorld* World = GEngine->GameViewport->GetWorld();
		if (World && World->GetGameInstance())
		{
			return World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
		}
	}
	
	UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to get validation subsystem"));
	return nullptr;
}

UObject* FDelveDeepValidationCommands::LoadObjectFromPath(const FString& ObjectPath)
{
	if (ObjectPath.IsEmpty())
	{
		return nullptr;
	}
	
	// Try to load the object
	UObject* Object = StaticLoadObject(UObject::StaticClass(), nullptr, *ObjectPath);
	
	if (!Object)
	{
		// Try finding it if it's already loaded
		Object = FindObject<UObject>(nullptr, *ObjectPath);
	}
	
	return Object;
}

void FDelveDeepValidationCommands::ValidateObject(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Usage: DelveDeep.ValidateObject <ObjectPath>"));
		return;
	}
	
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	FString ObjectPath = Args[0];
	UObject* Object = LoadObjectFromPath(ObjectPath);
	
	if (!Object)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load object: %s"), *ObjectPath);
		return;
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validating object: %s"), *Object->GetName());
	
	FDelveDeepValidationContext Context;
	bool bResult = ValidationSubsystem->ValidateObject(Object, Context);
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Context.GetReport());
	
	if (bResult)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation PASSED"));
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Validation FAILED"));
	}
}

void FDelveDeepValidationCommands::ListValidationRules(const TArray<FString>& Args)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	const TMap<UClass*, TArray<FValidationRuleDefinition>>& AllRules = ValidationSubsystem->GetAllRules();
	
	if (AllRules.Num() == 0)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("No validation rules registered"));
		return;
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Registered Validation Rules ==="));
	
	int32 TotalRules = 0;
	for (const auto& Pair : AllRules)
	{
		UClass* TargetClass = Pair.Key;
		const TArray<FValidationRuleDefinition>& Rules = Pair.Value;
		
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\nClass: %s (%d rules)"), *TargetClass->GetName(), Rules.Num());
		
		for (const FValidationRuleDefinition& Rule : Rules)
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("  - %s (Priority: %d)"), *Rule.RuleName.ToString(), Rule.Priority);
			if (!Rule.Description.IsEmpty())
			{
				UE_LOG(LogDelveDeepConfig, Display, TEXT("    Description: %s"), *Rule.Description);
			}
		}
		
		TotalRules += Rules.Num();
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\nTotal: %d rules across %d classes"), TotalRules, AllRules.Num());
}

void FDelveDeepValidationCommands::ListRulesForClass(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Usage: DelveDeep.ListRulesForClass <ClassName>"));
		return;
	}
	
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	FString ClassName = Args[0];
	UClass* TargetClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);
	
	if (!TargetClass)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Class not found: %s"), *ClassName);
		return;
	}
	
	TArray<FValidationRuleDefinition> Rules = ValidationSubsystem->GetRulesForClass(TargetClass);
	
	if (Rules.Num() == 0)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("No validation rules registered for class: %s"), *ClassName);
		return;
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Validation Rules for %s ==="), *ClassName);
	
	for (int32 i = 0; i < Rules.Num(); ++i)
	{
		const FValidationRuleDefinition& Rule = Rules[i];
		UE_LOG(LogDelveDeepConfig, Display, TEXT("%d. %s (Priority: %d)"), i + 1, *Rule.RuleName.ToString(), Rule.Priority);
		if (!Rule.Description.IsEmpty())
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("   Description: %s"), *Rule.Description);
		}
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\nTotal: %d rules"), Rules.Num());
}

void FDelveDeepValidationCommands::ShowValidationCache(const TArray<FString>& Args)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	// Note: Cache is private, so we can't directly access it
	// This is a placeholder for cache statistics
	UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Validation Cache Statistics ==="));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache statistics not directly accessible"));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Use DelveDeep.ShowValidationMetrics for performance data"));
}

void FDelveDeepValidationCommands::ClearValidationCache(const TArray<FString>& Args)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	ValidationSubsystem->ClearValidationCache();
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation cache cleared"));
}

void FDelveDeepValidationCommands::ShowValidationMetrics(const TArray<FString>& Args)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	FString Report = ValidationSubsystem->GetValidationMetricsReport();
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Report);
}

void FDelveDeepValidationCommands::ResetValidationMetrics(const TArray<FString>& Args)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	ValidationSubsystem->ResetValidationMetrics();
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation metrics reset"));
}

void FDelveDeepValidationCommands::ExportValidationMetrics(const TArray<FString>& Args)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	// Parse arguments
	FString Format = Args.Num() > 0 ? Args[0].ToLower() : TEXT("json");
	FString FilePath = Args.Num() > 1 ? Args[1] : TEXT("");
	
	// Get metrics data
	FValidationMetricsData Metrics = ValidationSubsystem->GetValidationMetrics();
	
	// Generate default file path if not provided
	if (FilePath.IsEmpty())
	{
		if (Format == TEXT("json"))
		{
			FilePath = FPaths::ProjectSavedDir() / TEXT("Validation") / TEXT("Metrics.json");
		}
		else if (Format == TEXT("csv"))
		{
			FilePath = FPaths::ProjectSavedDir() / TEXT("Validation") / TEXT("Metrics.csv");
		}
		else if (Format == TEXT("html"))
		{
			FilePath = FPaths::ProjectSavedDir() / TEXT("Validation") / TEXT("Metrics.html");
		}
		else
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("Unknown format: %s. Supported formats: json, csv, html"), *Format);
			return;
		}
	}
	
	// Export based on format
	FString Content;
	
	if (Format == TEXT("json"))
	{
		// Export as JSON
		ValidationSubsystem->SaveMetricsToFile(FilePath);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Metrics exported to: %s"), *FilePath);
		return;
	}
	else if (Format == TEXT("csv"))
	{
		// Export as CSV
		Content = TEXT("Metric,Value\n");
		Content += FString::Printf(TEXT("Total Validations,%d\n"), Metrics.TotalValidations);
		Content += FString::Printf(TEXT("Passed Validations,%d\n"), Metrics.PassedValidations);
		Content += FString::Printf(TEXT("Failed Validations,%d\n"), Metrics.FailedValidations);
		Content += TEXT("\nError,Frequency\n");
		for (const auto& Pair : Metrics.ErrorFrequency)
		{
			FString EscapedError = Pair.Key.Replace(TEXT(","), TEXT(";"));
			Content += FString::Printf(TEXT("\"%s\",%d\n"), *EscapedError, Pair.Value);
		}
	}
	else if (Format == TEXT("html"))
	{
		// Export as HTML
		Content = TEXT("<!DOCTYPE html>\n<html>\n<head>\n<title>Validation Metrics</title>\n");
		Content += TEXT("<style>body{font-family:Arial;margin:20px;}table{border-collapse:collapse;width:100%;}");
		Content += TEXT("th,td{border:1px solid #ddd;padding:8px;text-align:left;}th{background-color:#4CAF50;color:white;}</style>\n");
		Content += TEXT("</head>\n<body>\n<h1>Validation Metrics Report</h1>\n");
		Content += TEXT("<h2>Summary</h2>\n<table>\n");
		Content += FString::Printf(TEXT("<tr><td>Total Validations</td><td>%d</td></tr>\n"), Metrics.TotalValidations);
		Content += FString::Printf(TEXT("<tr><td>Passed Validations</td><td>%d</td></tr>\n"), Metrics.PassedValidations);
		Content += FString::Printf(TEXT("<tr><td>Failed Validations</td><td>%d</td></tr>\n"), Metrics.FailedValidations);
		Content += TEXT("</table>\n");
		
		if (Metrics.ErrorFrequency.Num() > 0)
		{
			Content += TEXT("<h2>Error Frequency</h2>\n<table>\n<tr><th>Error</th><th>Count</th></tr>\n");
			for (const auto& Pair : Metrics.ErrorFrequency)
			{
				Content += FString::Printf(TEXT("<tr><td>%s</td><td>%d</td></tr>\n"), *Pair.Key, Pair.Value);
			}
			Content += TEXT("</table>\n");
		}
		
		Content += TEXT("</body>\n</html>");
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Unknown format: %s"), *Format);
		return;
	}
	
	// Ensure directory exists
	FString Directory = FPaths::GetPath(FilePath);
	if (!FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}
	
	// Write to file
	if (FFileHelper::SaveStringToFile(Content, *FilePath))
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Metrics exported to: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to export metrics to: %s"), *FilePath);
	}
}

void FDelveDeepValidationCommands::TestValidationSeverity(const TArray<FString>& Args)
{
	UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Testing Validation Severity Levels ===\n"));
	
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("TestSystem");
	Context.OperationName = TEXT("TestValidationSeverity");
	
	// Test each severity level
	Context.AddCritical(TEXT("This is a CRITICAL issue"));
	Context.AddError(TEXT("This is an ERROR"));
	Context.AddWarning(TEXT("This is a WARNING"));
	Context.AddInfo(TEXT("This is an INFO message"));
	
	// Display counts
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Issue Counts:"));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Critical: %d"), Context.GetIssueCount(EValidationSeverity::Critical));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Errors: %d"), Context.GetIssueCount(EValidationSeverity::Error));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Warnings: %d"), Context.GetIssueCount(EValidationSeverity::Warning));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Info: %d"), Context.GetIssueCount(EValidationSeverity::Info));
	
	// Display validation status
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\nValidation Status:"));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Has Critical Issues: %s"), Context.HasCriticalIssues() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Has Errors: %s"), Context.HasErrors() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Has Warnings: %s"), Context.HasWarnings() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("  Is Valid: %s"), Context.IsValid() ? TEXT("Yes") : TEXT("No"));
	
	// Display full report
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Context.GetReport());
}

void FDelveDeepValidationCommands::ProfileValidation(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Usage: DelveDeep.ProfileValidation <ObjectPath>"));
		return;
	}
	
	UDelveDeepValidationSubsystem* ValidationSubsystem = GetValidationSubsystem();
	if (!ValidationSubsystem)
	{
		return;
	}
	
	FString ObjectPath = Args[0];
	UObject* Object = LoadObjectFromPath(ObjectPath);
	
	if (!Object)
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Failed to load object: %s"), *ObjectPath);
		return;
	}
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("=== Profiling Validation Performance ==="));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Object: %s"), *Object->GetName());
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Class: %s\n"), *Object->GetClass()->GetName());
	
	// Run validation multiple times to get average
	const int32 NumIterations = 100;
	double TotalTime = 0.0;
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Running %d validation iterations..."), NumIterations);
	
	for (int32 i = 0; i < NumIterations; ++i)
	{
		FDelveDeepValidationContext Context;
		
		double StartTime = FPlatformTime::Seconds();
		ValidationSubsystem->ValidateObject(Object, Context);
		double EndTime = FPlatformTime::Seconds();
		
		TotalTime += (EndTime - StartTime);
	}
	
	double AvgTime = TotalTime / NumIterations;
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\n=== Performance Results ==="));
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Total Time: %.3f ms"), TotalTime * 1000.0);
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Average Time: %.3f ms"), AvgTime * 1000.0);
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Min Expected: < 1.0 ms"));
	
	if (AvgTime * 1000.0 < 1.0)
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Performance: EXCELLENT (within target)"));
	}
	else if (AvgTime * 1000.0 < 5.0)
	{
		UE_LOG(LogDelveDeepConfig, Warning, TEXT("Performance: ACCEPTABLE (above target but reasonable)"));
	}
	else
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Performance: POOR (significantly above target)"));
	}
	
	// Test with cache
	UE_LOG(LogDelveDeepConfig, Display, TEXT("\n=== Testing Cache Performance ==="));
	
	TotalTime = 0.0;
	for (int32 i = 0; i < NumIterations; ++i)
	{
		FDelveDeepValidationContext Context;
		
		double StartTime = FPlatformTime::Seconds();
		ValidationSubsystem->ValidateObjectWithCache(Object, Context, false);
		double EndTime = FPlatformTime::Seconds();
		
		TotalTime += (EndTime - StartTime);
	}
	
	AvgTime = TotalTime / NumIterations;
	
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Average Time (with cache): %.3f ms"), AvgTime * 1000.0);
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Expected: < 0.1 ms for cache hits"));
}
