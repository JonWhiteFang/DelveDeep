// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepPerformanceReport.h"

FString FPerformanceReport::GenerateFormattedReport() const
{
	FString Report;
	
	// Header
	Report += TEXT("=================================================\n");
	Report += TEXT("           PERFORMANCE REPORT\n");
	Report += TEXT("=================================================\n\n");
	
	// Metadata
	Report += FString::Printf(TEXT("Generated: %s\n"), *GenerationTime.ToString());
	Report += FString::Printf(TEXT("Duration: %.2f seconds\n"), DurationSeconds);
	Report += FString::Printf(TEXT("Build Version: %s\n"), *BuildVersion);
	Report += FString::Printf(TEXT("Map: %s\n\n"), *MapName);
	
	// Frame Performance
	Report += TEXT("-------------------------------------------------\n");
	Report += TEXT("FRAME PERFORMANCE\n");
	Report += TEXT("-------------------------------------------------\n");
	Report += FString::Printf(TEXT("Average FPS: %.2f\n"), AverageFPS);
	Report += FString::Printf(TEXT("Min FPS: %.2f\n"), MinFPS);
	Report += FString::Printf(TEXT("Max FPS: %.2f\n"), MaxFPS);
	Report += FString::Printf(TEXT("1%% Low FPS: %.2f\n"), OnePercentLowFPS);
	Report += FString::Printf(TEXT("0.1%% Low FPS: %.2f\n"), PointOnePercentLowFPS);
	Report += FString::Printf(TEXT("Median Frame Time: %.2f ms\n"), MedianFrameTimeMs);
	Report += FString::Printf(TEXT("95th Percentile Frame Time: %.2f ms\n"), Percentile95FrameTimeMs);
	Report += FString::Printf(TEXT("99th Percentile Frame Time: %.2f ms\n"), Percentile99FrameTimeMs);
	Report += FString::Printf(TEXT("Total Frames: %d\n"), TotalFrames);
	Report += FString::Printf(TEXT("Spike Count: %d\n\n"), SpikeCount);
	
	// System Performance
	if (SystemBreakdown.Num() > 0)
	{
		Report += TEXT("-------------------------------------------------\n");
		Report += TEXT("SYSTEM PERFORMANCE\n");
		Report += TEXT("-------------------------------------------------\n");
		
		// Sort systems by average time (descending)
		TArray<FSystemPerformanceData> SortedSystems = SystemBreakdown;
		SortedSystems.Sort([](const FSystemPerformanceData& A, const FSystemPerformanceData& B)
		{
			return A.AverageTimeMs > B.AverageTimeMs;
		});
		
		for (const FSystemPerformanceData& System : SortedSystems)
		{
			float Utilization = System.BudgetTimeMs > 0.0 ? 
				(System.AverageTimeMs / System.BudgetTimeMs) * 100.0f : 0.0f;
			
			Report += FString::Printf(TEXT("%-25s Avg: %6.3f ms  Peak: %6.3f ms  Budget: %5.2f ms  Util: %5.1f%%\n"),
				*System.SystemName.ToString(),
				System.AverageTimeMs,
				System.PeakTimeMs,
				System.BudgetTimeMs,
				Utilization);
		}
		
		Report += FString::Printf(TEXT("\nTotal Budget Violations: %d\n\n"), TotalBudgetViolations);
	}
	
	// Memory Statistics
	Report += TEXT("-------------------------------------------------\n");
	Report += TEXT("MEMORY STATISTICS\n");
	Report += TEXT("-------------------------------------------------\n");
	Report += FString::Printf(TEXT("Average Memory: %.2f MB\n"), AverageMemoryUsage / (1024.0 * 1024.0));
	Report += FString::Printf(TEXT("Peak Memory: %.2f MB\n"), PeakMemoryUsage / (1024.0 * 1024.0));
	Report += FString::Printf(TEXT("Min Memory: %.2f MB\n"), MinMemoryUsage / (1024.0 * 1024.0));
	Report += FString::Printf(TEXT("Growth Rate: %.2f MB/min\n"), MemoryGrowthRate);
	Report += FString::Printf(TEXT("Memory Leak Detected: %s\n\n"), bMemoryLeakDetected ? TEXT("YES") : TEXT("NO"));
	
	// Gameplay Metrics
	Report += TEXT("-------------------------------------------------\n");
	Report += TEXT("GAMEPLAY METRICS\n");
	Report += TEXT("-------------------------------------------------\n");
	Report += FString::Printf(TEXT("Peak Monster Count: %d\n"), PeakMonsterCount);
	Report += FString::Printf(TEXT("Peak Projectile Count: %d\n"), PeakProjectileCount);
	Report += FString::Printf(TEXT("Peak Particle Count: %d\n"), PeakParticleCount);
	Report += FString::Printf(TEXT("Total Events Processed: %d\n\n"), TotalEventsProcessed);
	
	Report += TEXT("=================================================\n");
	
	return Report;
}

FString FPerformanceReport::GenerateSummary() const
{
	return FString::Printf(
		TEXT("Performance Report: %.2f FPS avg, %.2f ms frame time, %d spikes, %.2f MB memory, %d budget violations"),
		AverageFPS,
		MedianFrameTimeMs,
		SpikeCount,
		AverageMemoryUsage / (1024.0 * 1024.0),
		TotalBudgetViolations);
}
