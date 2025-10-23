// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"

// Performance profiling stats (declared in DelveDeepValidationSubsystem.cpp)
DECLARE_STATS_GROUP(TEXT("DelveDeepValidation"), STATGROUP_DelveDeepValidation, STATCAT_Advanced);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Generate Report"), STAT_GenerateReport, STATGROUP_DelveDeepValidation, );

DEFINE_LOG_CATEGORY(LogDelveDeepConfig);

void FValidationContext::AddError(const FString& Error)
{
	// Add to legacy array for backward compatibility
	ValidationErrors.Add(Error);
	
	// Add to new Issues array with Error severity
	AddIssue(EValidationSeverity::Error, Error);
}

void FValidationContext::AddWarning(const FString& Warning)
{
	// Add to legacy array for backward compatibility
	ValidationWarnings.Add(Warning);
	
	// Add to new Issues array with Warning severity
	AddIssue(EValidationSeverity::Warning, Warning);
}

void FValidationContext::AddIssue(EValidationSeverity Severity, const FString& Message, 
								  const FString& SourceFile, int32 SourceLine, 
								  const FString& SourceFunction)
{
	FValidationIssue Issue;
	Issue.Severity = Severity;
	Issue.Message = Message;
	Issue.SourceFile = SourceFile;
	Issue.SourceLine = SourceLine;
	Issue.SourceFunction = SourceFunction;
	Issue.Timestamp = FDateTime::Now();
	
	Issues.Add(Issue);
	
	// Log based on severity
	switch (Severity)
	{
		case EValidationSeverity::Critical:
			UE_LOG(LogDelveDeepConfig, Error, TEXT("[%s::%s] Critical: %s"), 
				*SystemName, *OperationName, *Message);
			break;
			
		case EValidationSeverity::Error:
			UE_LOG(LogDelveDeepConfig, Error, TEXT("[%s::%s] Error: %s"), 
				*SystemName, *OperationName, *Message);
			break;
			
		case EValidationSeverity::Warning:
			UE_LOG(LogDelveDeepConfig, Warning, TEXT("[%s::%s] Warning: %s"), 
				*SystemName, *OperationName, *Message);
			break;
			
		case EValidationSeverity::Info:
			UE_LOG(LogDelveDeepConfig, Display, TEXT("[%s::%s] Info: %s"), 
				*SystemName, *OperationName, *Message);
			break;
	}
}

void FValidationContext::AddCritical(const FString& Message)
{
	AddIssue(EValidationSeverity::Critical, Message);
}

void FValidationContext::AddInfo(const FString& Message)
{
	AddIssue(EValidationSeverity::Info, Message);
}

bool FValidationContext::HasCriticalIssues() const
{
	return GetIssueCount(EValidationSeverity::Critical) > 0;
}

bool FValidationContext::HasErrors() const
{
	return GetIssueCount(EValidationSeverity::Error) > 0;
}

bool FValidationContext::HasWarnings() const
{
	return GetIssueCount(EValidationSeverity::Warning) > 0;
}

int32 FValidationContext::GetIssueCount(EValidationSeverity Severity) const
{
	int32 Count = 0;
	for (const FValidationIssue& Issue : Issues)
	{
		if (Issue.Severity == Severity)
		{
			Count++;
		}
	}
	return Count;
}

bool FValidationContext::IsValid() const
{
	// Check for Critical or Error severity issues
	return !HasCriticalIssues() && !HasErrors();
}

void FValidationContext::AddChildContext(const FValidationContext& ChildContext)
{
	ChildContexts.Add(ChildContext);
}

void FValidationContext::MergeContext(const FValidationContext& OtherContext)
{
	// Merge issues
	Issues.Append(OtherContext.Issues);
	
	// Merge legacy arrays for backward compatibility
	ValidationErrors.Append(OtherContext.ValidationErrors);
	ValidationWarnings.Append(OtherContext.ValidationWarnings);
	
	// Merge child contexts
	ChildContexts.Append(OtherContext.ChildContexts);
}

