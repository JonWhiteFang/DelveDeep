// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepMemoryTracker.generated.h"

/**
 * Memory snapshot for a single point in time
 */
USTRUCT()
struct DELVEDEEP_API FMemorySnapshot
{
	GENERATED_BODY()

	/** Total memory usage in bytes */
	UPROPERTY()
	int64 TotalMemory = 0;

	/** Native (C++) memory usage in bytes */
	UPROPERTY()
	int64 NativeMemory = 0;

	/** Managed (UObject) memory usage in bytes */
	UPROPERTY()
	int64 ManagedMemory = 0;

	/** Per-system memory allocations */
	UPROPERTY()
	TMap<FName, int64> PerSystemMemory;

	/** Timestamp when snapshot was taken */
	UPROPERTY()
	FDateTime Timestamp;

	FMemorySnapshot()
		: TotalMemory(0)
		, NativeMemory(0)
		, ManagedMemory(0)
		, Timestamp(FDateTime::Now())
	{
	}
};

/**
 * Memory Tracker
 * 
 * Tracks memory usage across the game:
 * - Total memory (native + managed)
 * - Per-system memory allocation
 * - Memory leak detection
 * - Memory growth rate monitoring
 */
class DELVEDEEP_API FMemoryTracker
{
public:
	FMemoryTracker();

	/**
	 * Update memory snapshot (captures current memory state)
	 */
	void UpdateMemorySnapshot();

	/**
	 * Get current memory snapshot
	 * @return Current memory snapshot
	 */
	FMemorySnapshot GetCurrentSnapshot() const { return CurrentSnapshot; }

	/**
	 * Get memory usage for a specific system
	 * @param SystemName Name of the system
	 * @return Memory usage in bytes
	 */
	uint64 GetSystemMemory(FName SystemName) const;

	/**
	 * Track memory allocation for a system
	 * @param SystemName Name of the system
	 * @param AllocationSize Size of allocation in bytes
	 */
	void TrackSystemAllocation(FName SystemName, uint64 AllocationSize);

	/**
	 * Track memory deallocation for a system
	 * @param SystemName Name of the system
	 * @param DeallocationSize Size of deallocation in bytes
	 */
	void TrackSystemDeallocation(FName SystemName, uint64 DeallocationSize);

	/**
	 * Check if a memory leak is detected
	 * @return True if leak detected (memory growing faster than threshold)
	 */
	bool IsLeakDetected() const { return bLeakDetected; }

	/**
	 * Get memory growth rate in MB per minute
	 * @return Growth rate
	 */
	float GetMemoryGrowthRate() const { return MemoryGrowthRateMBPerMin; }

	/**
	 * Get memory history
	 * @return Array of memory snapshots
	 */
	TArray<FMemorySnapshot> GetMemoryHistory() const { return MemoryHistory; }

	/**
	 * Get peak memory usage
	 * @return Peak memory in bytes
	 */
	uint64 GetPeakMemoryUsage() const { return PeakMemoryUsage; }

	/**
	 * Reset all statistics
	 */
	void ResetStatistics();

private:
	/** Memory leak detection threshold (MB per minute) */
	static constexpr float LeakDetectionThresholdMBPerMin = 10.0f;

	/** Maximum number of snapshots to keep in history */
	static constexpr int32 MaxHistorySize = 100;

	/** Update interval in frames (update every 100 frames) */
	static constexpr int32 UpdateIntervalFrames = 100;

	/** Current memory snapshot */
	FMemorySnapshot CurrentSnapshot;

	/** Memory snapshot history */
	TArray<FMemorySnapshot> MemoryHistory;

	/** Last total memory recorded */
	uint64 LastTotalMemory;

	/** Peak memory usage */
	uint64 PeakMemoryUsage;

	/** Time of last memory check */
	FDateTime LastCheckTime;

	/** Memory growth rate in MB per minute */
	float MemoryGrowthRateMBPerMin;

	/** Whether a memory leak is detected */
	bool bLeakDetected;

	/** Frame counter for update interval */
	int32 FrameCounter;

	/**
	 * Detect memory leaks by analyzing growth rate
	 */
	void DetectMemoryLeaks();

	/**
	 * Calculate memory growth rate
	 */
	void CalculateGrowthRate();

	/**
	 * Capture platform memory statistics
	 */
	void CapturePlatformMemory();
};
