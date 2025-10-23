// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepValidation.generated.h"

// Logging category for DelveDeep configuration system
DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepConfig, Log, All);

/**
 * Severity levels for validation issues.
 */
UENUM(BlueprintType)
enum class EValidationSeverity : uint8
{
	Critical    UMETA(DisplayName = "Critical"),  // Prevents operation, logs as Error
	Error       UMETA(DisplayName = "Error"),     // Prevents operation, logs as Error
	Warning     UMETA(DisplayName = "Warning"),   // Allows operation, logs as Warning
	Info        UMETA(DisplayName = "Info")       // Informational, logs as Display
};

/**
 * Represents a single validation issue with severity, message, source location, and metadata.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FValidationIssue
{
	GENERATED_BODY()

	/** Severity level of this validation issue */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	EValidationSeverity Severity = EValidationSeverity::Error;

	/** The validation issue message */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Message;

	/** Source file where the issue was added */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString SourceFile;

	/** Line number in the source file */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 SourceLine = 0;

	/** Function name where the issue was added */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString SourceFunction;

	/** Timestamp when the issue was added */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FDateTime Timestamp;

	/** Additional metadata for the issue (key-value pairs) */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TMap<FString, FString> Metadata;
};

/**
 * Validation context for tracking errors and warnings during data validation.
 * Provides comprehensive error reporting with system and operation context.
 * Enhanced with severity levels, source location tracking, and nested context support.
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

	/** Collection of validation errors (critical issues) - LEGACY, maintained for backward compatibility */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> ValidationErrors;

	/** Collection of validation warnings (potential issues) - LEGACY, maintained for backward compatibility */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> ValidationWarnings;

	/** Collection of all validation issues with severity levels */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FValidationIssue> Issues;

	/** Timestamp when the validation context was created */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FDateTime CreationTime;

	/** Timestamp when the validation was completed */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FDateTime CompletionTime;

	/** Child validation contexts for nested validation */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FValidationContext> ChildContexts;

	/**
	 * Constructor - initializes creation time
	 */
	FValidationContext()
	{
		CreationTime = FDateTime::Now();
	}

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
	 * Adds a validation issue with specified severity and source location.
	 * @param Severity The severity level of the issue
	 * @param Message The issue message
	 * @param SourceFile The source file where the issue was added (optional)
	 * @param SourceLine The line number in the source file (optional)
	 * @param SourceFunction The function name where the issue was added (optional)
	 */
	void AddIssue(EValidationSeverity Severity, const FString& Message, 
				  const FString& SourceFile = TEXT(""), int32 SourceLine = 0, 
				  const FString& SourceFunction = TEXT(""));

	/**
	 * Adds a critical issue to the validation context.
	 * @param Message The critical issue message
	 */
	void AddCritical(const FString& Message);

	/**
	 * Adds an informational message to the validation context.
	 * @param Message The informational message
	 */
	void AddInfo(const FString& Message);

	/**
	 * Checks if the validation context has any critical issues.
	 * @return True if there are critical issues, false otherwise
	 */
	bool HasCriticalIssues() const;

	/**
	 * Checks if the validation context has any errors.
	 * @return True if there are errors, false otherwise
	 */
	bool HasErrors() const;

	/**
	 * Checks if the validation context has any warnings.
	 * @return True if there are warnings, false otherwise
	 */
	bool HasWarnings() const;

	/**
	 * Gets the count of issues with a specific severity level.
	 * @param Severity The severity level to count
	 * @return The number of issues with the specified severity
	 */
	int32 GetIssueCount(EValidationSeverity Severity) const;

	/**
	 * Checks if the validation context has no errors or critical issues.
	 * @return True if there are no validation errors or critical issues, false otherwise
	 */
	bool IsValid() const;

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
