// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepProfilingSession.generated.h"

/**
 * Profiling session for detailed performance metric collection
 * 
 * Captures per-frame data for all systems with increased sampling rate.
 * Limited to 60 seconds duration to prevent excessive memory usage.
 * Automatically saves profiling data with timestamp-based filename.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FProfilingSession
{
	GENERATED_BODY()

	/** Name of this profiling session */
	UPROPERTY(BlueprintReadOnly, Category = "Profiling")
	FName SessionName;

	/** Session start time */
	UPROPERTY(BlueprintReadOnly, Category = "Profiling")
	FDateTime StartTime;

	/** Session end time */
	UPROPERTY(BlueprintReadOnly, Category = "Profiling")
	FDateTime EndTime;

	/** Whether session is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "Profiling")
	bool bActive = false;

	/** Frame performance data captured during session */
	TArray<FFramePerformanceData> FrameData;

	/** System performance data captured per frame */
	TMap<FName, TArray<FSystemPerformanceData>> SystemData;

	/** Memory snapshots captured during session */
	TArray<FMemorySnapshot> MemorySnapshots;

	/** Total frames captured */
	UPROPERTY(BlueprintReadOnly, Category = "Profiling")
	int32 TotalFrames = 0;

	/** Session duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Profiling")
	float DurationSeconds = 0.0f;

	/** Maximum session duration (60 seconds) */
	static constexpr float MaxDurationSeconds = 60.0f;

	/**
	 * Check if session has reached maximum duration
	 * @return True if session should be stopped
	 */
	bool HasReachedMaxDuration() const
	{
		return DurationSeconds >= MaxDurationSeconds;
	}

	/**
	 * Get estimated memory usage of this session
	 * @return Estimated memory in bytes
	 */
	uint64 GetEstimatedMemoryUsage() const
	{
		uint64 TotalMemory = 0;

		// Frame data: ~32 bytes per frame
		TotalMemory += FrameData.Num() * 32;

		// System data: ~64 bytes per system per frame
		for (const auto& Pair : SystemData)
		{
			TotalMemory += Pair.Value.Num() * 64;
		}

		// Memory snapshots: ~128 bytes per snapshot
		TotalMemory += MemorySnapshots.Num() * 128;

		return TotalMemory;
	}

	/**
	 * Clear all captured data
	 */
	void Clear()
	{
		FrameData.Empty();
		SystemData.Empty();
		MemorySnapshots.Empty();
		TotalFrames = 0;
		DurationSeconds = 0.0f;
	}
};

/**
 * Profiling session report with aggregated statistics
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FProfilingSessionReport
{
	GENERATED_BODY()

	/** Session name */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FName SessionName;

	/** Session duration */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float DurationSeconds = 0.0f;

	/** Total frames captured */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 TotalFrames = 0;

	/** Average FPS */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float AverageFPS = 0.0f;

	/** Minimum FPS */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float MinFPS = 0.0f;

	/** Maximum FPS */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float MaxFPS = 0.0f;

	/** 1% low FPS */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float OnePercentLowFPS = 0.0f;

	/** Number of frame spikes detected */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 SpikeCount = 0;

	/** System performance breakdown */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	TArray<FSystemPerformanceData> SystemBreakdown;

	/** Average memory usage */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	uint64 AverageMemoryUsage = 0;

	/** Peak memory usage */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	uint64 PeakMemoryUsage = 0;

	/** Session start time */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FDateTime StartTime;

	/** Session end time */
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FDateTime EndTime;
};
