// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepValidation.h"
#include "HAL/IConsoleManager.h"

/**
 * Console command to test the validation system
 */
static FAutoConsoleCommand TestValidationSystemCmd(
	TEXT("DelveDeep.TestValidationSystem"),
	TEXT("Tests the validation system by creating a sample validation context with errors and warnings"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Testing validation system..."));

		// Create a test validation context
		FValidationContext Context;
		Context.SystemName = TEXT("Configuration");
		Context.OperationName = TEXT("TestValidation");

		// Add some test errors
		Context.AddError(TEXT("Test error 1: Invalid health value (expected 1-10000, got -50)"));
		Context.AddError(TEXT("Test error 2: Missing required asset reference"));

		// Add some test warnings
		Context.AddWarning(TEXT("Test warning 1: Using default fallback value"));
		Context.AddWarning(TEXT("Test warning 2: Asset found outside standard directory"));

		// Generate and log the report
		FString Report = Context.GetReport();
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Report);

		// Test reset functionality
		Context.Reset();
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Context reset - IsValid: %s"), 
			Context.IsValid() ? TEXT("true") : TEXT("false"));
	})
);

/**
 * Console command to validate a sample data structure
 */
static FAutoConsoleCommand ValidateSampleDataCmd(
	TEXT("DelveDeep.ValidateSampleData"),
	TEXT("Validates sample data to demonstrate validation system usage"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Validating sample data..."));

		// Simulate validating character data
		FValidationContext Context;
		Context.SystemName = TEXT("Configuration");
		Context.OperationName = TEXT("ValidateCharacterData");

		// Simulate validation checks
		float BaseHealth = 100.0f;
		float BaseDamage = 10.0f;
		float MoveSpeed = 300.0f;

		// Validate health
		if (BaseHealth <= 0.0f || BaseHealth > 10000.0f)
		{
			Context.AddError(FString::Printf(
				TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), BaseHealth));
		}

		// Validate damage
		if (BaseDamage <= 0.0f || BaseDamage > 1000.0f)
		{
			Context.AddError(FString::Printf(
				TEXT("BaseDamage out of range: %.2f (expected 1-1000)"), BaseDamage));
		}

		// Validate move speed
		if (MoveSpeed < 50.0f || MoveSpeed > 1000.0f)
		{
			Context.AddError(FString::Printf(
				TEXT("MoveSpeed out of range: %.2f (expected 50-1000)"), MoveSpeed));
		}

		// Generate report
		FString Report = Context.GetReport();
		UE_LOG(LogDelveDeepConfig, Display, TEXT("\n%s"), *Report);

		if (Context.IsValid())
		{
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Sample data validation PASSED"));
		}
		else
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("Sample data validation FAILED"));
		}
	})
);