void FValidationContext::AttachMetadata(const FString& Key, const FString& Value)
{
	if (Issues.Num() > 0)
	{
		Issues.Last().Metadata.Add(Key, Value);
	}
}

FTimespan FValidationContext::GetValidationDuration() const
{
	if (CompletionTime == FDateTime::MinValue())
	{
		// If not completed, calculate duration from now
		return FDateTime::Now() - CreationTime;
	}
	return CompletionTime - CreationTime;
}

FString FValidationContext::GetReport() const
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateReport);
	return GetNestedReport(0);
}

FString FValidationContext::GetNestedReport(int32 IndentLevel) const
{
	FString Report;
	FString Indent = FString::ChrN(IndentLevel * 2, ' ');
	
	// Add header
	Report += Indent + FString::Printf(TEXT("=== Validation Report ===\n"));
	Report += Indent + FString::Printf(TEXT("System: %s\n"), *SystemName);
	Report += Indent + FString::Printf(TEXT("Operation: %s\n"), *OperationName);
	
	// Add timestamp information
	Report += Indent + FString::Printf(TEXT("Created: %s\n"), *CreationTime.ToString());
	if (CompletionTime != FDateTime::MinValue())
	{
		Report += Indent + FString::Printf(TEXT("Completed: %s\n"), *CompletionTime.ToString());
		FTimespan Duration = GetValidationDuration();
		Report += Indent + FString::Printf(TEXT("Duration: %.3f ms\n"), Duration.GetTotalMilliseconds());
	}
	Report += TEXT("\n");
	
	// Group issues by severity
	int32 CriticalCount = GetIssueCount(EValidationSeverity::Critical);
	int32 ErrorCount = GetIssueCount(EValidationSeverity::Error);
	int32 WarningCount = GetIssueCount(EValidationSeverity::Warning);
	int32 InfoCount = GetIssueCount(EValidationSeverity::Info);
	
	// Add critical issues section
	if (CriticalCount > 0)
	{
		Report += Indent + FString::Printf(TEXT("CRITICAL ISSUES (%d):\n"), CriticalCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Critical)
			{
				Report += Indent + FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (!Issue.SourceFile.IsEmpty())
				{
					Report += Indent + FString::Printf(TEXT("     Source: %s:%d (%s)\n"), 
						*Issue.SourceFile, Issue.SourceLine, *Issue.SourceFunction);
				}
				if (Issue.Metadata.Num() > 0)
				{
					Report += Indent + TEXT("     Metadata: ");
					for (const auto& Pair : Issue.Metadata)
					{
						Report += FString::Printf(TEXT("%s=%s; "), *Pair.Key, *Pair.Value);
					}
					Report += TEXT("\n");
				}
			}
		}
		Report += TEXT("\n");
	}
	
	// Add errors section
	if (ErrorCount > 0)
	{
		Report += Indent + FString::Printf(TEXT("ERRORS (%d):\n"), ErrorCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Error)
			{
				Report += Indent + FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (!Issue.SourceFile.IsEmpty())
				{
					Report += Indent + FString::Printf(TEXT("     Source: %s:%d (%s)\n"), 
						*Issue.SourceFile, Issue.SourceLine, *Issue.SourceFunction);
				}
				if (Issue.Metadata.Num() > 0)
				{
					Report += Indent + TEXT("     Metadata: ");
					for (const auto& Pair : Issue.Metadata)
					{
						Report += FString::Printf(TEXT("%s=%s; "), *Pair.Key, *Pair.Value);
					}
					Report += TEXT("\n");
				}
			}
		}
		Report += TEXT("\n");
	}
	else if (CriticalCount == 0)
	{
		Report += Indent + TEXT("No errors found.\n\n");
	}
	
	// Add warnings section
	if (WarningCount > 0)
	{
		Report += Indent + FString::Printf(TEXT("WARNINGS (%d):\n"), WarningCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Warning)
			{
				Report += Indent + FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (!Issue.SourceFile.IsEmpty())
				{
					Report += Indent + FString::Printf(TEXT("     Source: %s:%d (%s)\n"), 
						*Issue.SourceFile, Issue.SourceLine, *Issue.SourceFunction);
				}
				if (Issue.Metadata.Num() > 0)
				{
					Report += Indent + TEXT("     Metadata: ");
					for (const auto& Pair : Issue.Metadata)
					{
						Report += FString::Printf(TEXT("%s=%s; "), *Pair.Key, *Pair.Value);
					}
					Report += TEXT("\n");
				}
			}
		}
		Report += TEXT("\n");
	}
	else
	{
		Report += Indent + TEXT("No warnings found.\n\n");
	}
	
	// Add info section
	if (InfoCount > 0)
	{
		Report += Indent + FString::Printf(TEXT("INFO (%d):\n"), InfoCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Info)
			{
				Report += Indent + FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (Issue.Metadata.Num() > 0)
				{
					Report += Indent + TEXT("     Metadata: ");
					for (const auto& Pair : Issue.Metadata)
					{
						Report += FString::Printf(TEXT("%s=%s; "), *Pair.Key, *Pair.Value);
					}
					Report += TEXT("\n");
				}
			}
		}
		Report += TEXT("\n");
	}
	
	// Add child contexts
	if (ChildContexts.Num() > 0)
	{
		Report += Indent + FString::Printf(TEXT("NESTED CONTEXTS (%d):\n"), ChildContexts.Num());
		for (int32 i = 0; i < ChildContexts.Num(); ++i)
		{
			Report += Indent + FString::Printf(TEXT("  [Child Context %d]\n"), i + 1);
			Report += ChildContexts[i].GetNestedReport(IndentLevel + 2);
		}
		Report += TEXT("\n");
	}
	
	// Add summary
	Report += Indent + FString::Printf(TEXT("=== Summary ===\n"));
	Report += Indent + FString::Printf(TEXT("Status: %s\n"), IsValid() ? TEXT("PASSED") : TEXT("FAILED"));
	Report += Indent + FString::Printf(TEXT("Critical Issues: %d\n"), CriticalCount);
	Report += Indent + FString::Printf(TEXT("Errors: %d\n"), ErrorCount);
	Report += Indent + FString::Printf(TEXT("Warnings: %d\n"), WarningCount);
	Report += Indent + FString::Printf(TEXT("Info: %d\n"), InfoCount);
	if (ChildContexts.Num() > 0)
	{
		Report += Indent + FString::Printf(TEXT("Child Contexts: %d\n"), ChildContexts.Num());
	}
	
	return Report;
}

