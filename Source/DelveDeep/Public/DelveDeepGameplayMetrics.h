// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepGameplayMetrics.generated.h"

/**
 * Entity count data for gameplay metrics
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FEntityCountData
{
	GENERATED_BODY()

	/** Current entity count */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 CurrentCount = 0;

	/** Peak entity count */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 PeakCount = 0;

	/** Average count over last minute */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageCount = 0.0f;

	/** History of counts for averaging (last 60 samples) */
	TArray<int32> CountHistory;

	/** Maximum history size */
	static constexpr int32 MaxHistorySize = 60;

	/**
	 * Update entity count
	 * @param NewCount New entity count
	 */
	void UpdateCount(int32 NewCount)
	{
		CurrentCount = NewCount;
		PeakCount = FMath::Max(PeakCount, NewCount);

		// Add to history
		CountHistory.Add(NewCount);
		if (CountHistory.Num() > MaxHistorySize)
		{
			CountHistory.RemoveAt(0);
		}

		// Calculate average
		if (CountHistory.Num() > 0)
		{
			int32 Total = 0;
			for (int32 Count : CountHistory)
			{
				Total += Count;
			}
			AverageCount = static_cast<float>(Total) / CountHistory.Num();
		}
	}

	/**
	 * Reset statistics
	 */
	void Reset()
	{
		CurrentCount = 0;
		PeakCount = 0;
		AverageCount = 0.0f;
		CountHistory.Empty();
	}
};

/**
 * Gameplay metrics tracker for performance correlation
 * 
 * Tracks entity counts (monsters, projectiles, particles) and correlates
 * them with frame time to identify performance relationships.
 * 
 * Updates every 10 frames to minimize overhead.
 */
class DELVEDEEP_API FDelveDeepGameplayMetrics
{
public:
	FDelveDeepGameplayMetrics();

	/**
	 * Track entity count for a specific type
	 * @param EntityType Type of entity (e.g., "Monsters", "Projectiles")
	 * @param Count Current count
	 */
	void TrackEntityCount(FName EntityType, int32 Count);

	/**
	 * Get current entity count
	 * @param EntityType Type of entity
	 * @return Current count
	 */
	int32 GetEntityCount(FName EntityType) const;

	/**
	 * Get peak entity count
	 * @param EntityType Type of entity
	 * @return Peak count
	 */
	int32 GetPeakEntityCount(FName EntityType) const;

	/**
	 * Get average entity count over last minute
	 * @param EntityType Type of entity
	 * @return Average count
	 */
	float GetAverageEntityCount(FName EntityType) const;

	/**
	 * Get all entity count data
	 * @return Map of entity types to count data
	 */
	const TMap<FName, FEntityCountData>& GetAllEntityCounts() const { return EntityCounts; }

	/**
	 * Check if entity count exceeds recommended limit
	 * @param EntityType Type of entity
	 * @return True if count exceeds limit
	 */
	bool IsEntityCountExceedingLimit(FName EntityType) const;

	/**
	 * Get recommended limit for entity type
	 * @param EntityType Type of entity
	 * @return Recommended limit
	 */
	int32 GetRecommendedLimit(FName EntityType) const;

	/**
	 * Reset all statistics
	 */
	void ResetStatistics();

	/**
	 * Update frame (called every frame, but only processes every 10 frames)
	 */
	void UpdateFrame();

private:
	/** Entity count tracking */
	TMap<FName, FEntityCountData> EntityCounts;

	/** Recommended limits for entity types */
	TMap<FName, int32> RecommendedLimits;

	/** Frame counter for update throttling */
	int32 FrameCounter = 0;

	/** Update interval (every 10 frames) */
	static constexpr int32 UpdateInterval = 10;

	/**
	 * Initialize recommended limits
	 */
	void InitializeRecommendedLimits();

	/**
	 * Check and log warnings for entities exceeding limits
	 */
	void CheckEntityLimits();
};
