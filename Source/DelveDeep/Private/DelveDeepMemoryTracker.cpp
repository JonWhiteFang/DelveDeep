// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepMemoryTracker.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "HAL/PlatformMemory.h"
#include "Stats/StatsData.h"

FMemoryTracker::FMemoryTracker()
	: LastTotalMemory(0)
	, PeakMemoryUsage(0)
	, LastCheckTime(FDateTime::Now())
	, MemoryGrowthRateMBPerMin(0.0f)
	, bLeakDetected(false)
	, FrameCounter(0)
{
	MemoryHistory.Reserve(MaxHistorySize);
}

void FMemoryTracker::UpdateMemorySnapshot()
{
	FrameCounter++;

	// Only update every N frames to minimize overhead
	if (FrameCounter < UpdateIntervalFrames)
	{
		return;
	}

	FrameCounter = 0;

	// Capture platform memory statistics
	CapturePlatformMemory();

	// Update timestamp
	CurrentSnapshot.Timestamp = FDateTime::Now();

	// Add to history
	MemoryHistory.Add(CurrentSnapshot);

	// Keep history size limited
	if (MemoryHistory.Num() > MaxHistorySize)
	{
		MemoryHistory.RemoveAt(0);
	}

	// Update peak memory
	if (CurrentSnapshot.TotalMemory > PeakMemoryUsage)
	{
		PeakMemoryUsage = CurrentSnapshot.TotalMemory;
	}

	// Calculate growth rate
	CalculateGrowthRate();

	// Detect memory leaks
	DetectMemoryLeaks();

	// Update last total memory
	LastTotalMemory = CurrentSnapshot.TotalMemory;
}

uint64 FMemoryTracker::GetSystemMemory(FName SystemName) const
{
	const uint64* Memory = CurrentSnapshot.PerSystemMemory.Find(SystemName);
	return Memory ? *Memory : 0;
}

void FMemoryTracker::TrackSystemAllocation(FName SystemName, uint64 AllocationSize)
{
	uint64& SystemMemory = CurrentSnapshot.PerSystemMemory.FindOrAdd(SystemName);
	SystemMemory += AllocationSize;
	CurrentSnapshot.TotalMemory += AllocationSize;
}

void FMemoryTracker::TrackSystemDeallocation(FName SystemName, uint64 DeallocationSize)
{
	uint64* SystemMemory = CurrentSnapshot.PerSystemMemory.Find(SystemName);
	if (SystemMemory)
	{
		*SystemMemory = (*SystemMemory > DeallocationSize) ? (*SystemMemory - DeallocationSize) : 0;
	}

	CurrentSnapshot.TotalMemory = (CurrentSnapshot.TotalMemory > DeallocationSize) 
		? (CurrentSnapshot.TotalMemory - DeallocationSize) 
		: 0;
}

void FMemoryTracker::ResetStatistics()
{
	CurrentSnapshot = FMemorySnapshot();
	MemoryHistory.Empty();
	LastTotalMemory = 0;
	PeakMemoryUsage = 0;
	LastCheckTime = FDateTime::Now();
	MemoryGrowthRateMBPerMin = 0.0f;
	bLeakDetected = false;
	FrameCounter = 0;

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Memory tracker statistics reset"));
}

void FMemoryTracker::DetectMemoryLeaks()
{
	// Check if growth rate exceeds threshold
	if (MemoryGrowthRateMBPerMin > LeakDetectionThresholdMBPerMin)
	{
		if (!bLeakDetected)
		{
			bLeakDetected = true;
			UE_LOG(LogDelveDeepTelemetry, Warning,
				TEXT("Potential memory leak detected: %.2f MB/min growth rate (threshold: %.2f MB/min)"),
				MemoryGrowthRateMBPerMin,
				LeakDetectionThresholdMBPerMin);
		}
	}
	else
	{
		if (bLeakDetected)
		{
			bLeakDetected = false;
			UE_LOG(LogDelveDeepTelemetry, Display,
				TEXT("Memory growth rate normalized: %.2f MB/min"),
				MemoryGrowthRateMBPerMin);
		}
	}
}

void FMemoryTracker::CalculateGrowthRate()
{
	if (LastTotalMemory == 0)
	{
		MemoryGrowthRateMBPerMin = 0.0f;
		return;
	}

	// Calculate time elapsed since last check
	const FDateTime CurrentTime = FDateTime::Now();
	const FTimespan TimeDelta = CurrentTime - LastCheckTime;
	const double MinutesElapsed = TimeDelta.GetTotalMinutes();

	if (MinutesElapsed <= 0.0)
	{
		return;
	}

	// Calculate memory delta in MB
	const int64 MemoryDelta = static_cast<int64>(CurrentSnapshot.TotalMemory) - static_cast<int64>(LastTotalMemory);
	const double MemoryDeltaMB = MemoryDelta / (1024.0 * 1024.0);

	// Calculate growth rate (MB per minute)
	MemoryGrowthRateMBPerMin = static_cast<float>(MemoryDeltaMB / MinutesElapsed);

	// Update last check time
	LastCheckTime = CurrentTime;
}

void FMemoryTracker::CapturePlatformMemory()
{
	// Get platform memory statistics
	FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();

	// Update total memory
	CurrentSnapshot.TotalMemory = MemoryStats.UsedPhysical;

	// Estimate native vs managed memory
	// This is a simplified estimation - actual tracking would require custom allocators
	const uint64 EstimatedManagedMemory = GUObjectArray.GetObjectArrayEstimatedAvailableMemory();
	CurrentSnapshot.ManagedMemory = EstimatedManagedMemory;
	CurrentSnapshot.NativeMemory = (CurrentSnapshot.TotalMemory > EstimatedManagedMemory) 
		? (CurrentSnapshot.TotalMemory - EstimatedManagedMemory) 
		: 0;

	// Note: Per-system memory tracking requires manual instrumentation
	// Systems should call TrackSystemAllocation/Deallocation when allocating memory
}