FString FValidationContext::GetReportJSON() const
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateReport);
	
	FString JSON = TEXT("{\n");
	
	// Add context information
	JSON += FString::Printf(TEXT("  \"systemName\": \"%s\",\n"), *SystemName.ReplaceCharWithEscapedChar());
	JSON += FString::Printf(TEXT("  \"operationName\": \"%s\",\n"), *OperationName.ReplaceCharWithEscapedChar());
	JSON += FString::Printf(TEXT("  \"creationTime\": \"%s\",\n"), *CreationTime.ToIso8601());
	
	if (CompletionTime != FDateTime::MinValue())
	{
		JSON += FString::Printf(TEXT("  \"completionTime\": \"%s\",\n"), *CompletionTime.ToIso8601());
		JSON += FString::Printf(TEXT("  \"durationMs\": %.3f,\n"), GetValidationDuration().GetTotalMilliseconds());
	}
	
	JSON += FString::Printf(TEXT("  \"isValid\": %s,\n"), IsValid() ? TEXT("true") : TEXT("false"));
	
	// Add issue counts
	JSON += TEXT("  \"summary\": {\n");
	JSON += FString::Printf(TEXT("    \"critical\": %d,\n"), GetIssueCount(EValidationSeverity::Critical));
	JSON += FString::Printf(TEXT("    \"errors\": %d,\n"), GetIssueCount(EValidationSeverity::Error));
	JSON += FString::Printf(TEXT("    \"warnings\": %d,\n"), GetIssueCount(EValidationSeverity::Warning));
	JSON += FString::Printf(TEXT("    \"info\": %d\n"), GetIssueCount(EValidationSeverity::Info));
	JSON += TEXT("  },\n");
	
	// Add issues array
	JSON += TEXT("  \"issues\": [\n");
	for (int32 i = 0; i < Issues.Num(); ++i)
	{
		const FValidationIssue& Issue = Issues[i];
		JSON += TEXT("    {\n");
		
		// Severity
		FString SeverityStr;
		switch (Issue.Severity)
		{
			case EValidationSeverity::Critical: SeverityStr = TEXT("critical"); break;
			case EValidationSeverity::Error: SeverityStr = TEXT("error"); break;
			case EValidationSeverity::Warning: SeverityStr = TEXT("warning"); break;
			case EValidationSeverity::Info: SeverityStr = TEXT("info"); break;
		}
		JSON += FString::Printf(TEXT("      \"severity\": \"%s\",\n"), *SeverityStr);
		JSON += FString::Printf(TEXT("      \"message\": \"%s\",\n"), *Issue.Message.ReplaceCharWithEscapedChar());
		JSON += FString::Printf(TEXT("      \"timestamp\": \"%s\""), *Issue.Timestamp.ToIso8601());
		
		// Source location
		if (!Issue.SourceFile.IsEmpty())
		{
			JSON += TEXT(",\n");
			JSON += FString::Printf(TEXT("      \"sourceFile\": \"%s\",\n"), *Issue.SourceFile.ReplaceCharWithEscapedChar());
			JSON += FString::Printf(TEXT("      \"sourceLine\": %d,\n"), Issue.SourceLine);
			JSON += FString::Printf(TEXT("      \"sourceFunction\": \"%s\""), *Issue.SourceFunction.ReplaceCharWithEscapedChar());
		}
		
		// Metadata
		if (Issue.Metadata.Num() > 0)
		{
			JSON += TEXT(",\n      \"metadata\": {\n");
			int32 MetaIndex = 0;
			for (const auto& Pair : Issue.Metadata)
			{
				JSON += FString::Printf(TEXT("        \"%s\": \"%s\""), 
					*Pair.Key.ReplaceCharWithEscapedChar(), 
					*Pair.Value.ReplaceCharWithEscapedChar());
				if (++MetaIndex < Issue.Metadata.Num())
				{
					JSON += TEXT(",");
				}
				JSON += TEXT("\n");
			}
			JSON += TEXT("      }");
		}
		
		JSON += TEXT("\n    }");
		if (i < Issues.Num() - 1)
		{
			JSON += TEXT(",");
		}
		JSON += TEXT("\n");
	}
	JSON += TEXT("  ]");
	
	// Add child contexts
	if (ChildContexts.Num() > 0)
	{
		JSON += TEXT(",\n  \"childContexts\": [\n");
		for (int32 i = 0; i < ChildContexts.Num(); ++i)
		{
			FString ChildJSON = ChildContexts[i].GetReportJSON();
			// Indent child JSON
			ChildJSON = ChildJSON.Replace(TEXT("\n"), TEXT("\n    "));
			JSON += TEXT("    ") + ChildJSON;
			if (i < ChildContexts.Num() - 1)
			{
				JSON += TEXT(",");
			}
			JSON += TEXT("\n");
		}
		JSON += TEXT("  ]");
	}
	
	JSON += TEXT("\n}");
	return JSON;
}

