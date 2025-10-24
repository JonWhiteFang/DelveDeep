// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepAssetLoadTracker.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "Misc/Paths.h"

FDelveDeepAssetLoadTracker::FDelveDeepAssetLoadTracker()
{
	LoadHistory.Reserve(MaxHistorySize);
}

void FDelveDeepAssetLoadTracker::RecordAssetLoad(const FString& AssetPath, float LoadTimeMs, uint64 AssetSize, bool bSynchronous)
{
	if (AssetPath.IsEmpty())
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Attempted to record asset load with empty path"));
		return;
	}

	if (LoadTimeMs < 0.0f)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Invalid load time for asset '%s': %.2fms (must be non-negative)"),
			*AssetPath, LoadTimeMs);
		return;
	}

	// Determine asset type from path
	const FName AssetType = DetermineAssetType(AssetPath);

	// Create load record
	FAssetLoadRecord Record(AssetPath, AssetType, LoadTimeMs, AssetSize, bSynchronous);

	// Add to history
	LoadHistory.Add(Record);

	// Limit history size
	if (LoadHistory.Num() > MaxHistorySize)
	{
		LoadHistory.RemoveAt(0);
	}

	// Update statistics
	UpdateStatistics(AssetType, LoadTimeMs, AssetSize, bSynchronous);

	// Check for slow loads
	CheckSlowLoad(AssetPath, LoadTimeMs);

	UE_LOG(LogDelveDeepTelemetry, Verbose,
		TEXT("Recorded asset load: %s (Type: %s, Time: %.2fms, Size: %llu bytes, %s)"),
		*AssetPath,
		*AssetType.ToString(),
		LoadTimeMs,
		AssetSize,
		bSynchronous ? TEXT("Sync") : TEXT("Async"));
}

FAssetLoadStatistics FDelveDeepAssetLoadTracker::GetAssetLoadStatistics(FName AssetType) const
{
	const FAssetLoadStatistics* Stats = TypeStatistics.Find(AssetType);
	if (Stats)
	{
		return *Stats;
	}

	// Return empty statistics if type not found
	FAssetLoadStatistics EmptyStats;
	EmptyStats.AssetType = AssetType;
	return EmptyStats;
}

TArray<FAssetLoadStatistics> FDelveDeepAssetLoadTracker::GetAllAssetLoadStatistics() const
{
	TArray<FAssetLoadStatistics> AllStats;
	AllStats.Reserve(TypeStatistics.Num());

	for (const auto& Pair : TypeStatistics)
	{
		AllStats.Add(Pair.Value);
	}

	// Sort by total loads (descending)
	AllStats.Sort([](const FAssetLoadStatistics& A, const FAssetLoadStatistics& B)
	{
		return A.TotalLoads > B.TotalLoads;
	});

	return AllStats;
}

TArray<FAssetLoadRecord> FDelveDeepAssetLoadTracker::GetRecentAssetLoads(int32 Count) const
{
	TArray<FAssetLoadRecord> RecentLoads;

	const int32 StartIndex = FMath::Max(0, LoadHistory.Num() - Count);
	const int32 NumToReturn = FMath::Min(Count, LoadHistory.Num());

	RecentLoads.Reserve(NumToReturn);

	for (int32 i = StartIndex; i < LoadHistory.Num(); ++i)
	{
		RecentLoads.Add(LoadHistory[i]);
	}

	return RecentLoads;
}

TArray<FAssetLoadRecord> FDelveDeepAssetLoadTracker::GetSlowestAssetLoads(int32 Count) const
{
	// Copy load history
	TArray<FAssetLoadRecord> SortedLoads = LoadHistory;

	// Sort by load time (descending)
	SortedLoads.Sort([](const FAssetLoadRecord& A, const FAssetLoadRecord& B)
	{
		return A.LoadTimeMs > B.LoadTimeMs;
	});

	// Return top N
	const int32 NumToReturn = FMath::Min(Count, SortedLoads.Num());
	TArray<FAssetLoadRecord> SlowestLoads;
	SlowestLoads.Reserve(NumToReturn);

	for (int32 i = 0; i < NumToReturn; ++i)
	{
		SlowestLoads.Add(SortedLoads[i]);
	}

	return SlowestLoads;
}

int32 FDelveDeepAssetLoadTracker::GetTotalSlowLoads() const
{
	int32 TotalSlowLoads = 0;

	for (const auto& Pair : TypeStatistics)
	{
		TotalSlowLoads += Pair.Value.SlowLoadCount;
	}

	return TotalSlowLoads;
}

void FDelveDeepAssetLoadTracker::ResetStatistics()
{
	LoadHistory.Empty();
	TypeStatistics.Empty();

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Asset load tracker statistics reset"));
}

