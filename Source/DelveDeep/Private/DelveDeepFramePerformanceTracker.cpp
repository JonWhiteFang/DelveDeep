// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepTelemetrySubsystem.h"

FFramePerformanceTracker::FFramePerformanceTracker()
	: FrameTimes(MaxFrameHistory)
	, CurrentFPS(0.0f)
	, ConsecutiveSlowFrames(0)
	, TotalFramesRecorded(0)
{
}

void FFramePerformanceTracker::RecordFrame(float DeltaTime)
{
	// Calculate frame time in milliseconds
	const float FrameTimeMs = DeltaTime * 1000.0f;

	// Store in circular buffer
	FrameTimes.Add(FrameTimeMs);

	// Calculate current FPS
	if (DeltaTime > 0.0f)
	{
		CurrentFPS = 1.0f / DeltaTime;
	}

	// Update current frame data
	CurrentFrameData.FrameTimeMs = FrameTimeMs;
	CurrentFrameData.GameThreadTimeMs = FrameTimeMs; // Simplified for now
	CurrentFrameData.RenderThreadTimeMs = 0.0f; // Will be updated when render thread tracking is added
	CurrentFrameData.Timestamp = FDateTime::Now();

	// Detect spikes
	DetectSpikes(FrameTimeMs);

	// Increment total frames
	TotalFramesRecorded++;
}

float FFramePerformanceTracker::GetAverageFPS(int32 NumFrames) const
{
	if (FrameTimes.Num() == 0)
	{
		return 0.0f;
	}

	// Limit to available frames
	const int32 FramesToAverage = FMath::Min(NumFrames, FrameTimes.Num());
	
	// Calculate average frame time
	float TotalFrameTime = 0.0f;
	for (int32 i = 0; i < FramesToAverage; ++i)
	{
		const int32 Index = FrameTimes.Num() - 1 - i;
		TotalFrameTime += FrameTimes[Index];
	}

	const float AverageFrameTime = TotalFrameTime / FramesToAverage;

	// Convert to FPS (avoid division by zero)
	if (AverageFrameTime > 0.0f)
	{
		return 1000.0f / AverageFrameTime;
	}

	return 0.0f;
}

float FFramePerformanceTracker::GetOnePercentLowFPS() const
{
	return CalculatePercentileFPS(0.99f);
}

float FFramePerformanceTracker::GetPointOnePercentLowFPS() const
{
	return CalculatePercentileFPS(0.999f);
}

TArray<float> FFramePerformanceTracker::GetFrameTimeHistory(int32 NumFrames) const
{
	TArray<float> History;
	
	if (FrameTimes.Num() == 0)
	{
		return History;
	}

	// Limit to available frames
	const int32 FramesToRetrieve = FMath::Min(NumFrames, FrameTimes.Num());
	History.Reserve(FramesToRetrieve);

	// Get most recent frames
	for (int32 i = 0; i < FramesToRetrieve; ++i)
	{
		const int32 Index = FrameTimes.Num() - FramesToRetrieve + i;
		History.Add(FrameTimes[Index]);
	}

	return History;
}

void FFramePerformanceTracker::ResetStatistics()
{
	FrameTimes.Empty();
	CurrentFPS = 0.0f;
	ConsecutiveSlowFrames = 0;
	TotalFramesRecorded = 0;
	CurrentFrameData = FFramePerformanceData();
}

void FFramePerformanceTracker::CalculateFPSStatistics()
{
	// Statistics are calculated on-demand in getter methods
	// This method is reserved for future batch calculations
}

void FFramePerformanceTracker::DetectSpikes(float FrameTimeMs)
{
	// Check if frame time exceeds spike threshold
	if (FrameTimeMs > SpikeThresholdMs)
	{
		ConsecutiveSlowFrames++;

		// Log warning for significant spikes
		if (ConsecutiveSlowFrames == 1)
		{
			UE_LOG(LogDelveDeepTelemetry, Warning, 
				TEXT("Performance spike detected: %.2fms (%.1f FPS)"),
				FrameTimeMs, CurrentFPS);
		}
		else if (ConsecutiveSlowFrames >= 5)
		{
			UE_LOG(LogDelveDeepTelemetry, Error,
				TEXT("Sustained performance issue: %d consecutive slow frames (%.2fms average)"),
				ConsecutiveSlowFrames, FrameTimeMs);
		}
	}
	else
	{
		// Reset counter when performance recovers
		if (ConsecutiveSlowFrames > 0)
		{
			UE_LOG(LogDelveDeepTelemetry, Display,
				TEXT("Performance recovered after %d slow frames"),
				ConsecutiveSlowFrames);
			ConsecutiveSlowFrames = 0;
		}
	}
}

float FFramePerformanceTracker::CalculatePercentileFPS(float Percentile) const
{
	if (FrameTimes.Num() == 0)
	{
		return 0.0f;
	}

	// Copy frame times to array for sorting
	TArray<float> SortedFrameTimes;
	SortedFrameTimes.Reserve(FrameTimes.Num());
	
	for (int32 i = 0; i < FrameTimes.Num(); ++i)
	{
		SortedFrameTimes.Add(FrameTimes[i]);
	}

	// Sort frame times (ascending)
	SortedFrameTimes.Sort();

	// Calculate percentile index
	const int32 PercentileIndex = FMath::FloorToInt(Percentile * (SortedFrameTimes.Num() - 1));
	const float PercentileFrameTime = SortedFrameTimes[PercentileIndex];

	// Convert to FPS
	if (PercentileFrameTime > 0.0f)
	{
		return 1000.0f / PercentileFrameTime;
	}

	return 0.0f;
}