FString FValidationContext::GetReportCSV() const
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateReport);
	
	FString CSV;
	
	// Add header row
	CSV += TEXT("Severity,Message,Timestamp,System,Operation,SourceFile,SourceLine,SourceFunction,Metadata\n");
	
	// Add issues
	for (const FValidationIssue& Issue : Issues)
	{
		// Severity
		FString SeverityStr;
		switch (Issue.Severity)
		{
			case EValidationSeverity::Critical: SeverityStr = TEXT("Critical"); break;
			case EValidationSeverity::Error: SeverityStr = TEXT("Error"); break;
			case EValidationSeverity::Warning: SeverityStr = TEXT("Warning"); break;
			case EValidationSeverity::Info: SeverityStr = TEXT("Info"); break;
		}
		
		// Escape CSV fields (wrap in quotes if contains comma, quote, or newline)
		auto EscapeCSV = [](const FString& Field) -> FString
		{
			if (Field.Contains(TEXT(",")) || Field.Contains(TEXT("\"")) || Field.Contains(TEXT("\n")))
			{
				return TEXT("\"") + Field.Replace(TEXT("\""), TEXT("\"\"")) + TEXT("\"");
			}
			return Field;
		};
		
		CSV += EscapeCSV(SeverityStr) + TEXT(",");
		CSV += EscapeCSV(Issue.Message) + TEXT(",");
		CSV += EscapeCSV(Issue.Timestamp.ToString()) + TEXT(",");
		CSV += EscapeCSV(SystemName) + TEXT(",");
		CSV += EscapeCSV(OperationName) + TEXT(",");
		CSV += EscapeCSV(Issue.SourceFile) + TEXT(",");
		CSV += FString::Printf(TEXT("%d"), Issue.SourceLine) + TEXT(",");
		CSV += EscapeCSV(Issue.SourceFunction) + TEXT(",");
		
		// Metadata as key=value pairs
		FString MetadataStr;
		for (const auto& Pair : Issue.Metadata)
		{
			if (!MetadataStr.IsEmpty())
			{
				MetadataStr += TEXT("; ");
			}
			MetadataStr += Pair.Key + TEXT("=") + Pair.Value;
		}
		CSV += EscapeCSV(MetadataStr);
		
		CSV += TEXT("\n");
	}
	
	// Add child context issues
	for (const FValidationContext& ChildContext : ChildContexts)
	{
		// Recursively get child CSV (skip header)
		FString ChildCSV = ChildContext.GetReportCSV();
		TArray<FString> Lines;
		ChildCSV.ParseIntoArrayLines(Lines);
		for (int32 i = 1; i < Lines.Num(); ++i) // Skip header
		{
			CSV += Lines[i] + TEXT("\n");
		}
	}
	
	return CSV;
}