FName FDelveDeepAssetLoadTracker::DetermineAssetType(const FString& AssetPath) const
{
	// Extract file extension
	const FString Extension = FPaths::GetExtension(AssetPath).ToLower();

	// Map common extensions to asset types
	if (Extension == TEXT("uasset") || Extension == TEXT("umap"))
	{
		// Check for specific asset types in path
		if (AssetPath.Contains(TEXT("/Textures/")) || AssetPath.Contains(TEXT("_T.")))
		{
			return TEXT("Texture");
		}
		else if (AssetPath.Contains(TEXT("/Meshes/")) || AssetPath.Contains(TEXT("_SM.")) || AssetPath.Contains(TEXT("_SK.")))
		{
			return TEXT("Mesh");
		}
		else if (AssetPath.Contains(TEXT("/Sounds/")) || AssetPath.Contains(TEXT("_Cue")))
		{
			return TEXT("Sound");
		}
		else if (AssetPath.Contains(TEXT("/Data/")) || AssetPath.Contains(TEXT("DA_")) || AssetPath.Contains(TEXT("DT_")))
		{
			return TEXT("DataAsset");
		}
		else if (AssetPath.Contains(TEXT("/Materials/")) || AssetPath.Contains(TEXT("_M.")) || AssetPath.Contains(TEXT("_MI.")))
		{
			return TEXT("Material");
		}
		else if (AssetPath.Contains(TEXT("/Blueprints/")) || AssetPath.Contains(TEXT("BP_")))
		{
			return TEXT("Blueprint");
		}
		else if (AssetPath.Contains(TEXT("/Animations/")) || AssetPath.Contains(TEXT("_Anim")))
		{
			return TEXT("Animation");
		}
		else if (AssetPath.Contains(TEXT("/Particles/")) || AssetPath.Contains(TEXT("_P.")))
		{
			return TEXT("Particle");
		}
		else if (Extension == TEXT("umap"))
		{
			return TEXT("Map");
		}
		else
		{
			return TEXT("Asset");
		}
	}
	else if (Extension == TEXT("png") || Extension == TEXT("jpg") || Extension == TEXT("jpeg") || Extension == TEXT("tga") || Extension == TEXT("bmp"))
	{
		return TEXT("Texture");
	}
	else if (Extension == TEXT("fbx") || Extension == TEXT("obj"))
	{
		return TEXT("Mesh");
	}
	else if (Extension == TEXT("wav") || Extension == TEXT("mp3") || Extension == TEXT("ogg"))
	{
		return TEXT("Sound");
	}
	else if (Extension == TEXT("csv") || Extension == TEXT("json"))
	{
		return TEXT("DataTable");
	}
	else
	{
		return TEXT("Unknown");
	}
}

void FDelveDeepAssetLoadTracker::UpdateStatistics(FName AssetType, float LoadTimeMs, uint64 AssetSize, bool bSynchronous)
{
	FAssetLoadStatistics& Stats = TypeStatistics.FindOrAdd(AssetType);

	// Initialize if first load of this type
	if (Stats.TotalLoads == 0)
	{
		Stats.AssetType = AssetType;
		Stats.MinLoadTimeMs = LoadTimeMs;
		Stats.MaxLoadTimeMs = LoadTimeMs;
		Stats.AverageLoadTimeMs = LoadTimeMs;
	}
	else
	{
		// Update min/max
		Stats.MinLoadTimeMs = FMath::Min(Stats.MinLoadTimeMs, LoadTimeMs);
		Stats.MaxLoadTimeMs = FMath::Max(Stats.MaxLoadTimeMs, LoadTimeMs);

		// Update average (incremental)
		Stats.AverageLoadTimeMs = ((Stats.AverageLoadTimeMs * Stats.TotalLoads) + LoadTimeMs) / (Stats.TotalLoads + 1);
	}

	// Update counts
	Stats.TotalLoads++;
	if (bSynchronous)
	{
		Stats.SynchronousLoads++;
	}
	else
	{
		Stats.AsynchronousLoads++;
	}

	// Update size
	Stats.TotalSize += AssetSize;

	// Check for slow load
	if (LoadTimeMs > SlowLoadThresholdMs)
	{
		Stats.SlowLoadCount++;
	}
}

void FDelveDeepAssetLoadTracker::CheckSlowLoad(const FString& AssetPath, float LoadTimeMs)
{
	if (LoadTimeMs > SlowLoadThresholdMs)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Slow asset load detected: %s (%.2fms, threshold: %.2fms)"),
			*AssetPath,
			LoadTimeMs,
			SlowLoadThresholdMs);
	}
}
