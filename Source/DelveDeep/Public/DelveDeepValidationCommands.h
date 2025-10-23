// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Console commands for validation system testing and debugging.
 * Provides commands for validating objects, managing rules, cache, and metrics.
 */
class DELVEDEEP_API FDelveDeepValidationCommands
{
public:
	/** Registers all validation console commands */
	static void RegisterCommands();

	/** Unregisters all validation console commands */
	static void UnregisterCommands();

private:
	// Command implementations
	static void ValidateObject(const TArray<FString>& Args);
	static void ListValidationRules(const TArray<FString>& Args);
	static void ListRulesForClass(const TArray<FString>& Args);
	static void ShowValidationCache(const TArray<FString>& Args);
	static void ClearValidationCache(const TArray<FString>& Args);
	static void ShowValidationMetrics(const TArray<FString>& Args);
	static void ResetValidationMetrics(const TArray<FString>& Args);
	static void ExportValidationMetrics(const TArray<FString>& Args);
	static void TestValidationSeverity(const TArray<FString>& Args);
	static void ProfileValidation(const TArray<FString>& Args);

	// Helper functions
	static class UDelveDeepValidationSubsystem* GetValidationSubsystem();
	static UObject* LoadObjectFromPath(const FString& ObjectPath);
};
