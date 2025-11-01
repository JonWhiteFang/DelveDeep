// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepPerformanceReport.generated.h"

/**
 * Performance Report
 * 
 * Aggregated performance data over a time period with comprehensive statistics.
 */
USTRUCT()
struct DELVEDEEP_API FPerformanceReport
{
	GENERATED_BODY()

	/** Timestamp when report was generated */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FDateTime GenerationTime;

	/** Duration of data collection in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float DurationSeconds = 0.0f;

	/** Build version */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FString BuildVersion;

	/** Map name */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FString MapName;

	// Frame Statistics

	/** Average FPS over the reporting period */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float AverageFPS = 0.0f;

	/** Minimum FPS recorded */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float MinFPS = 0.0f;

	/** Maximum FPS recorded */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float MaxFPS = 0.0f;

	/** 1% low FPS (99th percentile worst frames) */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float OnePercentLowFPS = 0.0f;

	/** 0.1% low FPS (99.9th percentile worst frames) */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float PointOnePercentLowFPS = 0.0f;

	/** 50th percentile frame time (median) */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float MedianFrameTimeMs = 0.0f;

	/** 95th percentile frame time */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float Percentile95FrameTimeMs = 0.0f;

	/** 99th percentile frame time */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	float Percentile99FrameTimeMs = 0.0f;

	/** Total frames recorded */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	int32 TotalFrames = 0;

	/** Number of performance spikes detected */
	UPROPERTY(BlueprintReadOnly, Category = "Frame")
	int32 SpikeCount = 0;

	// System Performance

	/** System performance breakdown */
	UPROPERTY(BlueprintReadOnly, Category = "Systems")
	TArray<FSystemPerformanceData> SystemBreakdown;

	/** Total budget violations across all systems */
	UPROPERTY(BlueprintReadOnly, Category = "Systems")
	int32 TotalBudgetViolations = 0;

	// Memory Statistics

	/** Average memory usage in bytes */
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	uint64 AverageMemoryUsage = 0;

	/** Peak memory usage in bytes */
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	uint64 PeakMemoryUsage = 0;

	/** Minimum memory usage in bytes */
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	uint64 MinMemoryUsage = 0;

	/** Memory growth rate in MB per minute */
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	float MemoryGrowthRate = 0.0f;

	/** Whether a memory leak was detected */
	UPROPERTY(BlueprintReadOnly, Category = "Memory")
	bool bMemoryLeakDetected = false;

	// Gameplay Metrics

	/** Peak monster count during reporting period */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 PeakMonsterCount = 0;

	/** Peak projectile count during reporting period */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 PeakProjectileCount = 0;

	/** Peak particle count during reporting period */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 PeakParticleCount = 0;

	/** Total events processed during reporting period */
	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 TotalEventsProcessed = 0;

	FPerformanceReport()
		: GenerationTime(FDateTime::Now())
		, DurationSeconds(0.0f)
		, AverageFPS(0.0f)
		, MinFPS(0.0f)
		, MaxFPS(0.0f)
		, OnePercentLowFPS(0.0f)
		, PointOnePercentLowFPS(0.0f)
		, MedianFrameTimeMs(0.0f)
		, Percentile95FrameTimeMs(0.0f)
		, Percentile99FrameTimeMs(0.0f)
		, TotalFrames(0)
		, SpikeCount(0)
		, TotalBudgetViolations(0)
		, AverageMemoryUsage(0)
		, PeakMemoryUsage(0)
		, MinMemoryUsage(0)
		, MemoryGrowthRate(0.0f)
		, bMemoryLeakDetected(false)
		, PeakMonsterCount(0)
		, PeakProjectileCount(0)
		, PeakParticleCount(0)
		, TotalEventsProcessed(0)
	{
	}

	/**
	 * Generate a formatted text report
	 * @return Formatted report string
	 */
	FString GenerateFormattedReport() const;

	/**
	 * Generate a summary string (one-line overview)
	 * @return Summary string
	 */
	FString GenerateSummary() const;
};
