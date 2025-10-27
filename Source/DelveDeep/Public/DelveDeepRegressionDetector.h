// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DelveDeepTestReport.h"
#include "DelveDeepRegressionDetector.generated.h"

/**
 * Regression Detection
 * 
 * Compares test results across builds to detect performance and memory regressions.
 * Generates reports highlighting tests that have degraded since the baseline.
 * 
 * Usage:
 *   FRegressionDetector Detector;
 *   bool bHasRegressions = Detector.CompareTestResults(Baseline, Current);
 *   TArray<FRegressionReport> Regressions = Detector.GetRegressions();
 */

UENUM(BlueprintType)
enum class ERegressionType : uint8
{
	None UMETA(DisplayName = "None"),
	Performance UMETA(DisplayName = "Performance"),
	Memory UMETA(DisplayName = "Memory"),
	Failure UMETA(DisplayName = "Test Failure"),
	Flaky UMETA(DisplayName = "Flaky Test")
};

USTRUCT(BlueprintType)
struct DELVEDEEP_API FRegressionReport
{
	GENERATED_BODY()

	UPROPERTY()
	FString TestName;

	UPROPERTY()
	ERegressionType RegressionType;

	UPROPERTY()
	FString Description;

	UPROPERTY()
	float BaselineValue;

	UPROPERTY()
	float CurrentValue;

	UPROPERTY()
	float PercentageChange;

	UPROPERTY()
	FDateTime DetectionTime;

	FString GetSeverity() const
	{
		if (PercentageChange > 50.0f)
		{
			return TEXT("Critical");
		}
		else if (PercentageChange > 25.0f)
		{
			return TEXT("High");
		}
		else if (PercentageChange > 10.0f)
		{
			return TEXT("Medium");
		}
		else
		{
			return TEXT("Low");
		}
	}
};

USTRUCT(BlueprintType)
struct DELVEDEEP_API FRegressionThresholds
{
	GENERATED_BODY()

	// Performance regression threshold (percentage)
	UPROPERTY()
	float PerformanceThreshold = 10.0f;

	// Memory regression threshold (percentage)
	UPROPERTY()
	float MemoryThreshold = 15.0f;

	// Flaky test threshold (failure rate)
	UPROPERTY()
	float FlakyTestThreshold = 0.2f;  // 20% failure rate

	// Minimum execution time to consider for performance regression (ms)
	UPROPERTY()
	float MinExecutionTime = 1.0f;
};

UCLASS()
class DELVEDEEP_API UDelveDeepRegressionDetector : public UObject
{
	GENERATED_BODY()

public:
	UDelveDeepRegressionDetector();

	// Compare test results and detect regressions
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Regression")
	bool CompareTestResults(
		const FDelveDeepTestReport& Baseline,
		const FDelveDeepTestReport& Current);

	// Get detected regressions
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Regression")
	TArray<FRegressionReport> GetRegressions() const { return Regressions; }

	// Get regressions by type
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Regression")
	TArray<FRegressionReport> GetRegressionsByType(ERegressionType Type) const;

	// Get critical regressions (>50% degradation)
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Regression")
	TArray<FRegressionReport> GetCriticalRegressions() const;

	// Check if any regressions were detected
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Regression")
	bool HasRegressions() const { return Regressions.Num() > 0; }

	// Get regression count
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Regression")
	int32 GetRegressionCount() const { return Regressions.Num(); }

	// Set regression thresholds
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Regression")
	void SetThresholds(const FRegressionThresholds& NewThresholds);

	// Get current thresholds
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Regression")
	FRegressionThresholds GetThresholds() const { return Thresholds; }

	// Generate regression report
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Regression")
	FString GenerateRegressionReport() const;

	// Export regression report to file
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Regression")
	bool ExportRegressionReport(const FString& OutputPath) const;

	// Load baseline report from file
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Regression")
	bool LoadBaselineReport(const FString& FilePath);

	// Save current report as baseline
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Regression")
	bool SaveBaselineReport(const FDelveDeepTestReport& Report, const FString& FilePath);

private:
	// Detect performance regressions
	void DetectPerformanceRegressions(
		const FDelveDeepTestReport& Baseline,
		const FDelveDeepTestReport& Current);

	// Detect memory regressions
	void DetectMemoryRegressions(
		const FDelveDeepTestReport& Baseline,
		const FDelveDeepTestReport& Current);

	// Detect test failures
	void DetectTestFailures(
		const FDelveDeepTestReport& Baseline,
		const FDelveDeepTestReport& Current);

	// Detect flaky tests
	void DetectFlakyTests(
		const FDelveDeepTestReport& Baseline,
		const FDelveDeepTestReport& Current);

	// Find test result by name
	const FDelveDeepTestResult* FindTestResult(
		const FDelveDeepTestReport& Report,
		const FString& TestName) const;

	// Calculate percentage change
	float CalculatePercentageChange(float Baseline, float Current) const;

	// Generate HTML report
	FString GenerateHTMLReport() const;

	// Generate markdown report
	FString GenerateMarkdownReport() const;

	UPROPERTY()
	TArray<FRegressionReport> Regressions;

	UPROPERTY()
	FRegressionThresholds Thresholds;

	UPROPERTY()
	FDelveDeepTestReport BaselineReport;
};
