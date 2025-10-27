// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DelveDeepCodeCoverageTracker.generated.h"

/**
 * Code Coverage Tracking
 * 
 * Tracks which lines of code are executed during test runs to measure
 * test coverage. Generates reports showing covered and uncovered code.
 * 
 * Usage:
 *   FCodeCoverageTracker Tracker;
 *   Tracker.StartTracking();
 *   // Run tests
 *   Tracker.StopTracking();
 *   FCodeCoverageReport Report = Tracker.GenerateReport();
 */

USTRUCT(BlueprintType)
struct DELVEDEEP_API FCodeCoverageData
{
	GENERATED_BODY()

	UPROPERTY()
	FString FilePath;

	UPROPERTY()
	int32 TotalLines;

	UPROPERTY()
	int32 CoveredLines;

	UPROPERTY()
	TArray<int32> ExecutedLines;

	UPROPERTY()
	TArray<int32> UncoveredLines;

	float GetCoveragePercentage() const
	{
		return TotalLines > 0 ? (float)CoveredLines / TotalLines * 100.0f : 0.0f;
	}
};

USTRUCT(BlueprintType)
struct DELVEDEEP_API FSystemCoverageData
{
	GENERATED_BODY()

	UPROPERTY()
	FString SystemName;

	UPROPERTY()
	TArray<FCodeCoverageData> Files;

	UPROPERTY()
	int32 TotalLines;

	UPROPERTY()
	int32 CoveredLines;

	float GetCoveragePercentage() const
	{
		return TotalLines > 0 ? (float)CoveredLines / TotalLines * 100.0f : 0.0f;
	}
};

USTRUCT(BlueprintType)
struct DELVEDEEP_API FCodeCoverageReport
{
	GENERATED_BODY()

	UPROPERTY()
	FDateTime GenerationTime;

	UPROPERTY()
	TArray<FSystemCoverageData> Systems;

	UPROPERTY()
	int32 TotalLines;

	UPROPERTY()
	int32 CoveredLines;

	UPROPERTY()
	float OverallCoverage;

	UPROPERTY()
	TMap<FString, float> CoverageBySystem;
};

UCLASS()
class DELVEDEEP_API UDelveDeepCodeCoverageTracker : public UObject
{
	GENERATED_BODY()

public:
	UDelveDeepCodeCoverageTracker();

	// Start tracking code coverage
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Coverage")
	void StartTracking();

	// Stop tracking code coverage
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Coverage")
	void StopTracking();

	// Check if currently tracking
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Coverage")
	bool IsTracking() const { return bIsTracking; }

	// Generate coverage report
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Coverage")
	FCodeCoverageReport GenerateReport();

	// Export report to HTML
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Coverage")
	bool ExportToHTML(const FString& OutputPath);

	// Export report to XML (Cobertura format)
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Coverage")
	bool ExportToXML(const FString& OutputPath);

	// Export report to JSON
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Coverage")
	bool ExportToJSON(const FString& OutputPath);

	// Get coverage for specific system
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Coverage")
	float GetSystemCoverage(const FString& SystemName) const;

	// Get overall coverage percentage
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Coverage")
	float GetOverallCoverage() const;

private:
	// Scan source files for coverage analysis
	void ScanSourceFiles();

	// Analyze file for line coverage
	FCodeCoverageData AnalyzeFile(const FString& FilePath);

	// Determine which system a file belongs to
	FString GetSystemForFile(const FString& FilePath) const;

	// Generate HTML report content
	FString GenerateHTMLContent(const FCodeCoverageReport& Report);

	// Generate XML report content (Cobertura format)
	FString GenerateXMLContent(const FCodeCoverageReport& Report);

	// Generate JSON report content
	FString GenerateJSONContent(const FCodeCoverageReport& Report);

	// Calculate coverage statistics
	void CalculateStatistics(FCodeCoverageReport& Report);

	UPROPERTY()
	bool bIsTracking;

	UPROPERTY()
	TMap<FString, FCodeCoverageData> CoverageData;

	UPROPERTY()
	TArray<FString> SourceFiles;

	UPROPERTY()
	FDateTime TrackingStartTime;
};
