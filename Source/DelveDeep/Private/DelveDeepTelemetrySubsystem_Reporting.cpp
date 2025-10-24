// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTelemetrySubsystem.h"
#include "DelveDeepValidation.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"
#include "Misc/App.h"
#include "Misc/Compression.h"

// Performance Reporting Implementation

bool UDelveDeepTelemetrySubsystem::GeneratePerformanceReport(FPerformanceReport& OutReport, float DurationSeconds)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("GeneratePerformanceReport");

	// Validate duration
	if (DurationSeconds <= 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Invalid duration: %.2f seconds"),
			DurationSeconds));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Initialize report
	OutReport = FPerformanceReport();
	OutReport.GenerationTime = FDateTime::Now();
	OutReport.DurationSeconds = DurationSeconds;
	OutReport.BuildVersion = FApp::GetBuildVersion();
	
	// Get map name
	if (UWorld* World = GetWorld())
	{
		OutReport.MapName = World->GetMapName();
	}

	// Get frame time history for the requested duration
	// Calculate number of frames based on duration (assume 60 FPS average)
	int32 NumFrames = FMath::Min(static_cast<int32>(DurationSeconds * 60.0f), 3600);
	TArray<float> FrameHistory = FrameTracker.GetFrameTimeHistory(NumFrames);
	
	if (FrameHistory.Num() == 0)
	{
		Context.AddWarning(TEXT("No frame history available"));
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("%s"), *Context.GetReport());
		return false;
	}

	OutReport.TotalFrames = FrameHistory.Num();

	// Calculate frame statistics
	OutReport.AverageFPS = FrameTracker.GetAverageFPS(NumFrames);
	OutReport.OnePercentLowFPS = FrameTracker.GetOnePercentLowFPS();
	OutReport.PointOnePercentLowFPS = FrameTracker.GetPointOnePercentLowFPS();

	// Calculate min/max FPS
	float MinFrameTime = FLT_MAX;
	float MaxFrameTime = 0.0f;
	int32 SpikeCount = 0;
	
	for (float FrameTime : FrameHistory)
	{
		MinFrameTime = FMath::Min(MinFrameTime, FrameTime);
		MaxFrameTime = FMath::Max(MaxFrameTime, FrameTime);
		
		// Count spikes (frames over 16.67ms)
		if (FrameTime > 16.67f)
		{
			SpikeCount++;
		}
	}
	
	OutReport.MinFPS = MinFrameTime > 0.0f ? 1000.0f / MinFrameTime : 0.0f;
	OutReport.MaxFPS = MaxFrameTime > 0.0f ? 1000.0f / MaxFrameTime : 0.0f;
	OutReport.SpikeCount = SpikeCount;

	// Calculate percentiles
	TArray<float> SortedFrameTimes = FrameHistory;
	SortedFrameTimes.Sort();
	
	auto GetPercentile = [&SortedFrameTimes](float Percentile) -> float
	{
		if (SortedFrameTimes.Num() == 0) return 0.0f;
		int32 Index = FMath::Clamp(
			static_cast<int32>(SortedFrameTimes.Num() * Percentile),
			0,
			SortedFrameTimes.Num() - 1);
		return SortedFrameTimes[Index];
	};
	
	OutReport.MedianFrameTimeMs = GetPercentile(0.50f);
	OutReport.Percentile95FrameTimeMs = GetPercentile(0.95f);
	OutReport.Percentile99FrameTimeMs = GetPercentile(0.99f);

	// Get system performance data
	OutReport.SystemBreakdown = SystemProfiler.GetAllSystemData();
	OutReport.TotalBudgetViolations = SystemProfiler.GetTotalViolationCount();

	// Get memory statistics
	FMemorySnapshot CurrentMemory = MemoryTracker.GetCurrentSnapshot();
	OutReport.AverageMemoryUsage = CurrentMemory.TotalMemory;
	OutReport.PeakMemoryUsage = MemoryTracker.GetPeakMemoryUsage();
	OutReport.MinMemoryUsage = CurrentMemory.TotalMemory; // TODO: Track min memory
	OutReport.MemoryGrowthRate = MemoryTracker.GetMemoryGrowthRate();
	OutReport.bMemoryLeakDetected = MemoryTracker.IsLeakDetected();

	// TODO: Get gameplay metrics when entity tracking is implemented
	OutReport.PeakMonsterCount = 0;
	OutReport.PeakProjectileCount = 0;
	OutReport.PeakParticleCount = 0;
	OutReport.TotalEventsProcessed = 0;

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Generated performance report: %.2f FPS avg, %d frames, %d spikes, %d budget violations"),
		OutReport.AverageFPS,
		OutReport.TotalFrames,
		OutReport.SpikeCount,
		OutReport.TotalBudgetViolations);

	return true;
}

