// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DelveDeepStats.h"
#include "DelveDeepFramePerformanceTracker.h"
#include "DelveDeepSystemProfiler.h"
#include "DelveDeepMemoryTracker.h"
#include "DelveDeepPerformanceBudget.h"
#include "DelveDeepPerformanceBaseline.h"
#include "DelveDeepPerformanceReport.h"
#include "DelveDeepPerformanceOverlay.h"
#include "DelveDeepProfilingSession.h"
#include "DelveDeepGameplayMetrics.h"
#include "DelveDeepAssetLoadTracker.h"
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

	// Baseline Management

	/**
	 * Capture a performance baseline with the given name
	 * @param BaselineName Name for this baseline
	 * @return True if baseline was captured successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool CaptureBaseline(FName BaselineName);

	/**
	 * Compare current performance to a saved baseline
	 * @param BaselineName Name of the baseline to compare against
	 * @param OutComparison Comparison results
	 * @return True if comparison was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool CompareToBaseline(FName BaselineName, FPerformanceComparison& OutComparison);

	/**
	 * Get a list of all available baseline names
	 * @return Array of baseline names
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<FName> GetAvailableBaselines() const;

	/**
	 * Get a specific baseline by name
	 * @param BaselineName Name of the baseline
	 * @param OutBaseline The baseline data
	 * @return True if baseline was found
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool GetBaseline(FName BaselineName, FPerformanceBaseline& OutBaseline) const;

	/**
	 * Save a baseline to disk
	 * @param BaselineName Name of the baseline to save
	 * @param FilePath Path where to save the baseline (optional, uses default if empty)
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool SaveBaseline(FName BaselineName, const FString& FilePath = TEXT(""));

	/**
	 * Load a baseline from disk
	 * @param BaselineName Name to assign to the loaded baseline
	 * @param FilePath Path to the baseline file
	 * @return True if load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool LoadBaseline(FName BaselineName, const FString& FilePath);

	/**
	 * Delete a baseline
	 * @param BaselineName Name of the baseline to delete
	 * @return True if baseline was deleted
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool DeleteBaseline(FName BaselineName);

	// Performance Reporting

	/**
	 * Generate a performance report for the specified duration
	 * @param OutReport Generated performance report
	 * @param DurationSeconds Duration to report on (default: 300 seconds = 5 minutes)
	 * @return True if report was generated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool GeneratePerformanceReport(FPerformanceReport& OutReport, float DurationSeconds = 300.0f);

	/**
	 * Export a performance report to CSV format
	 * @param Report The report to export
	 * @param FilePath Path where to save the CSV file
	 * @return True if export was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool ExportReportToCSV(const FPerformanceReport& Report, const FString& FilePath);

	/**
	 * Export a performance report to JSON format
	 * @param Report The report to export
	 * @param FilePath Path where to save the JSON file
	 * @return True if export was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool ExportReportToJSON(const FPerformanceReport& Report, const FString& FilePath);

	// Performance Overlay

	/**
	 * Enable the performance overlay
	 * @param Mode Display mode (Minimal, Standard, or Detailed)
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void EnablePerformanceOverlay(EOverlayMode Mode = EOverlayMode::Standard);

	/**
	 * Disable the performance overlay
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void DisablePerformanceOverlay();

	/**
	 * Check if the performance overlay is enabled
	 * @return True if overlay is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	bool IsOverlayEnabled() const;

	/**
	 * Set the overlay display mode
	 * @param Mode Display mode to set
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void SetOverlayMode(EOverlayMode Mode);

	/**
	 * Get the current overlay display mode
	 * @return Current display mode
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	EOverlayMode GetOverlayMode() const;

	/**
	 * Render the performance overlay (typically called from HUD)
	 * @param Canvas Canvas to draw on
	 */
	void RenderPerformanceOverlay(UCanvas* Canvas);

	// Profiling Sessions

	/**
	 * Start a profiling session with detailed metric collection
	 * @param SessionName Name for this profiling session
	 * @return True if session was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool StartProfilingSession(FName SessionName);

	/**
	 * Stop the current profiling session
	 * @return True if session was stopped successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool StopProfilingSession();

	/**
	 * Check if a profiling session is currently active
	 * @return True if session is active
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	bool IsProfilingActive() const;

	/**
	 * Get the current profiling session
	 * @param OutSession Current session data
	 * @return True if session is active
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool GetCurrentSession(FProfilingSession& OutSession) const;

	/**
	 * Generate a report from the current profiling session
	 * @param OutReport Generated report
	 * @return True if report was generated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool GenerateProfilingReport(FProfilingSessionReport& OutReport);

	/**
	 * Save the current profiling session to disk
	 * @param FilePath Path where to save the session (optional, uses default if empty)
	 * @return True if save was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	bool SaveProfilingSession(const FString& FilePath = TEXT(""));

	// Gameplay Metrics

	/**
	 * Track entity count for a specific type
	 * @param EntityType Type of entity (e.g., "Monsters", "Projectiles")
	 * @param Count Current count
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void TrackEntityCount(FName EntityType, int32 Count);

	/**
	 * Get current entity count
	 * @param EntityType Type of entity
	 * @return Current count
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	int32 GetEntityCount(FName EntityType) const;

	/**
	 * Get peak entity count
	 * @param EntityType Type of entity
	 * @return Peak count
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	int32 GetPeakEntityCount(FName EntityType) const;

	/**
	 * Get average entity count over last minute
	 * @param EntityType Type of entity
	 * @return Average count
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	float GetAverageEntityCount(FName EntityType) const;

	/**
	 * Check if entity count exceeds recommended limit
	 * @param EntityType Type of entity
	 * @return True if count exceeds limit
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	bool IsEntityCountExceedingLimit(FName EntityType) const;

	/**
	 * Get recommended limit for entity type
	 * @param EntityType Type of entity
	 * @return Recommended limit (0 if no limit set)
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	int32 GetRecommendedEntityLimit(FName EntityType) const;

	// Asset Loading Tracking

	/**
	 * Record an asset load operation
	 * @param AssetPath Path to the loaded asset
	 * @param LoadTimeMs Load time in milliseconds
	 * @param AssetSize Asset size in bytes
	 * @param bSynchronous Whether this was a synchronous load
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	void RecordAssetLoad(const FString& AssetPath, float LoadTimeMs, int64 AssetSize, bool bSynchronous = true);

	/**
	 * Get asset load statistics for a specific type
	 * @param AssetType Type of asset
	 * @return Statistics for that asset type
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	FAssetLoadStatistics GetAssetLoadStatistics(FName AssetType) const;

	/**
	 * Get asset load statistics for all types
	 * @return Array of statistics for all asset types
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<FAssetLoadStatistics> GetAllAssetLoadStatistics() const;

	/**
	 * Get recent asset load records
	 * @param Count Number of recent records to retrieve (default: 100)
	 * @return Array of recent load records
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<FAssetLoadRecord> GetRecentAssetLoads(int32 Count = 100) const;

	/**
	 * Get slowest asset loads
	 * @param Count Number of slowest loads to retrieve (default: 10)
	 * @return Array of slowest load records
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
	TArray<FAssetLoadRecord> GetSlowestAssetLoads(int32 Count = 10) const;

	/**
	 * Get total number of asset loads
	 * @return Total load count
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	int32 GetTotalAssetLoads() const;

	/**
	 * Get total number of slow loads (>100ms)
	 * @return Slow load count
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
	int32 GetTotalSlowLoads() const;

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
	// In shipping builds, telemetry is disabled by default to minimize overhead
#if UE_BUILD_SHIPPING
	bool bTelemetryEnabled = false;
#else
	bool bTelemetryEnabled = true;
#endif
	bool bInitialized = false;

	// Baseline storage
	TMap<FName, FPerformanceBaseline> Baselines;

	// Performance overlay
	TSharedPtr<FDelveDeepPerformanceOverlay> PerformanceOverlay;
	bool bOverlayEnabled = false;

	// Profiling session
	FProfilingSession CurrentSession;
	bool bProfilingActive = false;
	int32 ProfilingFrameCounter = 0;

	// Gameplay metrics
	FDelveDeepGameplayMetrics GameplayMetrics;

	// Asset load tracking
	FDelveDeepAssetLoadTracker AssetLoadTracker;

	/**
	 * Register default system budgets
	 */
	void RegisterDefaultBudgets();

	/**
	 * Get the default baseline save directory
	 * @return Default directory path
	 */
	FString GetDefaultBaselineDirectory() const;

	/**
	 * Validate a baseline for compatibility with current build
	 * @param Baseline The baseline to validate
	 * @param Context Validation context for error reporting
	 * @return True if baseline is valid
	 */
	bool ValidateBaseline(const FPerformanceBaseline& Baseline, FValidationContext& Context) const;

	/**
	 * Get the default profiling save directory
	 * @return Default directory path
	 */
	FString GetDefaultProfilingDirectory() const;

	/**
	 * Format bytes to human-readable string
	 * @param Bytes Number of bytes
	 * @return Formatted string (e.g., "1.5 MB")
	 */
	FString FormatBytes(uint64 Bytes) const;
};
