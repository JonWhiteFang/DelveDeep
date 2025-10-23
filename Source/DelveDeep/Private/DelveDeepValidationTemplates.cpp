// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidationTemplates.h"

namespace DelveDeepValidation
{
	bool ValidateString(const FString& Value, const FString& PropertyName,
						FValidationContext& Context, int32 MinLength, 
						int32 MaxLength, bool bAllowEmpty)
	{
		bool bIsValid = true;
		
		// Check if empty
		if (!bAllowEmpty && Value.IsEmpty())
		{
			Context.AddError(FString::Printf(TEXT("%s is empty"), *PropertyName));
			bIsValid = false;
		}
		
		// Check length constraints
		int32 Length = Value.Len();
		if (Length < MinLength)
		{
			Context.AddError(FString::Printf(
				TEXT("%s is too short: %d characters (minimum: %d)"),
				*PropertyName, Length, MinLength));
			bIsValid = false;
		}
		
		if (Length > MaxLength)
		{
			Context.AddError(FString::Printf(
				TEXT("%s is too long: %d characters (maximum: %d)"),
				*PropertyName, Length, MaxLength));
			bIsValid = false;
		}
		
		return bIsValid;
	}

	bool ValidateText(const FText& Value, const FString& PropertyName,
					  FValidationContext& Context, bool bAllowEmpty)
	{
		if (!bAllowEmpty && Value.IsEmpty())
		{
			Context.AddError(FString::Printf(TEXT("%s is empty"), *PropertyName));
			return false;
		}
		return true;
	}

	bool ValidateName(const FName& Value, const FString& PropertyName,
					  FValidationContext& Context, bool bAllowNone)
	{
		if (!bAllowNone && Value == NAME_None)
		{
			Context.AddError(FString::Printf(TEXT("%s is None"), *PropertyName));
			return false;
		}
		return true;
	}
}
