// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepValidation.h"

/**
 * Namespace containing template validation functions for common validation scenarios.
 * These templates provide reusable validation logic with automatic error message generation.
 */
namespace DelveDeepValidation
{
	/**
	 * Validates that a numeric value is within a specified range.
	 * 
	 * @param Value The value to validate
	 * @param Min The minimum allowed value (inclusive)
	 * @param Max The maximum allowed value (inclusive)
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the value is within range, false otherwise
	 */
	template<typename T>
	bool ValidateRange(T Value, T Min, T Max, const FString& PropertyName, FValidationContext& Context)
	{
		if (Value < Min || Value > Max)
		{
			Context.AddError(FString::Printf(
				TEXT("%s out of range: %s (expected %s-%s)"),
				*PropertyName, 
				*LexToString(Value), 
				*LexToString(Min), 
				*LexToString(Max)));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a UObject pointer is not null and is valid.
	 * 
	 * @param Pointer The pointer to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param bAllowNull If true, null pointers are considered valid
	 * @return True if the pointer is valid (or null if allowed), false otherwise
	 */
	template<typename T>
	bool ValidatePointer(const T* Pointer, const FString& PropertyName, FValidationContext& Context, bool bAllowNull = false)
	{
		if (!bAllowNull && !Pointer)
		{
			Context.AddError(FString::Printf(TEXT("%s is null"), *PropertyName));
			return false;
		}
		
		if (Pointer && !IsValid(Pointer))
		{
			Context.AddError(FString::Printf(TEXT("%s is invalid (pending kill or garbage)"), *PropertyName));
			return false;
		}
		
		return true;
	}

	/**
	 * Validates that a TSoftObjectPtr reference is not null.
	 * 
	 * @param SoftPtr The soft object pointer to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param bAllowNull If true, null references are considered valid
	 * @return True if the reference is not null (or null if allowed), false otherwise
	 */
	template<typename T>
	bool ValidateSoftReference(const TSoftObjectPtr<T>& SoftPtr, const FString& PropertyName, 
							   FValidationContext& Context, bool bAllowNull = false)
	{
		if (!bAllowNull && SoftPtr.IsNull())
		{
			Context.AddError(FString::Printf(TEXT("%s soft reference is null"), *PropertyName));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a TSoftClassPtr reference is not null.
	 * 
	 * @param SoftClassPtr The soft class pointer to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param bAllowNull If true, null references are considered valid
	 * @return True if the reference is not null (or null if allowed), false otherwise
	 */
	template<typename T>
	bool ValidateSoftClassReference(const TSoftClassPtr<T>& SoftClassPtr, const FString& PropertyName,
									FValidationContext& Context, bool bAllowNull = false)
	{
		if (!bAllowNull && SoftClassPtr.IsNull())
		{
			Context.AddError(FString::Printf(TEXT("%s soft class reference is null"), *PropertyName));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a string meets length requirements and is not empty.
	 * 
	 * @param Value The string to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param MinLength The minimum allowed length (default: 0)
	 * @param MaxLength The maximum allowed length (default: MAX_int32)
	 * @param bAllowEmpty If false, empty strings are considered invalid
	 * @return True if the string meets all requirements, false otherwise
	 */
	DELVEDEEP_API bool ValidateString(const FString& Value, const FString& PropertyName,
									  FValidationContext& Context, int32 MinLength = 0, 
									  int32 MaxLength = MAX_int32, bool bAllowEmpty = false);

	/**
	 * Validates that an FText is not empty.
	 * 
	 * @param Value The FText to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param bAllowEmpty If false, empty text is considered invalid
	 * @return True if the text is not empty (or empty if allowed), false otherwise
	 */
	DELVEDEEP_API bool ValidateText(const FText& Value, const FString& PropertyName,
									FValidationContext& Context, bool bAllowEmpty = false);

	/**
	 * Validates that an FName is not None.
	 * 
	 * @param Value The FName to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param bAllowNone If false, NAME_None is considered invalid
	 * @return True if the name is not None (or None if allowed), false otherwise
	 */
	DELVEDEEP_API bool ValidateName(const FName& Value, const FString& PropertyName,
									FValidationContext& Context, bool bAllowNone = false);

	/**
	 * Validates that an array size is within specified bounds.
	 * 
	 * @param Array The array to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @param MinSize The minimum allowed size (default: 0)
	 * @param MaxSize The maximum allowed size (default: MAX_int32)
	 * @return True if the array size is within bounds, false otherwise
	 */
	template<typename T>
	bool ValidateArraySize(const TArray<T>& Array, const FString& PropertyName,
						   FValidationContext& Context, int32 MinSize = 0, 
						   int32 MaxSize = MAX_int32)
	{
		int32 Size = Array.Num();
		if (Size < MinSize || Size > MaxSize)
		{
			Context.AddError(FString::Printf(
				TEXT("%s array size out of range: %d (expected %d-%d)"),
				*PropertyName, Size, MinSize, MaxSize));
			return false;
		}
		return true;
	}

	/**
	 * Validates that an array is not empty.
	 * 
	 * @param Array The array to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the array is not empty, false otherwise
	 */
	template<typename T>
	bool ValidateArrayNotEmpty(const TArray<T>& Array, const FString& PropertyName, FValidationContext& Context)
	{
		if (Array.Num() == 0)
		{
			Context.AddError(FString::Printf(TEXT("%s array is empty"), *PropertyName));
			return false;
		}
		return true;
	}

	/**
	 * Validates that an enum value is within the valid range defined by the enum.
	 * 
	 * @param Value The enum value to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the enum value is valid, false otherwise
	 */
	template<typename T>
	bool ValidateEnum(T Value, const FString& PropertyName, FValidationContext& Context)
	{
		UEnum* EnumClass = StaticEnum<T>();
		if (!EnumClass || !EnumClass->IsValidEnumValue(static_cast<int64>(Value)))
		{
			Context.AddError(FString::Printf(
				TEXT("%s has invalid enum value: %d"), 
				*PropertyName, static_cast<int32>(Value)));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a map is not empty.
	 * 
	 * @param Map The map to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the map is not empty, false otherwise
	 */
	template<typename KeyType, typename ValueType>
	bool ValidateMapNotEmpty(const TMap<KeyType, ValueType>& Map, const FString& PropertyName, FValidationContext& Context)
	{
		if (Map.Num() == 0)
		{
			Context.AddError(FString::Printf(TEXT("%s map is empty"), *PropertyName));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a set is not empty.
	 * 
	 * @param Set The set to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the set is not empty, false otherwise
	 */
	template<typename T>
	bool ValidateSetNotEmpty(const TSet<T>& Set, const FString& PropertyName, FValidationContext& Context)
	{
		if (Set.Num() == 0)
		{
			Context.AddError(FString::Printf(TEXT("%s set is empty"), *PropertyName));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a value is positive (greater than zero).
	 * 
	 * @param Value The value to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the value is positive, false otherwise
	 */
	template<typename T>
	bool ValidatePositive(T Value, const FString& PropertyName, FValidationContext& Context)
	{
		if (Value <= static_cast<T>(0))
		{
			Context.AddError(FString::Printf(
				TEXT("%s must be positive: %s"),
				*PropertyName, *LexToString(Value)));
			return false;
		}
		return true;
	}

	/**
	 * Validates that a value is non-negative (greater than or equal to zero).
	 * 
	 * @param Value The value to validate
	 * @param PropertyName The name of the property being validated (for error messages)
	 * @param Context The validation context to add issues to
	 * @return True if the value is non-negative, false otherwise
	 */
	template<typename T>
	bool ValidateNonNegative(T Value, const FString& PropertyName, FValidationContext& Context)
	{
		if (Value < static_cast<T>(0))
		{
			Context.AddError(FString::Printf(
				TEXT("%s must be non-negative: %s"),
				*PropertyName, *LexToString(Value)));
			return false;
		}
		return true;
	}
}
