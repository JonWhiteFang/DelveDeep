// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Validation System Examples
 * 
 * This file contains comprehensive examples of using the DelveDeep validation system.
 * These examples demonstrate best practices and common usage patterns.
 * 
 * NOTE: These are example functions for documentation purposes.
 * They are not compiled into the game by default.
 */

#if 0 // Set to 1 to compile examples

#include "CoreMinimal.h"
#include "DelveDeepValidation.h"
#include "DelveDeepValidationSubsystem.h"
#include "DelveDeepValidationTemplates.h"
#include "DelveDeepValidationInterface.h"
#include "DelveDeepCharacterData.h"
#include "Engine/GameInstance.h"

// ============================================================================
// Example 1: Basic Validation with Severity Levels
// ============================================================================

/**
 * Demonstrates basic validation using different severity levels.
 * Shows how to add issues and check validation status.
 */
void Example_BasicValidationWithSeverity()
{
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Example");
	Context.OperationName = TEXT("BasicValidation");

	// Add different severity levels
	Context.AddCritical(TEXT("Critical issue - asset is corrupted"));
	Context.AddError(TEXT("Error - invalid configuration"));
	Context.AddWarning(TEXT("Warning - suboptimal setting"));
	Context.AddInfo(TEXT("Info - validation started"));

	// Check for specific severity levels
	if (Context.HasCriticalIssues())
	{
		UE_LOG(LogTemp, Error, TEXT("Critical issues detected!"));
	}

	if (Context.HasErrors())
	{
		UE_LOG(LogTemp, Error, TEXT("Errors detected!"));
	}

	if (Context.HasWarnings())
	{
		UE_LOG(LogTemp, Warning, TEXT("Warnings detected!"));
	}

	// Check overall validation status
	if (!Context.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Validation failed:\n%s"), *Context.GetReport());
	}

	// Get issue counts
	int32 CriticalCount = Context.GetIssueCount(EValidationSeverity::Critical);
	int32 ErrorCount = Context.GetIssueCount(EValidationSeverity::Error);
	int32 WarningCount = Context.GetIssueCount(EValidationSeverity::Warning);
	int32 InfoCount = Context.GetIssueCount(EValidationSeverity::Info);

	UE_LOG(LogTemp, Display, TEXT("Issues: %d critical, %d errors, %d warnings, %d info"),
		CriticalCount, ErrorCount, WarningCount, InfoCount);
}

// ============================================================================
// Example 2: Using Validation Templates
// ============================================================================

/**
 * Demonstrates using validation templates for common validation scenarios.
 * Templates provide consistent error messages and reduce boilerplate code.
 */
bool Example_ValidationTemplates(const UDelveDeepCharacterData* CharacterData)
{
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Example");
	Context.OperationName = TEXT("TemplateValidation");

	bool bIsValid = true;

	// Validate numeric ranges
	bIsValid &= DelveDeepValidation::ValidateRange(
		CharacterData->BaseHealth, 1.0f, 10000.0f, TEXT("BaseHealth"), Context);

	bIsValid &= DelveDeepValidation::ValidateRange(
		CharacterData->BaseDamage, 1.0f, 1000.0f, TEXT("BaseDamage"), Context);

	bIsValid &= DelveDeepValidation::ValidateRange(
		CharacterData->MoveSpeed, 50.0f, 1000.0f, TEXT("MoveSpeed"), Context);

	// Validate pointer (not null)
	bIsValid &= DelveDeepValidation::ValidatePointer(
		CharacterData, TEXT("CharacterData"), Context, false);

	// Validate soft reference (allow null, warning severity)
	DelveDeepValidation::ValidateSoftReference(
		CharacterData->StartingWeapon, TEXT("StartingWeapon"), Context, 
		true, EValidationSeverity::Warning);

	// Validate string
	FString CharacterName = CharacterData->CharacterName.ToString();
	bIsValid &= DelveDeepValidation::ValidateString(
		CharacterName, TEXT("CharacterName"), Context, 1, 100, false);

	// Validate array size
	bIsValid &= DelveDeepValidation::ValidateArraySize(
		CharacterData->StartingAbilities, TEXT("StartingAbilities"), Context, 0, 10);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("Template validation failed:\n%s"), *Context.GetReport());
	}

	return bIsValid;
}

