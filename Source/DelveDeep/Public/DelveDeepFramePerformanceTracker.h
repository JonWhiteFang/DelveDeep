// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/CircularBuffer.h"
#include "DelveDeepFramePerformanceTracker.generated.h"

/**
 * Frame performance data for a single frame
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FFramePerformanceData
{
	GENERATED_BODY()

	/** Frame time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FrameTimeMs = 0.0f;

	/** Game thread time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GameThreadTimeMs = 0.0f;

	/** Render thread time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float RenderThreadTimeMs = 0.0f;

	/** Timestamp when this frame was recorded */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FDateTime Timestamp;

	FFramePerformanceData()
		: FrameTimeMs(0.0f)
		, GameThreadTimeMs(0.0f)
		, RenderThreadTimeMs(0.0f)
		, Timestamp(FDateTime::Now())
	{
	}
};

/**
 * Frame Performance Tracker
 * 
 * Tracks frame performance metrics including:
 * - Current, average, and percentile FPS
 * - Frame time history
 * - Spike detection
 * - Performance statistics
 */
class DELVEDEEP_API FFramePerformanceTracker
{
public:
	FFramePerformanceTracker();

	/**
	 * Record a frame's performance data
	 * @param DeltaTime Frame delta time in seconds
	 */
	void RecordFrame(float DeltaTime);

	/**
	 * Get current frames per second
	 * @return Current FPS
	 */
	float GetCurrentFPS() const { return CurrentFPS; }

	/**
	 * Get average FPS over recent frames
	 * @param NumFrames Number of frames to average (default: 60)
	 * @return Average FPS
	 */
	float GetAverageFPS(int32 NumFrames = 60) const;

	/**
	 * Get 1% low FPS (99th percentile worst frame times)
	 * @return 1% low FPS
	 */
	float GetOnePercentLowFPS() const;

	/**
	 * Get 0.1% low FPS (99.9th percentile worst frame times)
	 * @return 0.1% low FPS
	 */
	float GetPointOnePercentLowFPS() const;

	/**
	 * Check if a performance spike was detected in recent frames
	 * @return True if spike detected
	 */
	bool IsSpikeDetected() const { return ConsecutiveSlowFrames > 0; }

	/**
	 * Get frame time history
	 * @param NumFrames Number of recent frames to retrieve
	 * @return Array of frame times in milliseconds
	 */
	TArray<float> GetFrameTimeHistory(int32 NumFrames) const;

	/**
	 * Get current frame performance data
	 * @return Current frame data
	 */
	FFramePerformanceData GetCurrentFrameData() const { return CurrentFrameData; }

	/**
	 * Reset all statistics
	 */
	void ResetStatistics();

private:
	/** Maximum number of frames to track (60 seconds at 60 FPS) */
	static constexpr int32 MaxFrameHistory = 3600;

	/** Spike threshold in milliseconds (16.67ms = 60 FPS) */
	static constexpr float SpikeThresholdMs = 16.67f;

	/** Circular buffer for frame times */
	TCircularBuffer<float> FrameTimes;

	/** Current FPS */
	float CurrentFPS;

	/** Current frame data */
	FFramePerformanceData CurrentFrameData;

	/** Number of consecutive slow frames */
	int32 ConsecutiveSlowFrames;

	/** Total frames recorded */
	int64 TotalFramesRecorded;

	/**
	 * Calculate FPS statistics (average, percentiles)
	 */
	void CalculateFPSStatistics();

	/**
	 * Detect performance spikes
	 */
	void DetectSpikes(float FrameTimeMs);

	/**
	 * Calculate percentile FPS from frame times
	 * @param Percentile Percentile to calculate (0.0 to 1.0)
	 * @return FPS at the given percentile
	 */
	float CalculatePercentileFPS(float Percentile) const;
};