bool UDelveDeepTelemetrySubsystem::ExportReportToCSV(const FPerformanceReport& Report, const FString& FilePath)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("ExportReportToCSV");

	// Validate file path
	if (FilePath.IsEmpty())
	{
		Context.AddError(TEXT("File path cannot be empty"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Ensure directory exists
	FString Directory = FPaths::GetPath(FilePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		if (!PlatformFile.CreateDirectoryTree(*Directory))
		{
			Context.AddError(FString::Printf(
				TEXT("Failed to create directory: %s"),
				*Directory));
			UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
			return false;
		}
	}

	// Build CSV content
	FString CSV;
	
	// Metadata section
	CSV += TEXT("Performance Report\n");
	CSV += FString::Printf(TEXT("Generated,%s\n"), *Report.GenerationTime.ToString());
	CSV += FString::Printf(TEXT("Duration,%.2f\n"), Report.DurationSeconds);
	CSV += FString::Printf(TEXT("Build Version,%s\n"), *Report.BuildVersion);
	CSV += FString::Printf(TEXT("Map,%s\n\n"), *Report.MapName);
	
	// Frame performance section
	CSV += TEXT("Frame Performance\n");
	CSV += TEXT("Metric,Value\n");
	CSV += FString::Printf(TEXT("Average FPS,%.2f\n"), Report.AverageFPS);
	CSV += FString::Printf(TEXT("Min FPS,%.2f\n"), Report.MinFPS);
	CSV += FString::Printf(TEXT("Max FPS,%.2f\n"), Report.MaxFPS);
	CSV += FString::Printf(TEXT("1%% Low FPS,%.2f\n"), Report.OnePercentLowFPS);
	CSV += FString::Printf(TEXT("0.1%% Low FPS,%.2f\n"), Report.PointOnePercentLowFPS);
	CSV += FString::Printf(TEXT("Median Frame Time (ms),%.2f\n"), Report.MedianFrameTimeMs);
	CSV += FString::Printf(TEXT("95th Percentile Frame Time (ms),%.2f\n"), Report.Percentile95FrameTimeMs);
	CSV += FString::Printf(TEXT("99th Percentile Frame Time (ms),%.2f\n"), Report.Percentile99FrameTimeMs);
	CSV += FString::Printf(TEXT("Total Frames,%d\n"), Report.TotalFrames);
	CSV += FString::Printf(TEXT("Spike Count,%d\n\n"), Report.SpikeCount);
	
	// System performance section
	if (Report.SystemBreakdown.Num() > 0)
	{
		CSV += TEXT("System Performance\n");
		CSV += TEXT("System Name,Average Time (ms),Peak Time (ms),Budget (ms),Utilization (%%)\n");
		
		for (const FSystemPerformanceData& System : Report.SystemBreakdown)
		{
			float Utilization = System.BudgetTimeMs > 0.0 ? 
				(System.AverageTimeMs / System.BudgetTimeMs) * 100.0f : 0.0f;
			
			CSV += FString::Printf(TEXT("%s,%.3f,%.3f,%.2f,%.1f\n"),
				*System.SystemName.ToString(),
				System.AverageTimeMs,
				System.PeakTimeMs,
				System.BudgetTimeMs,
				Utilization);
		}
		
		CSV += FString::Printf(TEXT("\nTotal Budget Violations,%d\n\n"), Report.TotalBudgetViolations);
	}
	
	// Memory statistics section
	CSV += TEXT("Memory Statistics\n");
	CSV += TEXT("Metric,Value (MB)\n");
	CSV += FString::Printf(TEXT("Average Memory,%.2f\n"), Report.AverageMemoryUsage / (1024.0 * 1024.0));
	CSV += FString::Printf(TEXT("Peak Memory,%.2f\n"), Report.PeakMemoryUsage / (1024.0 * 1024.0));
	CSV += FString::Printf(TEXT("Min Memory,%.2f\n"), Report.MinMemoryUsage / (1024.0 * 1024.0));
	CSV += FString::Printf(TEXT("Growth Rate (MB/min),%.2f\n"), Report.MemoryGrowthRate);
	CSV += FString::Printf(TEXT("Memory Leak Detected,%s\n\n"), Report.bMemoryLeakDetected ? TEXT("YES") : TEXT("NO"));
	
	// Gameplay metrics section
	CSV += TEXT("Gameplay Metrics\n");
	CSV += TEXT("Metric,Value\n");
	CSV += FString::Printf(TEXT("Peak Monster Count,%d\n"), Report.PeakMonsterCount);
	CSV += FString::Printf(TEXT("Peak Projectile Count,%d\n"), Report.PeakProjectileCount);
	CSV += FString::Printf(TEXT("Peak Particle Count,%d\n"), Report.PeakParticleCount);
	CSV += FString::Printf(TEXT("Total Events Processed,%d\n"), Report.TotalEventsProcessed);

	// Write to file
	if (!FFileHelper::SaveStringToFile(CSV, *FilePath))
	{
		Context.AddError(FString::Printf(
			TEXT("Failed to write CSV to file: %s"),
			*FilePath));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Exported performance report to CSV: %s (%.2f KB)"),
		*FilePath,
		CSV.Len() / 1024.0f);

	return true;
}

bool UDelveDeepTelemetrySubsystem::ExportReportToJSON(const FPerformanceReport& Report, const FString& FilePath)
{
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("ExportReportToJSON");

	// Validate file path
	if (FilePath.IsEmpty())
	{
		Context.AddError(TEXT("File path cannot be empty"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Ensure directory exists
	FString Directory = FPaths::GetPath(FilePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		if (!PlatformFile.CreateDirectoryTree(*Directory))
		{
			Context.AddError(FString::Printf(
				TEXT("Failed to create directory: %s"),
				*Directory));
			UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
			return false;
		}
	}

	// Create JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	
	// Metadata
	TSharedPtr<FJsonObject> MetadataObj = MakeShareable(new FJsonObject());
	MetadataObj->SetStringField(TEXT("GenerationTime"), Report.GenerationTime.ToString());
	MetadataObj->SetNumberField(TEXT("DurationSeconds"), Report.DurationSeconds);
	MetadataObj->SetStringField(TEXT("BuildVersion"), Report.BuildVersion);
	MetadataObj->SetStringField(TEXT("MapName"), Report.MapName);
	JsonObject->SetObjectField(TEXT("Metadata"), MetadataObj);
	
	// Frame performance
	TSharedPtr<FJsonObject> FrameObj = MakeShareable(new FJsonObject());
	FrameObj->SetNumberField(TEXT("AverageFPS"), Report.AverageFPS);
	FrameObj->SetNumberField(TEXT("MinFPS"), Report.MinFPS);
	FrameObj->SetNumberField(TEXT("MaxFPS"), Report.MaxFPS);
	FrameObj->SetNumberField(TEXT("OnePercentLowFPS"), Report.OnePercentLowFPS);
	FrameObj->SetNumberField(TEXT("PointOnePercentLowFPS"), Report.PointOnePercentLowFPS);
	FrameObj->SetNumberField(TEXT("MedianFrameTimeMs"), Report.MedianFrameTimeMs);
	FrameObj->SetNumberField(TEXT("Percentile95FrameTimeMs"), Report.Percentile95FrameTimeMs);
	FrameObj->SetNumberField(TEXT("Percentile99FrameTimeMs"), Report.Percentile99FrameTimeMs);
	FrameObj->SetNumberField(TEXT("TotalFrames"), Report.TotalFrames);
	FrameObj->SetNumberField(TEXT("SpikeCount"), Report.SpikeCount);
	JsonObject->SetObjectField(TEXT("FramePerformance"), FrameObj);
	
	// System performance
	TArray<TSharedPtr<FJsonValue>> SystemArray;
	for (const FSystemPerformanceData& System : Report.SystemBreakdown)
	{
		TSharedPtr<FJsonObject> SystemObj = MakeShareable(new FJsonObject());
		SystemObj->SetStringField(TEXT("SystemName"), System.SystemName.ToString());
		SystemObj->SetNumberField(TEXT("AverageTimeMs"), System.AverageTimeMs);
		SystemObj->SetNumberField(TEXT("PeakTimeMs"), System.PeakTimeMs);
		SystemObj->SetNumberField(TEXT("BudgetTimeMs"), System.BudgetTimeMs);
		
		float Utilization = System.BudgetTimeMs > 0.0 ? 
			(System.AverageTimeMs / System.BudgetTimeMs) * 100.0f : 0.0f;
		SystemObj->SetNumberField(TEXT("UtilizationPercent"), Utilization);
		
		SystemArray.Add(MakeShareable(new FJsonValueObject(SystemObj)));
	}
	JsonObject->SetArrayField(TEXT("SystemPerformance"), SystemArray);
	JsonObject->SetNumberField(TEXT("TotalBudgetViolations"), Report.TotalBudgetViolations);
	
	// Memory statistics
	TSharedPtr<FJsonObject> MemoryObj = MakeShareable(new FJsonObject());
	MemoryObj->SetNumberField(TEXT("AverageMemoryMB"), Report.AverageMemoryUsage / (1024.0 * 1024.0));
	MemoryObj->SetNumberField(TEXT("PeakMemoryMB"), Report.PeakMemoryUsage / (1024.0 * 1024.0));
	MemoryObj->SetNumberField(TEXT("MinMemoryMB"), Report.MinMemoryUsage / (1024.0 * 1024.0));
	MemoryObj->SetNumberField(TEXT("GrowthRateMBPerMin"), Report.MemoryGrowthRate);
	MemoryObj->SetBoolField(TEXT("MemoryLeakDetected"), Report.bMemoryLeakDetected);
	JsonObject->SetObjectField(TEXT("MemoryStatistics"), MemoryObj);
	
	// Gameplay metrics
	TSharedPtr<FJsonObject> GameplayObj = MakeShareable(new FJsonObject());
	GameplayObj->SetNumberField(TEXT("PeakMonsterCount"), Report.PeakMonsterCount);
	GameplayObj->SetNumberField(TEXT("PeakProjectileCount"), Report.PeakProjectileCount);
	GameplayObj->SetNumberField(TEXT("PeakParticleCount"), Report.PeakParticleCount);
	GameplayObj->SetNumberField(TEXT("TotalEventsProcessed"), Report.TotalEventsProcessed);
	JsonObject->SetObjectField(TEXT("GameplayMetrics"), GameplayObj);

	// Serialize to JSON string
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
	{
		Context.AddError(TEXT("Failed to serialize report to JSON"));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	// Write to file
	if (!FFileHelper::SaveStringToFile(JsonString, *FilePath))
	{
		Context.AddError(FString::Printf(
			TEXT("Failed to write JSON to file: %s"),
			*FilePath));
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
		return false;
	}

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Exported performance report to JSON: %s (%.2f KB)"),
		*FilePath,
		JsonString.Len() / 1024.0f);

	return true;
}