// ============================================================================
// Example 3: Custom Validation Rule Registration
// ============================================================================

/**
 * Demonstrates registering custom validation rules with the validation subsystem.
 * Rules are automatically applied when validating objects of the target type.
 */
void Example_RegisterCustomValidationRule(UGameInstance* GameInstance)
{
	// Get validation subsystem
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();

	if (!ValidationSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("Validation subsystem not available"));
		return;
	}

	// Register a simple validation rule
	ValidationSubsystem->RegisterValidationRule(
		FName(TEXT("ValidateCharacterBalance")),
		UDelveDeepCharacterData::StaticClass(),
		FValidationRuleDelegate::CreateLambda([](const UObject* Object, FDelveDeepValidationContext& Context) -> bool
		{
			const UDelveDeepCharacterData* CharacterData = Cast<UDelveDeepCharacterData>(Object);
			if (!CharacterData)
			{
				return false;
			}

			// Check if character is balanced (health to damage ratio)
			float HealthToDamageRatio = CharacterData->BaseHealth / CharacterData->BaseDamage;
			
			if (HealthToDamageRatio < 5.0f)
			{
				Context.AddWarning(FString::Printf(
					TEXT("Character may be too fragile (Health/Damage ratio: %.2f, expected > 5.0)"),
					HealthToDamageRatio));
			}
			else if (HealthToDamageRatio > 20.0f)
			{
				Context.AddWarning(FString::Printf(
					TEXT("Character may be too tanky (Health/Damage ratio: %.2f, expected < 20.0)"),
					HealthToDamageRatio));
			}

			return true; // Warnings don't fail validation
		}),
		50, // Medium priority
		TEXT("Validates character balance (health to damage ratio)")
	);

	UE_LOG(LogTemp, Display, TEXT("Registered custom validation rule"));
}

// ============================================================================
// Example 4: Using Validation Subsystem with Caching
// ============================================================================

/**
 * Demonstrates using the validation subsystem with caching for improved performance.
 * Shows cache hit/miss behavior and cache invalidation.
 */
void Example_ValidationWithCaching(UGameInstance* GameInstance, UDelveDeepCharacterData* CharacterData)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();

	if (!ValidationSubsystem)
	{
		return;
	}

	// First validation - cache miss, full validation
	{
		FDelveDeepValidationContext Context;
		double StartTime = FPlatformTime::Seconds();
		
		bool bIsValid = ValidationSubsystem->ValidateObjectWithCache(CharacterData, Context);
		
		double EndTime = FPlatformTime::Seconds();
		double Duration = (EndTime - StartTime) * 1000.0; // Convert to ms

		UE_LOG(LogTemp, Display, TEXT("First validation (cache miss): %.4f ms, Valid: %s"),
			Duration, bIsValid ? TEXT("Yes") : TEXT("No"));
	}

	// Second validation - cache hit, instant return
	{
		FDelveDeepValidationContext Context;
		double StartTime = FPlatformTime::Seconds();
		
		bool bIsValid = ValidationSubsystem->ValidateObjectWithCache(CharacterData, Context);
		
		double EndTime = FPlatformTime::Seconds();
		double Duration = (EndTime - StartTime) * 1000.0;

		UE_LOG(LogTemp, Display, TEXT("Second validation (cache hit): %.4f ms, Valid: %s"),
			Duration, bIsValid ? TEXT("Yes") : TEXT("No"));
	}

	// Modify object and invalidate cache
	CharacterData->BaseHealth = 150.0f;
	ValidationSubsystem->InvalidateCache(CharacterData);

	// Third validation - cache miss after invalidation
	{
		FDelveDeepValidationContext Context;
		double StartTime = FPlatformTime::Seconds();
		
		bool bIsValid = ValidationSubsystem->ValidateObjectWithCache(CharacterData, Context);
		
		double EndTime = FPlatformTime::Seconds();
		double Duration = (EndTime - StartTime) * 1000.0;

		UE_LOG(LogTemp, Display, TEXT("Third validation (cache invalidated): %.4f ms, Valid: %s"),
			Duration, bIsValid ? TEXT("Yes") : TEXT("No"));
	}
}