FString FValidationContext::GetReportHTML() const
{
	SCOPE_CYCLE_COUNTER(STAT_GenerateReport);
	
	FString HTML;
	
	// HTML header with embedded CSS
	HTML += TEXT("<!DOCTYPE html>\n<html>\n<head>\n");
	HTML += TEXT("<meta charset=\"UTF-8\">\n");
	HTML += TEXT("<title>Validation Report - ") + SystemName + TEXT("</title>\n");
	HTML += TEXT("<style>\n");
	HTML += TEXT("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 20px; background-color: #f5f5f5; }\n");
	HTML += TEXT(".container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
	HTML += TEXT("h1 { color: #333; border-bottom: 3px solid #007acc; padding-bottom: 10px; }\n");
	HTML += TEXT("h2 { color: #555; margin-top: 30px; }\n");
	HTML += TEXT(".header-info { background-color: #f9f9f9; padding: 15px; border-radius: 5px; margin-bottom: 20px; }\n");
	HTML += TEXT(".header-info p { margin: 5px 0; }\n");
	HTML += TEXT(".summary { display: flex; gap: 20px; margin: 20px 0; }\n");
	HTML += TEXT(".summary-box { flex: 1; padding: 15px; border-radius: 5px; text-align: center; }\n");
	HTML += TEXT(".summary-box.passed { background-color: #d4edda; border: 2px solid #28a745; }\n");
	HTML += TEXT(".summary-box.failed { background-color: #f8d7da; border: 2px solid #dc3545; }\n");
	HTML += TEXT(".summary-box h3 { margin: 0 0 10px 0; }\n");
	HTML += TEXT(".summary-box .count { font-size: 32px; font-weight: bold; }\n");
	HTML += TEXT(".issue { margin: 10px 0; padding: 15px; border-radius: 5px; border-left: 4px solid; }\n");
	HTML += TEXT(".issue.critical { background-color: #f8d7da; border-color: #721c24; }\n");
	HTML += TEXT(".issue.error { background-color: #f8d7da; border-color: #dc3545; }\n");
	HTML += TEXT(".issue.warning { background-color: #fff3cd; border-color: #ffc107; }\n");
	HTML += TEXT(".issue.info { background-color: #d1ecf1; border-color: #17a2b8; }\n");
	HTML += TEXT(".issue-header { font-weight: bold; margin-bottom: 5px; }\n");
	HTML += TEXT(".issue-message { margin: 5px 0; }\n");
	HTML += TEXT(".issue-source { font-size: 0.9em; color: #666; font-family: monospace; }\n");
	HTML += TEXT(".issue-metadata { font-size: 0.9em; color: #666; margin-top: 5px; }\n");
	HTML += TEXT(".child-context { margin-left: 20px; padding-left: 20px; border-left: 3px solid #ccc; margin-top: 20px; }\n");
	HTML += TEXT(".collapsible { cursor: pointer; padding: 10px; background-color: #007acc; color: white; border: none; text-align: left; width: 100%; font-size: 16px; border-radius: 5px; margin-top: 10px; }\n");
	HTML += TEXT(".collapsible:hover { background-color: #005a9e; }\n");
	HTML += TEXT(".content { display: none; padding: 10px; margin-top: 5px; }\n");
	HTML += TEXT(".content.show { display: block; }\n");
	HTML += TEXT("</style>\n");
	HTML += TEXT("<script>\n");
	HTML += TEXT("function toggleSection(id) {\n");
	HTML += TEXT("  var content = document.getElementById(id);\n");
	HTML += TEXT("  content.classList.toggle('show');\n");
	HTML += TEXT("}\n");
	HTML += TEXT("</script>\n");
	HTML += TEXT("</head>\n<body>\n");
	
	// Container
	HTML += TEXT("<div class=\"container\">\n");
	
	// Header
	HTML += TEXT("<h1>Validation Report</h1>\n");
	HTML += TEXT("<div class=\"header-info\">\n");
	HTML += TEXT("<p><strong>System:</strong> ") + SystemName + TEXT("</p>\n");
	HTML += TEXT("<p><strong>Operation:</strong> ") + OperationName + TEXT("</p>\n");
	HTML += TEXT("<p><strong>Created:</strong> ") + CreationTime.ToString() + TEXT("</p>\n");
	if (CompletionTime != FDateTime::MinValue())
	{
		HTML += TEXT("<p><strong>Completed:</strong> ") + CompletionTime.ToString() + TEXT("</p>\n");
		HTML += FString::Printf(TEXT("<p><strong>Duration:</strong> %.3f ms</p>\n"), GetValidationDuration().GetTotalMilliseconds());
	}
	HTML += TEXT("</div>\n");
	
	// Summary boxes
	int32 CriticalCount = GetIssueCount(EValidationSeverity::Critical);
	int32 ErrorCount = GetIssueCount(EValidationSeverity::Error);
	int32 WarningCount = GetIssueCount(EValidationSeverity::Warning);
	int32 InfoCount = GetIssueCount(EValidationSeverity::Info);
	
	HTML += TEXT("<div class=\"summary\">\n");
	HTML += FString::Printf(TEXT("<div class=\"summary-box %s\">\n"), IsValid() ? TEXT("passed") : TEXT("failed"));
	HTML += TEXT("<h3>Status</h3>\n");
	HTML += FString::Printf(TEXT("<div class=\"count\">%s</div>\n"), IsValid() ? TEXT("PASSED") : TEXT("FAILED"));
	HTML += TEXT("</div>\n");
	
	HTML += TEXT("<div class=\"summary-box\">\n<h3>Critical</h3>\n");
	HTML += FString::Printf(TEXT("<div class=\"count\" style=\"color: #721c24;\">%d</div>\n"), CriticalCount);
	HTML += TEXT("</div>\n");
	
	HTML += TEXT("<div class=\"summary-box\">\n<h3>Errors</h3>\n");
	HTML += FString::Printf(TEXT("<div class=\"count\" style=\"color: #dc3545;\">%d</div>\n"), ErrorCount);
	HTML += TEXT("</div>\n");
	
	HTML += TEXT("<div class=\"summary-box\">\n<h3>Warnings</h3>\n");
	HTML += FString::Printf(TEXT("<div class=\"count\" style=\"color: #ffc107;\">%d</div>\n"), WarningCount);
	HTML += TEXT("</div>\n");
	
	HTML += TEXT("<div class=\"summary-box\">\n<h3>Info</h3>\n");
	HTML += FString::Printf(TEXT("<div class=\"count\" style=\"color: #17a2b8;\">%d</div>\n"), InfoCount);
	HTML += TEXT("</div>\n");
	HTML += TEXT("</div>\n");
	
	// Issues sections
	auto AddIssueSection = [&](EValidationSeverity Severity, const FString& Title, const FString& CSSClass)
	{
		TArray<const FValidationIssue*> FilteredIssues;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == Severity)
			{
				FilteredIssues.Add(&Issue);
			}
		}
		
		if (FilteredIssues.Num() > 0)
		{
			HTML += FString::Printf(TEXT("<h2>%s (%d)</h2>\n"), *Title, FilteredIssues.Num());
			for (int32 i = 0; i < FilteredIssues.Num(); ++i)
			{
				const FValidationIssue* Issue = FilteredIssues[i];
				HTML += FString::Printf(TEXT("<div class=\"issue %s\">\n"), *CSSClass);
				HTML += FString::Printf(TEXT("<div class=\"issue-header\">%d. %s</div>\n"), i + 1, *Issue->Message);
				
				if (!Issue->SourceFile.IsEmpty())
				{
					HTML += FString::Printf(TEXT("<div class=\"issue-source\">Source: %s:%d (%s)</div>\n"),
						*Issue->SourceFile, Issue->SourceLine, *Issue->SourceFunction);
				}
				
				if (Issue->Metadata.Num() > 0)
				{
					HTML += TEXT("<div class=\"issue-metadata\">Metadata: ");
					for (const auto& Pair : Issue->Metadata)
					{
						HTML += FString::Printf(TEXT("%s=%s; "), *Pair.Key, *Pair.Value);
					}
					HTML += TEXT("</div>\n");
				}
				
				HTML += TEXT("</div>\n");
			}
		}
	};
	
	AddIssueSection(EValidationSeverity::Critical, TEXT("Critical Issues"), TEXT("critical"));
	AddIssueSection(EValidationSeverity::Error, TEXT("Errors"), TEXT("error"));
	AddIssueSection(EValidationSeverity::Warning, TEXT("Warnings"), TEXT("warning"));
	AddIssueSection(EValidationSeverity::Info, TEXT("Info"), TEXT("info"));
	
	// Child contexts
	if (ChildContexts.Num() > 0)
	{
		HTML += FString::Printf(TEXT("<h2>Nested Contexts (%d)</h2>\n"), ChildContexts.Num());
		for (int32 i = 0; i < ChildContexts.Num(); ++i)
		{
			HTML += FString::Printf(TEXT("<button class=\"collapsible\" onclick=\"toggleSection('child%d')\">Child Context %d: %s - %s</button>\n"),
				i, i + 1, *ChildContexts[i].SystemName, *ChildContexts[i].OperationName);
			HTML += FString::Printf(TEXT("<div id=\"child%d\" class=\"content\">\n"), i);
			HTML += TEXT("<div class=\"child-context\">\n");
			HTML += ChildContexts[i].GetReportHTML();
			HTML += TEXT("</div>\n</div>\n");
		}
	}
	
	HTML += TEXT("</div>\n"); // Close container
	HTML += TEXT("</body>\n</html>");
	
	return HTML;
}

void FValidationContext::Reset()
{
	SystemName.Empty();
	OperationName.Empty();
	ValidationErrors.Empty();
	ValidationWarnings.Empty();
	Issues.Empty();
	ChildContexts.Empty();
	CreationTime = FDateTime::Now();
	CompletionTime = FDateTime::MinValue();
}
