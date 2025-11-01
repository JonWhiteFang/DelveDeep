// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepPerformanceBaseline.generated.h"

/**
 * Performance Baseline
 * 
 * Captures a snapshot of performance metrics under controlled conditions
 * for comparison with future performance measurements.
 */
USTRUCT()
struct DELVEDEEP_API FPerformanceBaseline
{
	GENERATED_BODY()

	/** Name of this baseline */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	FName BaselineName;

	/** Timestamp when baseline was captured */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	FDateTime CaptureTime;

	/** Build version when baseline was captured */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	FString BuildVersion;

	/** Map name when baseline was captured */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	FString MapName;

	/** Average frame performance data */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	FFramePerformanceData AverageFrameData;

	/** Average FPS */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	float AverageFPS = 0.0f;

	/** 1% low FPS */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	float OnePercentLowFPS = 0.0f;

	/** System performance data at time of capture */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	TMap<FName, FSystemPerformanceData> SystemData;

	/** Memory snapshot at time of capture */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	FMemorySnapshot MemoryData;

	/** Total frames captured for this baseline */
	UPROPERTY(BlueprintReadOnly, Category = "Baseline")
	int32 TotalFramesCaptured = 0;

	FPerformanceBaseline()
		: BaselineName(NAME_None)
		, CaptureTime(FDateTime::Now())
		, BuildVersion(TEXT(""))
		, MapName(TEXT(""))
		, AverageFPS(0.0f)
		, OnePercentLowFPS(0.0f)
		, TotalFramesCaptured(0)
	{
	}
};

/**
 * Performance Comparison Result
 * 
 * Contains the results of comparing current performance to a baseline.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FPerformanceComparison
{
	GENERATED_BODY()

	/** Name of the baseline being compared against */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	FName BaselineName;

	/** Timestamp of comparison */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	FDateTime ComparisonTime;

	/** FPS change percentage (positive = improvement, negative = regression) */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	float FPSChangePercent = 0.0f;

	/** Frame time change percentage (negative = improvement, positive = regression) */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	float FrameTimeChangePercent = 0.0f;

	/** 1% low FPS change percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	float OnePercentLowChangePercent = 0.0f;

	/** Memory usage change percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	float MemoryChangePercent = 0.0f;

	/** Per-system performance changes */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	TMap<FName, float> SystemTimeChanges;

	/** Whether this represents a performance regression */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	bool bIsRegression = false;

	/** Whether this represents a performance improvement */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	bool bIsImprovement = false;

	/** Detailed comparison report */
	UPROPERTY(BlueprintReadOnly, Category = "Comparison")
	FString DetailedReport;

	FPerformanceComparison()
		: BaselineName(NAME_None)
		, ComparisonTime(FDateTime::Now())
		, FPSChangePercent(0.0f)
		, FrameTimeChangePercent(0.0f)
		, OnePercentLowChangePercent(0.0f)
		, MemoryChangePercent(0.0f)
		, bIsRegression(false)
		, bIsImprovement(false)
	{
	}
};
