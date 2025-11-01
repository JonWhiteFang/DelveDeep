// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepValidation.h"

/**
 * Validation rule templates for common validation scenarios.
 * Provides reusable validation functions with automatic error message generation.
 */
namespace DelveDeepValidation
{
	/**
	 * Validates that a numeric value is within a specified range.
	 * @param Value The value to validate
	 * @param Min Minimum allowed value (inclusive)
	 * @param Max Maximum allowed value (inclusive)
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if value is within range, false otherwise
	 */
	template<typename T>
	bool ValidateRange(T Value, T Min, T Max, const FString& PropertyName, 
					  FDelveDeepValidationContext& Context, EValidationSeverity Severity = EValidationSeverity::Error)
	{
		if (Value < Min || Value > Max)
		{
			Context.AddIssue(Severity, FString::Printf(
				TEXT("%s out of range: %s (expected %s-%s)"),
				*PropertyName, *LexToString(Value), *LexToString(Min), *LexToString(Max)));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a UObject pointer is not null and is valid.
	 * @param Pointer The pointer to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param bAllowNull If true, null pointers are considered valid
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if pointer is valid, false otherwise
	 */
	template<typename T>
	bool ValidatePointer(const T* Pointer, const FString& PropertyName, 
						FDelveDeepValidationContext& Context, bool bAllowNull = false,
						EValidationSeverity Severity = EValidationSeverity::Error)
	{
		if (!bAllowNull && !Pointer)
		{
			Context.AddIssue(Severity, FString::Printf(TEXT("%s is null"), *PropertyName));
			return false;
		}
		
		if (Pointer && !IsValid(Pointer))
		{
			Context.AddIssue(Severity, FString::Printf(TEXT("%s is invalid"), *PropertyName));
			return false;
		}
		
		return true;
	}

	/**
	 * Validates that a TSoftObjectPtr reference is not null.
	 * @param SoftPtr The soft pointer to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param bAllowNull If true, null references are considered valid
	 * @param Severity Severity level for validation failures (default: Warning)
	 * @return True if reference is valid, false otherwise
	 */
	template<typename T>
	bool ValidateSoftReference(const TSoftObjectPtr<T>& SoftPtr, const FString& PropertyName,
							  FDelveDeepValidationContext& Context, bool bAllowNull = false,
							  EValidationSeverity Severity = EValidationSeverity::Warning)
	{
		if (!bAllowNull && SoftPtr.IsNull())
		{
			Context.AddIssue(Severity, FString::Printf(
				TEXT("%s soft reference is null"), *PropertyName));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a string meets length requirements.
	 * @param Value The string to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param MinLength Minimum allowed length (default: 0)
	 * @param MaxLength Maximum allowed length (default: MAX_int32)
	 * @param bAllowEmpty If false, empty strings are invalid (default: false)
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if string is valid, false otherwise
	 */
	inline bool ValidateString(const FString& Value, const FString& PropertyName,
							  FDelveDeepValidationContext& Context, int32 MinLength = 0, 
							  int32 MaxLength = MAX_int32, bool bAllowEmpty = false,
							  EValidationSeverity Severity = EValidationSeverity::Error)
	{
		if (!bAllowEmpty && Value.IsEmpty())
		{
			Context.AddIssue(Severity, FString::Printf(TEXT("%s is empty"), *PropertyName));
			return false;
		}
		
		int32 Length = Value.Len();
		if (Length < MinLength || Length > MaxLength)
		{
			Context.AddIssue(Severity, FString::Printf(
				TEXT("%s length out of range: %d (expected %d-%d)"),
				*PropertyName, Length, MinLength, MaxLength));
			return false;
		}
		
		return true;
	}

	/**
	 * Validates that an array size is within specified bounds.
	 * @param Array The array to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param MinSize Minimum allowed size (default: 0)
	 * @param MaxSize Maximum allowed size (default: MAX_int32)
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if array size is valid, false otherwise
	 */
	template<typename T>
	bool ValidateArraySize(const TArray<T>& Array, const FString& PropertyName,
						  FDelveDeepValidationContext& Context, int32 MinSize = 0, 
						  int32 MaxSize = MAX_int32,
						  EValidationSeverity Severity = EValidationSeverity::Error)
	{
		int32 Size = Array.Num();
		if (Size < MinSize || Size > MaxSize)
		{
			Context.AddIssue(Severity, FString::Printf(
				TEXT("%s array size out of range: %d (expected %d-%d)"),
				*PropertyName, Size, MinSize, MaxSize));
			return false;
		}
		return true;
	}

	/**
	 * Validates that an enum value is within valid range.
	 * @param Value The enum value to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if enum value is valid, false otherwise
	 */
	template<typename T>
	bool ValidateEnum(T Value, const FString& PropertyName, 
					 FDelveDeepValidationContext& Context,
					 EValidationSeverity Severity = EValidationSeverity::Error)
	{
		UEnum* EnumClass = StaticEnum<T>();
		if (!EnumClass || !EnumClass->IsValidEnumValue(static_cast<int64>(Value)))
		{
			Context.AddIssue(Severity, FString::Printf(
				TEXT("%s has invalid enum value: %d"), 
				*PropertyName, static_cast<int32>(Value)));
			return false;
		}
		return true;
	}

	/**
	 * Validates that an FText value is not empty.
	 * @param Value The FText to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param bAllowEmpty If true, empty text is considered valid (default: false)
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if text is valid, false otherwise
	 */
	bool ValidateText(const FText& Value, const FString& PropertyName,
					  FDelveDeepValidationContext& Context, bool bAllowEmpty = false,
					  EValidationSeverity Severity = EValidationSeverity::Error);

	/**
	 * Validates that an FName value is not None.
	 * @param Value The FName to validate
	 * @param PropertyName Name of the property being validated
	 * @param Context Validation context to add issues to
	 * @param bAllowNone If true, NAME_None is considered valid (default: false)
	 * @param Severity Severity level for validation failures (default: Error)
	 * @return True if name is valid, false otherwise
	 */
	bool ValidateName(const FName& Value, const FString& PropertyName,
					  FDelveDeepValidationContext& Context, bool bAllowNone = false,
					  EValidationSeverity Severity = EValidationSeverity::Error);
}
