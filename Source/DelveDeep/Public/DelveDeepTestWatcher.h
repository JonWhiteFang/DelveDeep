// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DelveDeepTestWatcher.generated.h"

/**
 * Test Watch Mode
 * 
 * Monitors test files for changes and automatically runs affected tests.
 * Provides real-time feedback during development for rapid iteration.
 * 
 * Usage:
 *   DelveDeep.Test.StartWatch - Start watching test files
 *   DelveDeep.Test.StopWatch - Stop watching
 *   DelveDeep.Test.RunAffected - Manually run affected tests
 */

USTRUCT(BlueprintType)
struct DELVEDEEP_API FTestFileInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FString FilePath;

	UPROPERTY()
	FDateTime LastModified;

	UPROPERTY()
	TArray<FString> AffectedTests;
};

UCLASS()
class DELVEDEEP_API UDelveDeepTestWatcher : public UObject
{
	GENERATED_BODY()

public:
	UDelveDeepTestWatcher();

	// Start watching test files
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Testing")
	void StartWatching();

	// Stop watching test files
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Testing")
	void StopWatching();

	// Check if currently watching
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Testing")
	bool IsWatching() const { return bIsWatching; }

	// Manually trigger test run for affected tests
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Testing")
	void RunAffectedTests();

	// Set filter pattern for tests to run
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Testing")
	void SetTestFilter(const FString& FilterPattern);

	// Get list of watched files
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Testing")
	TArray<FString> GetWatchedFiles() const;

	// Tick function for checking file changes
	void Tick(float DeltaTime);

private:
	// Scan for test files
	void ScanTestFiles();

	// Check for file modifications
	void CheckForModifications();

	// Determine which tests are affected by file change
	TArray<FString> GetAffectedTests(const FString& FilePath);

	// Execute tests matching filter
	void ExecuteTests(const TArray<FString>& TestNames);

	// Display test results
	void DisplayResults(const TArray<FString>& TestNames, bool bAllPassed);

	UPROPERTY()
	bool bIsWatching;

	UPROPERTY()
	TMap<FString, FTestFileInfo> WatchedFiles;

	UPROPERTY()
	FString TestFilterPattern;

	UPROPERTY()
	float CheckInterval;

	UPROPERTY()
	float TimeSinceLastCheck;

	UPROPERTY()
	TArray<FString> PendingTests;
};
