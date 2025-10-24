// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepGameplayMetrics.h"
#include "DelveDeepTelemetrySubsystem.h"

FDelveDeepGameplayMetrics::FDelveDeepGameplayMetrics()
	: FrameCounter(0)
{
	InitializeRecommendedLimits();
}

void FDelveDeepGameplayMetrics::TrackEntityCount(FName EntityType, int32 Count)
{
	if (Count < 0)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Invalid entity count for '%s': %d (must be non-negative)"),
			*EntityType.ToString(), Count);
		return;
	}

	FEntityCountData& CountData = EntityCounts.FindOrAdd(EntityType);
	CountData.UpdateCount(Count);
}

int32 FDelveDeepGameplayMetrics::GetEntityCount(FName EntityType) const
{
	const FEntityCountData* CountData = EntityCounts.Find(EntityType);
	return CountData ? CountData->CurrentCount : 0;
}

int32 FDelveDeepGameplayMetrics::GetPeakEntityCount(FName EntityType) const
{
	const FEntityCountData* CountData = EntityCounts.Find(EntityType);
	return CountData ? CountData->PeakCount : 0;
}

float FDelveDeepGameplayMetrics::GetAverageEntityCount(FName EntityType) const
{
	const FEntityCountData* CountData = EntityCounts.Find(EntityType);
	return CountData ? CountData->AverageCount : 0.0f;
}

bool FDelveDeepGameplayMetrics::IsEntityCountExceedingLimit(FName EntityType) const
{
	const int32 CurrentCount = GetEntityCount(EntityType);
	const int32 Limit = GetRecommendedLimit(EntityType);

	return Limit > 0 && CurrentCount > Limit;
}

int32 FDelveDeepGameplayMetrics::GetRecommendedLimit(FName EntityType) const
{
	const int32* Limit = RecommendedLimits.Find(EntityType);
	return Limit ? *Limit : 0;
}

void FDelveDeepGameplayMetrics::ResetStatistics()
{
	for (auto& Pair : EntityCounts)
	{
		Pair.Value.Reset();
	}

	FrameCounter = 0;

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Gameplay metrics statistics reset"));
}

void FDelveDeepGameplayMetrics::UpdateFrame()
{
	FrameCounter++;

	// Only process every UpdateInterval frames to minimize overhead
	if (FrameCounter % UpdateInterval == 0)
	{
		CheckEntityLimits();
	}
}

void FDelveDeepGameplayMetrics::InitializeRecommendedLimits()
{
	// Set recommended limits for common entity types
	// These are conservative limits to maintain 60+ FPS

	RecommendedLimits.Add(TEXT("Monsters"), 100);
	RecommendedLimits.Add(TEXT("Projectiles"), 200);
	RecommendedLimits.Add(TEXT("Particles"), 500);
	RecommendedLimits.Add(TEXT("Effects"), 300);
	RecommendedLimits.Add(TEXT("Items"), 150);
	RecommendedLimits.Add(TEXT("Pickups"), 100);
	RecommendedLimits.Add(TEXT("Traps"), 50);
	RecommendedLimits.Add(TEXT("Hazards"), 50);

	UE_LOG(LogDelveDeepTelemetry, Verbose,
		TEXT("Initialized %d entity type recommended limits"), RecommendedLimits.Num());
}

void FDelveDeepGameplayMetrics::CheckEntityLimits()
{
	for (const auto& Pair : EntityCounts)
	{
		const FName EntityType = Pair.Key;
		const FEntityCountData& CountData = Pair.Value;

		if (IsEntityCountExceedingLimit(EntityType))
		{
			const int32 Limit = GetRecommendedLimit(EntityType);
			UE_LOG(LogDelveDeepTelemetry, Warning,
				TEXT("Entity count for '%s' exceeds recommended limit: %d / %d (%.1f%%)"),
				*EntityType.ToString(),
				CountData.CurrentCount,
				Limit,
				(static_cast<float>(CountData.CurrentCount) / Limit) * 100.0f);
		}
	}
}
