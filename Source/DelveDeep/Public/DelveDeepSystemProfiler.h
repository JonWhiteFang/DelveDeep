// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepSystemProfiler.generated.h"

/**
 * System performance data for a single system
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FSystemPerformanceData
{
	GENERATED_BODY()

	/** Name of the system being tracked */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FName SystemName;

	/** Cycle time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	double CycleTimeMs = 0.0;

	/** Budget time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	double BudgetTimeMs = 0.0;

	/** Number of times this system was called this frame */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 CallCount = 0;

	/** Peak cycle time recorded */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	double PeakTimeMs = 0.0;

	/** Average cycle time over recent frames */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	double AverageTimeMs = 0.0;

	FSystemPerformanceData()
		: SystemName(NAME_None)
		, CycleTimeMs(0.0)
		, BudgetTimeMs(0.0)
		, CallCount(0)
		, PeakTimeMs(0.0)
		, AverageTimeMs(0.0)
	{
	}

	FSystemPerformanceData(FName InSystemName, double InBudgetMs)
		: SystemName(InSystemName)
		, CycleTimeMs(0.0)
		, BudgetTimeMs(InBudgetMs)
		, CallCount(0)
		, PeakTimeMs(0.0)
		, AverageTimeMs(0.0)
	{
	}
};

/**
 * Budget violation record
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FBudgetViolation
{
	GENERATED_BODY()

	/** Name of the system that violated its budget */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FName SystemName;

	/** Actual time taken in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float ActualTimeMs = 0.0f;

	/** Budget time in milliseconds */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float BudgetTimeMs = 0.0f;

	/** Overage percentage (how much over budget) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float OveragePercentage = 0.0f;

	/** Timestamp when violation occurred */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FDateTime Timestamp;

	/** Frame number when violation occurred */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 FrameNumber = 0;

	FBudgetViolation()
		: SystemName(NAME_None)
		, ActualTimeMs(0.0f)
		, BudgetTimeMs(0.0f)
		, OveragePercentage(0.0f)
		, Timestamp(FDateTime::Now())
		, FrameNumber(0)
	{
	}
};

/**
 * System Profiler
 * 
 * Tracks performance of individual game systems with budget monitoring:
 * - Per-system cycle time tracking
 * - Budget allocation and utilization
 * - Budget violation detection and logging
 * - Performance statistics (average, peak)
 */
class DELVEDEEP_API FSystemProfiler
{
public:
	FSystemProfiler();

	/**
	 * Register a system with a performance budget
	 * @param SystemName Name of the system to register
	 * @param BudgetMs Performance budget in milliseconds
	 */
	void RegisterSystem(FName SystemName, float BudgetMs);

	/**
	 * Record system execution time
	 * @param SystemName Name of the system
	 * @param CycleTimeMs Execution time in milliseconds
	 */
	void RecordSystemTime(FName SystemName, double CycleTimeMs);

	/**
	 * Get performance data for a specific system
	 * @param SystemName Name of the system
	 * @return System performance data
	 */
	FSystemPerformanceData GetSystemData(FName SystemName) const;

	/**
	 * Get performance data for all registered systems
	 * @return Array of system performance data
	 */
	TArray<FSystemPerformanceData> GetAllSystemData() const;

	/**
	 * Get budget utilization percentage for a system
	 * @param SystemName Name of the system
	 * @return Budget utilization (0.0 to 1.0+, >1.0 means over budget)
	 */
	float GetBudgetUtilization(FName SystemName) const;

	/**
	 * Check if a system is currently violating its budget
	 * @param SystemName Name of the system
	 * @return True if system is over budget
	 */
	bool IsBudgetViolated(FName SystemName) const;

	/**
	 * Get budget violation history
	 * @return Array of recent budget violations
	 */
	TArray<FBudgetViolation> GetViolationHistory() const { return ViolationHistory; }

	/**
	 * Get total number of budget violations
	 * @return Total violation count
	 */
	int32 GetTotalViolationCount() const { return TotalViolations; }

	/**
	 * Reset all statistics
	 */
	void ResetStatistics();

	/**
	 * Update frame (called once per frame to update averages)
	 */
	void UpdateFrame();

private:
	/** Maximum number of violations to track in history */
	static constexpr int32 MaxViolationHistory = 100;

	/** Number of frames to average for statistics */
	static constexpr int32 AverageFrameCount = 60;

	/** System performance data map */
	TMap<FName, FSystemPerformanceData> SystemData;

	/** Budget violation history (circular buffer) */
	TArray<FBudgetViolation> ViolationHistory;

	/** Total number of violations recorded */
	int32 TotalViolations;

	/** Current frame number */
	int64 CurrentFrame;

	/** Per-system frame time history for averaging */
	TMap<FName, TArray<double>> SystemTimeHistory;

	/**
	 * Check for budget violation and record if necessary
	 * @param SystemName Name of the system to check
	 */
	void CheckBudgetViolation(FName SystemName);

	/**
	 * Record a budget violation
	 * @param Violation Violation data to record
	 */
	void RecordViolation(const FBudgetViolation& Violation);

	/**
	 * Update average time for a system
	 * @param SystemName Name of the system
	 */
	void UpdateAverageTime(FName SystemName);
};
