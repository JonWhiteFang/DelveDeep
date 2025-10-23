// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"

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

FString FValidationContext::GetReport() const
{
	FString Report;
	
	// Add header
	Report += FString::Printf(TEXT("=== Validation Report ===\n"));
	Report += FString::Printf(TEXT("System: %s\n"), *SystemName);
	Report += FString::Printf(TEXT("Operation: %s\n\n"), *OperationName);
	
	// Group issues by severity
	int32 CriticalCount = GetIssueCount(EValidationSeverity::Critical);
	int32 ErrorCount = GetIssueCount(EValidationSeverity::Error);
	int32 WarningCount = GetIssueCount(EValidationSeverity::Warning);
	int32 InfoCount = GetIssueCount(EValidationSeverity::Info);
	
	// Add critical issues section
	if (CriticalCount > 0)
	{
		Report += FString::Printf(TEXT("CRITICAL ISSUES (%d):\n"), CriticalCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Critical)
			{
				Report += FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (!Issue.SourceFile.IsEmpty())
				{
					Report += FString::Printf(TEXT("     Source: %s:%d (%s)\n"), 
						*Issue.SourceFile, Issue.SourceLine, *Issue.SourceFunction);
				}
			}
		}
		Report += TEXT("\n");
	}
	
	// Add errors section
	if (ErrorCount > 0)
	{
		Report += FString::Printf(TEXT("ERRORS (%d):\n"), ErrorCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Error)
			{
				Report += FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (!Issue.SourceFile.IsEmpty())
				{
					Report += FString::Printf(TEXT("     Source: %s:%d (%s)\n"), 
						*Issue.SourceFile, Issue.SourceLine, *Issue.SourceFunction);
				}
			}
		}
		Report += TEXT("\n");
	}
	else if (CriticalCount == 0)
	{
		Report += TEXT("No errors found.\n\n");
	}
	
	// Add warnings section
	if (WarningCount > 0)
	{
		Report += FString::Printf(TEXT("WARNINGS (%d):\n"), WarningCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Warning)
			{
				Report += FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
				if (!Issue.SourceFile.IsEmpty())
				{
					Report += FString::Printf(TEXT("     Source: %s:%d (%s)\n"), 
						*Issue.SourceFile, Issue.SourceLine, *Issue.SourceFunction);
				}
			}
		}
		Report += TEXT("\n");
	}
	else
	{
		Report += TEXT("No warnings found.\n\n");
	}
	
	// Add info section
	if (InfoCount > 0)
	{
		Report += FString::Printf(TEXT("INFO (%d):\n"), InfoCount);
		int32 Index = 1;
		for (const FValidationIssue& Issue : Issues)
		{
			if (Issue.Severity == EValidationSeverity::Info)
			{
				Report += FString::Printf(TEXT("  %d. %s\n"), Index++, *Issue.Message);
			}
		}
		Report += TEXT("\n");
	}
	
	// Add summary
	Report += FString::Printf(TEXT("=== Summary ===\n"));
	Report += FString::Printf(TEXT("Status: %s\n"), IsValid() ? TEXT("PASSED") : TEXT("FAILED"));
	Report += FString::Printf(TEXT("Critical Issues: %d\n"), CriticalCount);
	Report += FString::Printf(TEXT("Errors: %d\n"), ErrorCount);
	Report += FString::Printf(TEXT("Warnings: %d\n"), WarningCount);
	Report += FString::Printf(TEXT("Info: %d\n"), InfoCount);
	
	return Report;
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
