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
struct DELVEDEEP_API FDelveDeepValidationContext
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

	/** Child validation contexts for nested validation (not exposed to Blueprint due to recursion) */
	TArray<FDelveDeepValidationContext> ChildContexts;

	/**
	 * Constructor - initializes creation time
	 */
	FDelveDeepValidationContext()
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
	 * Adds a child validation context for nested validation.
	 * @param ChildContext The child context to add
	 */
	void AddChildContext(const FDelveDeepValidationContext& ChildContext);

	/**
	 * Merges another validation context into this one.
	 * @param OtherContext The context to merge
	 */
	void MergeContext(const FDelveDeepValidationContext& OtherContext);

	/**
	 * Attaches metadata to the most recent issue.
	 * @param Key The metadata key
	 * @param Value The metadata value
	 */
	void AttachMetadata(const FString& Key, const FString& Value);

	/**
	 * Gets the validation duration (time between creation and completion).
	 * @return The validation duration as a timespan
	 */
	FTimespan GetValidationDuration() const;

	/**
	 * Generates a formatted validation report with all errors and warnings.
	 * Groups issues by severity with visual distinction and includes nested contexts.
	 * @return A formatted string containing the validation report
	 */
	FString GetReport() const;

	/**
	 * Generates a JSON-formatted validation report with all context data.
	 * @return A JSON string containing the validation report
	 */
	FString GetReportJSON() const;

	/**
	 * Generates a CSV-formatted validation report for spreadsheet analysis.
	 * @return A CSV string containing the validation report
	 */
	FString GetReportCSV() const;

	/**
	 * Generates an HTML-formatted validation report with color coding and interactivity.
	 * @return An HTML string containing the validation report
	 */
	FString GetReportHTML() const;

	/**
	 * Resets the validation context, clearing all errors and warnings.
	 */
	void Reset();

private:
	/**
	 * Helper function to generate report for nested contexts with indentation.
	 * @param IndentLevel The current indentation level
	 * @return A formatted string for nested contexts
	 */
	FString GetNestedReport(int32 IndentLevel = 0) const;
};

/**
 * Delegate signature for validation rules.
 * @param Object The object being validated
 * @param Context The validation context to populate with issues
 * @return True if validation passed, false otherwise
 */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FValidationRuleDelegate, const UObject*, FDelveDeepValidationContext&);

/**
 * Definition of a validation rule with metadata.
 */
USTRUCT()
struct DELVEDEEP_API FValidationRuleDefinition
{
	GENERATED_BODY()

	/** Unique name for this validation rule */
	FName RuleName;

	/** The class this rule applies to */
	UClass* TargetClass = nullptr;

	/** The validation function to execute */
	FValidationRuleDelegate ValidationDelegate;

	/** Priority for rule execution (higher priority runs first) */
	int32 Priority = 0;

	/** Human-readable description of what this rule validates */
	FString Description;

	/** Constructor */
	FValidationRuleDefinition()
		: RuleName(NAME_None)
		, TargetClass(nullptr)
		, Priority(0)
	{
	}

	/** Constructor with parameters */
	FValidationRuleDefinition(FName InRuleName, UClass* InTargetClass, FValidationRuleDelegate InDelegate, int32 InPriority = 0, const FString& InDescription = TEXT(""))
		: RuleName(InRuleName)
		, TargetClass(InTargetClass)
		, ValidationDelegate(InDelegate)
		, Priority(InPriority)
		, Description(InDescription)
	{
	}

	/** Comparison operator for sorting by priority */
	bool operator<(const FValidationRuleDefinition& Other) const
	{
		return Priority > Other.Priority; // Higher priority first
	}
};

/**
 * Cache entry for validation results.
 */
USTRUCT()
struct DELVEDEEP_API FValidationCacheEntry
{
	GENERATED_BODY()

	/** The cached validation context */
	FDelveDeepValidationContext Context;

	/** Timestamp when this entry was cached */
	FDateTime Timestamp;

	/** Hash of the asset at the time of validation */
	uint32 AssetHash = 0;

	/** Constructor */
	FValidationCacheEntry()
		: Timestamp(FDateTime::Now())
		, AssetHash(0)
	{
	}
};

/**
 * Blueprint-accessible validation metrics data.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FValidationMetricsData
{
	GENERATED_BODY()

	/** Total number of validations performed */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalValidations = 0;

	/** Number of validations that passed */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 PassedValidations = 0;

	/** Number of validations that failed */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 FailedValidations = 0;

	/** Frequency of each unique error message */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	TMap<FString, int32> ErrorFrequency;

	/** Average execution time per rule (in milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	TMap<FString, float> AverageRuleExecutionTime;

	/** Average execution time per system (in milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	TMap<FString, float> AverageSystemExecutionTime;

	/** Timestamp when metrics were last reset */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	FDateTime LastResetTime;
};

