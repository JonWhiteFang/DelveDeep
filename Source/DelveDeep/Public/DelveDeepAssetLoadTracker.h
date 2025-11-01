// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepAssetLoadTracker.generated.h"

/**
 * Asset load record for tracking asset loading performance
 */
USTRUCT()
struct DELVEDEEP_API FAssetLoadRecord
{
	GENERATED_BODY()

	/** Path to the loaded asset */
	UPROPERTY()
	FString AssetPath;

	/** Type of asset (Texture, Mesh, Sound, DataAsset, etc.) */
	UPROPERTY()
	FName AssetType;

	/** Load time in milliseconds */
	UPROPERTY()
	float LoadTimeMs = 0.0f;

	/** Asset size in bytes */
	UPROPERTY()
	int64 AssetSize = 0;

	/** Whether this was a synchronous load */
	UPROPERTY()
	bool bSynchronous = true;

	/** Timestamp when the load occurred */
	UPROPERTY()
	FDateTime Timestamp;

	FAssetLoadRecord()
		: AssetPath(TEXT(""))
		, AssetType(NAME_None)
		, LoadTimeMs(0.0f)
		, AssetSize(0)
		, bSynchronous(true)
		, Timestamp(FDateTime::Now())
	{
	}

	FAssetLoadRecord(const FString& InPath, FName InType, float InLoadTime, int64 InSize, bool bInSynchronous)
		: AssetPath(InPath)
		, AssetType(InType)
		, LoadTimeMs(InLoadTime)
		, AssetSize(InSize)
		, bSynchronous(bInSynchronous)
		, Timestamp(FDateTime::Now())
	{
	}
};

/**
 * Asset load statistics for a specific asset type
 */
USTRUCT()
struct DELVEDEEP_API FAssetLoadStatistics
{
	GENERATED_BODY()

	/** Asset type */
	UPROPERTY()
	FName AssetType;

	/** Total number of loads */
	UPROPERTY()
	int32 TotalLoads = 0;

	/** Number of synchronous loads */
	UPROPERTY()
	int32 SynchronousLoads = 0;

	/** Number of asynchronous loads */
	UPROPERTY()
	int32 AsynchronousLoads = 0;

	/** Average load time in milliseconds */
	UPROPERTY()
	float AverageLoadTimeMs = 0.0f;

	/** Minimum load time in milliseconds */
	UPROPERTY()
	float MinLoadTimeMs = 0.0f;

	/** Maximum load time in milliseconds */
	UPROPERTY()
	float MaxLoadTimeMs = 0.0f;

	/** Total size of all loaded assets in bytes */
	UPROPERTY()
	int64 TotalSize = 0;

	/** Number of slow loads (>100ms) */
	UPROPERTY()
	int32 SlowLoadCount = 0;
};

/**
 * Asset loading performance tracker
 * 
 * Tracks asset load operations and provides statistics for optimization.
 * Categorizes assets by type and tracks synchronous vs asynchronous loads.
 * Logs warnings for slow loads exceeding 100ms.
 */
class DELVEDEEP_API FDelveDeepAssetLoadTracker
{
public:
	FDelveDeepAssetLoadTracker();

	/**
	 * Record an asset load operation
	 * @param AssetPath Path to the loaded asset
	 * @param LoadTimeMs Load time in milliseconds
	 * @param AssetSize Asset size in bytes
	 * @param bSynchronous Whether this was a synchronous load
	 */
	void RecordAssetLoad(const FString& AssetPath, float LoadTimeMs, int64 AssetSize, bool bSynchronous = true);

	/**
	 * Get asset load statistics for a specific type
	 * @param AssetType Type of asset
	 * @return Statistics for that asset type
	 */
	FAssetLoadStatistics GetAssetLoadStatistics(FName AssetType) const;

	/**
	 * Get asset load statistics for all types
	 * @return Array of statistics for all asset types
	 */
	TArray<FAssetLoadStatistics> GetAllAssetLoadStatistics() const;

	/**
	 * Get recent asset load records
	 * @param Count Number of recent records to retrieve (default: 100)
	 * @return Array of recent load records
	 */
	TArray<FAssetLoadRecord> GetRecentAssetLoads(int32 Count = 100) const;

	/**
	 * Get slowest asset loads
	 * @param Count Number of slowest loads to retrieve (default: 10)
	 * @return Array of slowest load records
	 */
	TArray<FAssetLoadRecord> GetSlowestAssetLoads(int32 Count = 10) const;

	/**
	 * Get total number of asset loads
	 * @return Total load count
	 */
	int32 GetTotalAssetLoads() const { return LoadHistory.Num(); }

	/**
	 * Get total number of slow loads (>100ms)
	 * @return Slow load count
	 */
	int32 GetTotalSlowLoads() const;

	/**
	 * Reset all statistics
	 */
	void ResetStatistics();

private:
	/** History of asset loads (limited to MaxHistorySize) */
	TArray<FAssetLoadRecord> LoadHistory;

	/** Statistics per asset type */
	TMap<FName, FAssetLoadStatistics> TypeStatistics;

	/** Maximum history size */
	static constexpr int32 MaxHistorySize = 1000;

	/** Slow load threshold in milliseconds */
	static constexpr float SlowLoadThresholdMs = 100.0f;

	/**
	 * Determine asset type from path
	 * @param AssetPath Path to the asset
	 * @return Asset type name
	 */
	FName DetermineAssetType(const FString& AssetPath) const;

	/**
	 * Update statistics for an asset type
	 * @param AssetType Type of asset
	 * @param LoadTimeMs Load time in milliseconds
	 * @param AssetSize Asset size in bytes
	 * @param bSynchronous Whether this was a synchronous load
	 */
	void UpdateStatistics(FName AssetType, float LoadTimeMs, int64 AssetSize, bool bSynchronous);

	/**
	 * Check if load is slow and log warning if needed
	 * @param AssetPath Path to the asset
	 * @param LoadTimeMs Load time in milliseconds
	 */
	void CheckSlowLoad(const FString& AssetPath, float LoadTimeMs);
};
