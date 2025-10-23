// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepValidation.generated.h"

// Logging category for DelveDeep configuration system
DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All);

/**
 * Validation context for tracking errors and warnings during data validation.
 * Provides comprehensive error reporting with system and operation context.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FValidationContext
{
	GENERATED_BODY()

public:
	/** Name of the system performing validation */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString SystemName;

	/** Name of the specific operation being validated */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString OperationName;

	/** Collection of validation errors (critical issues) */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> ValidationErrors;

	/** Collection of validation warnings (potential issues) */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> ValidationWarnings;

	/**
	 * Adds an error to the validation context.
	 * @param Error The error message to add
	 */
	void AddError(const FString& Error);

	/**
	 * Adds a warning to the validation context.
	 * @param Warning The warning message to add
	 */
	void AddWarning(const FString& Warning);

	/**
	 * Checks if the validation context has no errors.
	 * @return True if there are no validation errors, false otherwise
	 */
	bool IsValid() const { return ValidationErrors.Num() == 0; }

	/**
	 * Generates a formatted validation report with all errors and warnings.
	 * @return A formatted string containing the validation report
	 */
	FString GetReport() const;

	/**
	 * Resets the validation context, clearing all errors and warnings.
	 */
	void Reset();
};