// ============================================================================
// Example 5: Validation Delegates
// ============================================================================

/**
 * Demonstrates using validation delegates to respond to validation events.
 * Shows pre-validation, post-validation, and critical issue delegates.
 */
void Example_ValidationDelegates(UGameInstance* GameInstance)
{
	UDelveDeepValidationSubsystem* ValidationSubsystem = 
		GameInstance->GetSubsystem<UDelveDeepValidationSubsystem>();

	if (!ValidationSubsystem)
	{
		return;
	}

	// Register pre-validation delegate
	FDelegateHandle PreValidationHandle = ValidationSubsystem->OnPreValidation.AddLambda(
		[](const UObject* Object, FDelveDeepValidationContext& Context)
		{
			UE_LOG(LogTemp, Display, TEXT("Pre-validation: About to validate %s"), 
				*Object->GetName());
			
			// Can modify context before validation
			Context.AttachMetadata(TEXT("ValidationStartTime"), 
				FString::Printf(TEXT("%.4f"), FPlatformTime::Seconds()));
		});

	// Register post-validation delegate
	FDelegateHandle PostValidationHandle = ValidationSubsystem->OnPostValidation.AddLambda(
		[](const UObject* Object, const FDelveDeepValidationContext& Context)
		{
			UE_LOG(LogTemp, Display, TEXT("Post-validation: Validated %s, Result: %s"), 
				*Object->GetName(), Context.IsValid() ? TEXT("PASSED") : TEXT("FAILED"));
			
			// Log validation duration
			FTimespan Duration = Context.GetValidationDuration();
			UE_LOG(LogTemp, Display, TEXT("  Duration: %.4f ms"), 
				Duration.GetTotalMilliseconds());
		});

	// Register critical issue delegate
	FDelegateHandle CriticalIssueHandle = ValidationSubsystem->OnCriticalIssue.AddLambda(
		[](const UObject* Object, const FValidationIssue& Issue)
		{
			UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUE in %s: %s"), 
				*Object->GetName(), *Issue.Message);
		});

	// Perform validation - delegates will fire
	UDelveDeepCharacterData* TestData = NewObject<UDelveDeepCharacterData>();
	TestData->BaseHealth = -10.0f; // Invalid value
	
	FDelveDeepValidationContext Context;
	ValidationSubsystem->ValidateObject(TestData, Context);

	// Unregister delegates when done
	ValidationSubsystem->OnPreValidation.Remove(PreValidationHandle);
	ValidationSubsystem->OnPostValidation.Remove(PostValidationHandle);
	ValidationSubsystem->OnCriticalIssue.Remove(CriticalIssueHandle);
}

// ============================================================================
// Example 6: Report Export Formats
// ============================================================================

/**
 * Demonstrates exporting validation reports in different formats.
 * Shows JSON, CSV, and HTML export for different use cases.
 */
void Example_ReportExportFormats()
{
	// Create a context with various issues
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("ExportExample");
	Context.OperationName = TEXT("GenerateReports");
	Context.AttachMetadata(TEXT("AssetPath"), TEXT("/Game/Data/Example"));

	Context.AddCritical(TEXT("Critical issue - asset corrupted"));
	Context.AddError(TEXT("Error - invalid health value"));
	Context.AddWarning(TEXT("Warning - suboptimal configuration"));
	Context.AddInfo(TEXT("Info - validation completed"));

	// Export as JSON
	FString JSONReport = Context.GetReportJSON();
	FFileHelper::SaveStringToFile(JSONReport, 
		TEXT("Saved/Validation/ExampleReport.json"));

	// Export as CSV
	FString CSVReport = Context.GetReportCSV();
	FFileHelper::SaveStringToFile(CSVReport, 
		TEXT("Saved/Validation/ExampleReport.csv"));

	// Export as HTML
	FString HTMLReport = Context.GetReportHTML();
	FFileHelper::SaveStringToFile(HTMLReport, 
		TEXT("Saved/Validation/ExampleReport.html"));
}

#endif // Example compilation guard
