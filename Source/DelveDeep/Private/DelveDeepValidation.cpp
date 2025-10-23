// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"

DEFINE_LOG_CATEGORY(LogDelveDeepConfig);

void FValidationContext::AddError(const FString& Error)
{
	ValidationErrors.Add(Error);
	UE_LOG(LogDelveDeepConfig, Error, TEXT("[%s::%s] Error: %s"), 
		*SystemName, *OperationName, *Error);
}

void FValidationContext::AddWarning(const FString& Warning)
{
	ValidationWarnings.Add(Warning);
	UE_LOG(LogDelveDeepConfig, Warning, TEXT("[%s::%s] Warning: %s"), 
		*SystemName, *OperationName, *Warning);
}

FString FValidationContext::GetReport() const
{
	FString Report;
	
	// Add header
	Report += FString::Printf(TEXT("=== Validation Report ===\n"));
	Report += FString::Printf(TEXT("System: %s\n"), *SystemName);
	Report += FString::Printf(TEXT("Operation: %s\n\n"), *OperationName);
	
	// Add errors section
	if (ValidationErrors.Num() > 0)
	{
		Report += FString::Printf(TEXT("Errors (%d):\n"), ValidationErrors.Num());
		for (int32 i = 0; i < ValidationErrors.Num(); ++i)
		{
			Report += FString::Printf(TEXT("  %d. %s\n"), i + 1, *ValidationErrors[i]);
		}
		Report += TEXT("\n");
	}
	else
	{
		Report += TEXT("No errors found.\n\n");
	}
	
	// Add warnings section
	if (ValidationWarnings.Num() > 0)
	{
		Report += FString::Printf(TEXT("Warnings (%d):\n"), ValidationWarnings.Num());
		for (int32 i = 0; i < ValidationWarnings.Num(); ++i)
		{
			Report += FString::Printf(TEXT("  %d. %s\n"), i + 1, *ValidationWarnings[i]);
		}
		Report += TEXT("\n");
	}
	else
	{
		Report += TEXT("No warnings found.\n\n");
	}
	
	// Add summary
	Report += FString::Printf(TEXT("=== Summary ===\n"));
	Report += FString::Printf(TEXT("Status: %s\n"), IsValid() ? TEXT("PASSED") : TEXT("FAILED"));
	Report += FString::Printf(TEXT("Total Errors: %d\n"), ValidationErrors.Num());
	Report += FString::Printf(TEXT("Total Warnings: %d\n"), ValidationWarnings.Num());
	
	return Report;
}

void FValidationContext::Reset()
{
	SystemName.Empty();
	OperationName.Empty();
	ValidationErrors.Empty();
	ValidationWarnings.Empty();
}
