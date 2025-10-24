// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DelveDeepStats.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepPerformanceBudget.h"
#include "DelveDeepTelemetrySubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepTelemetry, Log, All);

/**
 * Performance Telemetry Subsystem for DelveDeep
 * 
 * Provides comprehensive performance monitoring infrastructure including:
 * - Frame performance tracking (FPS, frame time, spike detection)
 * - System profiling with budget tracking
 * - Memory tracking and leak detection
 * - Performance baseline capture and comparison
 * - Real-time performance overlay
 * - Profiling sessions with detailed metrics
 * 
 * Designed for minimal runtime overhead (<0.5ms per frame) while providing
 * actionable performance insights for development and optimization.
 */
UCLASS(BlueprintType)
class DELVEDEEP_API UDelveDeepTelemetrySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UDelveDeepTelemetrySubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return !IsTemplate(); }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual UWorld* GetTickableGameObjectWorld() const override;

	/**
	 * Get current frames per second
	 * @return Current FPS value
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	float GetCurrentFPS() const;

	/**
	 * Get average FPS over recent frames
	 * @return Average FPS value
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	float GetAverageFPS() const;

	/**
	 * Get 1% low FPS (99th percentile worst frame times)
	 * @return 1% low FPS
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	float GetOnePercentLowFPS() const;

	/**
	 * Get current frame performance data
	 * @return Current frame data
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	FFramePerformanceData GetCurrentFrameData() const;

	/**
	 * Get frame time history
	 * @param NumFrames Number of recent frames to retrieve (default: 120)
	 * @return Array of frame times in milliseconds
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<float> GetFrameTimeHistory(int32 NumFrames = 120) const;

	// System Profiling

	/**
	 * Register a system with a performance budget
	 * @param SystemName Name of the system to register
	 * @param BudgetMs Performance budget in milliseconds
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void RegisterSystemBudget(FName SystemName, float BudgetMs);

	/**
	 * Load budgets from a performance budget data asset
	 * @param BudgetAsset Performance budget data asset to load
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void LoadBudgetsFromAsset(UDelveDeepPerformanceBudget* BudgetAsset);

	/**
	 * Record system execution time (typically called by SCOPE_CYCLE_COUNTER)
	 * @param SystemName Name of the system
	 * @param CycleTimeMs Execution time in milliseconds
	 */
	void RecordSystemTime(FName SystemName, double CycleTimeMs);

	/**
	 * Get performance data for a specific system
	 * @param SystemName Name of the system
	 * @return System performance data
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	FSystemPerformanceData GetSystemPerformance(FName SystemName) const;

	/**
	 * Get performance data for all registered systems
	 * @return Array of system performance data
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<FSystemPerformanceData> GetAllSystemPerformance() const;

	/**
	 * Get budget utilization percentage for a system
	 * @param SystemName Name of the system
	 * @return Budget utilization (0.0 to 1.0+, >1.0 means over budget)
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	float GetSystemBudgetUtilization(FName SystemName) const;

	/**
	 * Check if a system is currently violating its budget
	 * @param SystemName Name of the system
	 * @return True if system is over budget
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	bool IsSystemBudgetViolated(FName SystemName) const;

	/**
	 * Get budget violation history
	 * @return Array of recent budget violations
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<FBudgetViolation> GetBudgetViolationHistory() const;

	// Memory Tracking

	/**
	 * Get current memory snapshot
	 * @return Current memory snapshot
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	FMemorySnapshot GetCurrentMemorySnapshot() const;

	/**
	 * Get memory usage for a specific system
	 * @param SystemName Name of the system
	 * @return Memory usage in bytes
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	uint64 GetSystemMemoryUsage(FName SystemName) const;

	/**
	 * Track memory allocation for a system
	 * @param SystemName Name of the system
	 * @param AllocationSize Size of allocation in bytes
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void TrackSystemAllocation(FName SystemName, int64 AllocationSize);

	/**
	 * Track memory deallocation for a system
	 * @param SystemName Name of the system
	 * @param DeallocationSize Size of deallocation in bytes
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void TrackSystemDeallocation(FName SystemName, int64 DeallocationSize);

	/**
	 * Check if a memory leak is detected
	 * @return True if leak detected
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	bool IsMemoryLeakDetected() const;

	/**
	 * Get memory growth rate in MB per minute
	 * @return Growth rate
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	float GetMemoryGrowthRate() const;

	/**
	 * Get peak memory usage
	 * @return Peak memory in bytes
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	int64 GetPeakMemoryUsage() const;

private:
	// Frame tracking
	FFramePerformanceTracker FrameTracker;

	// System profiling
	FSystemProfiler SystemProfiler;

	// Memory tracking
	FMemoryTracker MemoryTracker;

	// Budget configuration
	UPROPERTY()
	UDelveDeepPerformanceBudget* CurrentBudgetAsset;

	// Telemetry state
	bool bTelemetryEnabled = true;
	bool bInitialized = false;

	/**
	 * Register default system budgets
	 */
	void RegisterDefaultBudgets();
};
