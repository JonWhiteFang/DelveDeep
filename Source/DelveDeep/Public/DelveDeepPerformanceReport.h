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
	UPROPERTY()
	FDateTime GenerationTime;

	/** Duration of data collection in seconds */
	UPROPERTY()
	float DurationSeconds = 0.0f;

	/** Build version */
	UPROPERTY()
	FString BuildVersion;

	/** Map name */
	UPROPERTY()
	FString MapName;

	// Frame Statistics

	/** Average FPS over the reporting period */
	UPROPERTY()
	float AverageFPS = 0.0f;

	/** Minimum FPS recorded */
	UPROPERTY()
	float MinFPS = 0.0f;

	/** Maximum FPS recorded */
	UPROPERTY()
	float MaxFPS = 0.0f;

	/** 1% low FPS (99th percentile worst frames) */
	UPROPERTY()
	float OnePercentLowFPS = 0.0f;

	/** 0.1% low FPS (99.9th percentile worst frames) */
	UPROPERTY()
	float PointOnePercentLowFPS = 0.0f;

	/** 50th percentile frame time (median) */
	UPROPERTY()
	float MedianFrameTimeMs = 0.0f;

	/** 95th percentile frame time */
	UPROPERTY()
	float Percentile95FrameTimeMs = 0.0f;

	/** 99th percentile frame time */
	UPROPERTY()
	float Percentile99FrameTimeMs = 0.0f;

	/** Total frames recorded */
	UPROPERTY()
	int32 TotalFrames = 0;

	/** Number of performance spikes detected */
	UPROPERTY()
	int32 SpikeCount = 0;

	// System Performance

	/** System performance breakdown */
	UPROPERTY()
	TArray<FSystemPerformanceData> SystemBreakdown;

	/** Total budget violations across all systems */
	UPROPERTY()
	int32 TotalBudgetViolations = 0;

	// Memory Statistics

	/** Average memory usage in bytes */
	UPROPERTY()
	int64 AverageMemoryUsage = 0;

	/** Peak memory usage in bytes */
	UPROPERTY()
	int64 PeakMemoryUsage = 0;

	/** Minimum memory usage in bytes */
	UPROPERTY()
	int64 MinMemoryUsage = 0;

	/** Memory growth rate in MB per minute */
	UPROPERTY()
	float MemoryGrowthRate = 0.0f;

	/** Whether a memory leak was detected */
	UPROPERTY()
	bool bMemoryLeakDetected = false;

	// Gameplay Metrics

	/** Peak monster count during reporting period */
	UPROPERTY()
	int32 PeakMonsterCount = 0;

	/** Peak projectile count during reporting period */
	UPROPERTY()
	int32 PeakProjectileCount = 0;

	/** Peak particle count during reporting period */
	UPROPERTY()
	int32 PeakParticleCount = 0;

	/** Total events processed during reporting period */
	UPROPERTY()
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
