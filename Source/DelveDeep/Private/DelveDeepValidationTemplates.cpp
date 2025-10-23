// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationTemplates.h"

namespace DelveDeepValidation
{
	// Note: ValidateString is implemented as inline in the header file
	// This file contains additional helper functions

	bool ValidateText(const FText& Value, const FString& PropertyName,
					  FValidationContext& Context, bool bAllowEmpty,
					  EValidationSeverity Severity)
	{
		if (!bAllowEmpty && Value.IsEmpty())
		{
			Context.AddIssue(Severity, FString::Printf(TEXT("%s is empty"), *PropertyName));
			return false;
		}
		return true;
	}

	bool ValidateName(const FName& Value, const FString& PropertyName,
					  FValidationContext& Context, bool bAllowNone,
					  EValidationSeverity Severity)
	{
		if (!bAllowNone && Value == NAME_None)
		{
			Context.AddIssue(Severity, FString::Printf(TEXT("%s is None"), *PropertyName));
			return false;
		}
		return true;
	}
}
