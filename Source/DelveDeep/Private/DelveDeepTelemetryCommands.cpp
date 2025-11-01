// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTelemetryCommands.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepTelemetryCommands, Log, All);

// Console command instances
static FAutoConsoleCommand ShowFPSCmd(
	TEXT("DelveDeep.Telemetry.ShowFPS"),
	TEXT("Display current FPS statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowFPS)
);

static FAutoConsoleCommand ShowFrameStatsCmd(
	TEXT("DelveDeep.Telemetry.ShowFrameStats"),
	TEXT("Display detailed frame statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowFrameStats)
);

static FAutoConsoleCommand ResetFrameStatsCmd(
	TEXT("DelveDeep.Telemetry.ResetFrameStats"),
	TEXT("Reset frame statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ResetFrameStats)
);

static FAutoConsoleCommand ShowSystemStatsCmd(
	TEXT("DelveDeep.Telemetry.ShowSystemStats"),
	TEXT("Display system performance statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowSystemStats)
);

static FAutoConsoleCommand ShowBudgetsCmd(
	TEXT("DelveDeep.Telemetry.ShowBudgets"),
	TEXT("Display system budget utilization"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowBudgets)
);

static FAutoConsoleCommand ResetBudgetsCmd(
	TEXT("DelveDeep.Telemetry.ResetBudgets"),
	TEXT("Reset budget statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ResetBudgets)
);

static FAutoConsoleCommand ShowMemoryCmd(
	TEXT("DelveDeep.Telemetry.ShowMemory"),
	TEXT("Display memory statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowMemory)
);

static FAutoConsoleCommand ShowMemoryHistoryCmd(
	TEXT("DelveDeep.Telemetry.ShowMemoryHistory"),
	TEXT("Display memory history"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowMemoryHistory)
);

static FAutoConsoleCommand CheckMemoryLeaksCmd(
	TEXT("DelveDeep.Telemetry.CheckMemoryLeaks"),
	TEXT("Check for memory leaks"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::CheckMemoryLeaks)
);

static FAutoConsoleCommand CaptureBaselineCmd(
	TEXT("DelveDeep.Telemetry.CaptureBaseline"),
	TEXT("Capture a performance baseline. Usage: DelveDeep.Telemetry.CaptureBaseline <BaselineName>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::CaptureBaseline)
);

static FAutoConsoleCommand CompareBaselineCmd(
	TEXT("DelveDeep.Telemetry.CompareBaseline"),
	TEXT("Compare current performance to a baseline. Usage: DelveDeep.Telemetry.CompareBaseline <BaselineName>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::CompareBaseline)
);

static FAutoConsoleCommand ListBaselinesCmd(
	TEXT("DelveDeep.Telemetry.ListBaselines"),
	TEXT("List all available baselines"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ListBaselines)
);

static FAutoConsoleCommand SaveBaselineCmd(
	TEXT("DelveDeep.Telemetry.SaveBaseline"),
	TEXT("Save a baseline to disk. Usage: DelveDeep.Telemetry.SaveBaseline <BaselineName> [FilePath]"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::SaveBaseline)
);

static FAutoConsoleCommand LoadBaselineCmd(
	TEXT("DelveDeep.Telemetry.LoadBaseline"),
	TEXT("Load a baseline from disk. Usage: DelveDeep.Telemetry.LoadBaseline <BaselineName> <FilePath>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::LoadBaseline)
);

static FAutoConsoleCommand DeleteBaselineCmd(
	TEXT("DelveDeep.Telemetry.DeleteBaseline"),
	TEXT("Delete a baseline. Usage: DelveDeep.Telemetry.DeleteBaseline <BaselineName>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::DeleteBaseline)
);

static FAutoConsoleCommand GenerateReportCmd(
	TEXT("DelveDeep.Telemetry.GenerateReport"),
	TEXT("Generate a performance report. Usage: DelveDeep.Telemetry.GenerateReport [DurationSeconds]"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::GenerateReport)
);

static FAutoConsoleCommand ExportCSVCmd(
	TEXT("DelveDeep.Telemetry.ExportCSV"),
	TEXT("Export report to CSV. Usage: DelveDeep.Telemetry.ExportCSV <FilePath>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ExportCSV)
);

static FAutoConsoleCommand ExportJSONCmd(
	TEXT("DelveDeep.Telemetry.ExportJSON"),
	TEXT("Export report to JSON. Usage: DelveDeep.Telemetry.ExportJSON <FilePath>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ExportJSON)
);

static FAutoConsoleCommand StartProfilingCmd(
	TEXT("DelveDeep.Telemetry.StartProfiling"),
	TEXT("Start a profiling session. Usage: DelveDeep.Telemetry.StartProfiling <SessionName>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::StartProfiling)
);

static FAutoConsoleCommand StopProfilingCmd(
	TEXT("DelveDeep.Telemetry.StopProfiling"),
	TEXT("Stop the current profiling session"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::StopProfiling)
);

static FAutoConsoleCommand ShowProfilingStatusCmd(
	TEXT("DelveDeep.Telemetry.ShowProfilingStatus"),
	TEXT("Show profiling session status"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowProfilingStatus)
);

static FAutoConsoleCommand EnableOverlayCmd(
	TEXT("DelveDeep.Telemetry.EnableOverlay"),
	TEXT("Enable performance overlay. Usage: DelveDeep.Telemetry.EnableOverlay [Minimal|Standard|Detailed]"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::EnableOverlay)
);

static FAutoConsoleCommand DisableOverlayCmd(
	TEXT("DelveDeep.Telemetry.DisableOverlay"),
	TEXT("Disable performance overlay"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::DisableOverlay)
);

static FAutoConsoleCommand SetOverlayModeCmd(
	TEXT("DelveDeep.Telemetry.SetOverlayMode"),
	TEXT("Set overlay mode. Usage: DelveDeep.Telemetry.SetOverlayMode <Minimal|Standard|Detailed>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::SetOverlayMode)
);

static FAutoConsoleCommand ShowGameplayMetricsCmd(
	TEXT("DelveDeep.Telemetry.ShowGameplayMetrics"),
	TEXT("Display gameplay metrics (entity counts)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowGameplayMetrics)
);

static FAutoConsoleCommand ShowAssetLoadsCmd(
	TEXT("DelveDeep.Telemetry.ShowAssetLoads"),
	TEXT("Display asset loading statistics"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepTelemetryCommands::ShowAssetLoads)
);

// Implementation

void FDelveDeepTelemetryCommands::RegisterCommands()
{
	// Commands are registered automatically via FAutoConsoleCommand
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Telemetry console commands registered"));
}

void FDelveDeepTelemetryCommands::UnregisterCommands()
{
	// Commands are unregistered automatically
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Telemetry console commands unregistered"));
}

UDelveDeepTelemetrySubsystem* FDelveDeepTelemetryCommands::GetTelemetrySubsystem()
{
	if (GEngine && GEngine->GetWorldContexts().Num() > 0)
	{
		UWorld* World = GEngine->GetWorldContexts()[0].World();
		if (World && World->GetGameInstance())
		{
			return World->GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>();
		}
	}
	return nullptr;
}

FString FDelveDeepTelemetryCommands::FormatBytes(uint64 Bytes)
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%llu B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f KB"), Bytes / 1024.0);
	}
	else if (Bytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f MB"), Bytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.2f GB"), Bytes / (1024.0 * 1024.0 * 1024.0));
	}
}

FString FDelveDeepTelemetryCommands::FormatTime(float Milliseconds)
{
	if (Milliseconds < 1.0f)
	{
		return FString::Printf(TEXT("%.3f ms"), Milliseconds);
	}
	else if (Milliseconds < 1000.0f)
	{
		return FString::Printf(TEXT("%.2f ms"), Milliseconds);
	}
	else
	{
		return FString::Printf(TEXT("%.2f s"), Milliseconds / 1000.0f);
	}
}

// Frame Tracking Commands

void FDelveDeepTelemetryCommands::ShowFPS(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const float CurrentFPS = Telemetry->GetCurrentFPS();
	const float AverageFPS = Telemetry->GetAverageFPS();
	const float OnePercentLow = Telemetry->GetOnePercentLowFPS();

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== FPS Statistics ==="));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Current FPS: %.1f"), CurrentFPS);
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Average FPS: %.1f"), AverageFPS);
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("1%% Low FPS: %.1f"), OnePercentLow);
}

void FDelveDeepTelemetryCommands::ShowFrameStats(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FFramePerformanceData FrameData = Telemetry->GetCurrentFrameData();
	const TArray<float> FrameHistory = Telemetry->GetFrameTimeHistory(60);

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Frame Statistics ==="));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Frame Time: %s"), *FormatTime(FrameData.FrameTimeMs));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Game Thread: %s"), *FormatTime(FrameData.GameThreadTimeMs));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Render Thread: %s"), *FormatTime(FrameData.RenderThreadTimeMs));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Recent frames: %d"), FrameHistory.Num());
}

void FDelveDeepTelemetryCommands::ResetFrameStats(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Frame statistics reset"));
}

// System Profiling Commands

void FDelveDeepTelemetryCommands::ShowSystemStats(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const TArray<FSystemPerformanceData> SystemData = Telemetry->GetAllSystemPerformance();

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== System Performance Statistics ==="));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-20s %10s %10s %10s"), TEXT("System"), TEXT("Time"), TEXT("Budget"), TEXT("Calls"));

	for (const FSystemPerformanceData& System : SystemData)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-20s %10s %10s %10d"),
			*System.SystemName.ToString(),
			*FormatTime(System.CycleTime),
			*FormatTime(System.BudgetTime),
			System.CallCount);
	}
}

void FDelveDeepTelemetryCommands::ShowBudgets(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const TArray<FSystemPerformanceData> SystemData = Telemetry->GetAllSystemPerformance();

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== System Budget Utilization ==="));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-20s %10s %10s %12s"), TEXT("System"), TEXT("Actual"), TEXT("Budget"), TEXT("Utilization"));

	for (const FSystemPerformanceData& System : SystemData)
	{
		const float Utilization = Telemetry->GetSystemBudgetUtilization(System.SystemName);
		const bool bViolated = Telemetry->IsSystemBudgetViolated(System.SystemName);

		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-20s %10s %10s %11.1f%% %s"),
			*System.SystemName.ToString(),
			*FormatTime(System.CycleTime),
			*FormatTime(System.BudgetTime),
			Utilization * 100.0f,
			bViolated ? TEXT("[OVER]") : TEXT(""));
	}
}

void FDelveDeepTelemetryCommands::ResetBudgets(const TArray<FString>& Args)
{
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Budget statistics reset"));
}

// Memory Tracking Commands

void FDelveDeepTelemetryCommands::ShowMemory(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FMemorySnapshot Snapshot = Telemetry->GetCurrentMemorySnapshot();
	const int64 PeakMemory = Telemetry->GetPeakMemoryUsage();
	const float GrowthRate = Telemetry->GetMemoryGrowthRate();

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Memory Statistics ==="));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Total Memory: %s"), *FormatBytes(Snapshot.TotalMemory));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Peak Memory: %s"), *FormatBytes(PeakMemory));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Growth Rate: %.2f MB/min"), GrowthRate);

	if (Snapshot.PerSystemMemory.Num() > 0)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("\nPer-System Memory:"));
		for (const auto& Pair : Snapshot.PerSystemMemory)
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("  %-20s: %s"),
				*Pair.Key.ToString(), *FormatBytes(Pair.Value));
		}
	}
}

void FDelveDeepTelemetryCommands::ShowMemoryHistory(const TArray<FString>& Args)
{
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Memory history display not yet implemented"));
}

void FDelveDeepTelemetryCommands::CheckMemoryLeaks(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const bool bLeakDetected = Telemetry->IsMemoryLeakDetected();
	const float GrowthRate = Telemetry->GetMemoryGrowthRate();

	if (bLeakDetected)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Warning,
			TEXT("Potential memory leak detected! Growth rate: %.2f MB/min"), GrowthRate);
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display,
			TEXT("No memory leaks detected. Growth rate: %.2f MB/min"), GrowthRate);
	}
}

// Baseline Management Commands

void FDelveDeepTelemetryCommands::CaptureBaseline(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.CaptureBaseline <BaselineName>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FName BaselineName = FName(*Args[0]);
	if (Telemetry->CaptureBaseline(BaselineName))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Captured baseline '%s'"), *BaselineName.ToString());
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to capture baseline '%s'"), *BaselineName.ToString());
	}
}

void FDelveDeepTelemetryCommands::CompareBaseline(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.CompareBaseline <BaselineName>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FName BaselineName = FName(*Args[0]);
	FPerformanceComparison Comparison;

	if (Telemetry->CompareToBaseline(BaselineName, Comparison))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Baseline Comparison: %s ==="), *BaselineName.ToString());
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("FPS Change: %.1f%%"), Comparison.FPSChangePercent);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Frame Time Change: %.1f%%"), Comparison.FrameTimeChangePercent);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Memory Change: %.1f%%"), Comparison.MemoryChangePercent);
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to compare to baseline '%s'"), *BaselineName.ToString());
	}
}

void FDelveDeepTelemetryCommands::ListBaselines(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const TArray<FName> Baselines = Telemetry->GetAvailableBaselines();

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Available Baselines (%d) ==="), Baselines.Num());
	for (const FName& BaselineName : Baselines)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("  - %s"), *BaselineName.ToString());
	}
}

void FDelveDeepTelemetryCommands::SaveBaseline(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.SaveBaseline <BaselineName> [FilePath]"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FName BaselineName = FName(*Args[0]);
	const FString FilePath = Args.Num() > 1 ? Args[1] : TEXT("");

	if (Telemetry->SaveBaseline(BaselineName, FilePath))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Saved baseline '%s'"), *BaselineName.ToString());
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to save baseline '%s'"), *BaselineName.ToString());
	}
}

void FDelveDeepTelemetryCommands::LoadBaseline(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.LoadBaseline <BaselineName> <FilePath>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FName BaselineName = FName(*Args[0]);
	const FString FilePath = Args[1];

	if (Telemetry->LoadBaseline(BaselineName, FilePath))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Loaded baseline '%s' from '%s'"), *BaselineName.ToString(), *FilePath);
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to load baseline from '%s'"), *FilePath);
	}
}

void FDelveDeepTelemetryCommands::DeleteBaseline(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.DeleteBaseline <BaselineName>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FName BaselineName = FName(*Args[0]);

	if (Telemetry->DeleteBaseline(BaselineName))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Deleted baseline '%s'"), *BaselineName.ToString());
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to delete baseline '%s'"), *BaselineName.ToString());
	}
}

// Reporting Commands

void FDelveDeepTelemetryCommands::GenerateReport(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const float Duration = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 300.0f;

	FPerformanceReport Report;
	if (Telemetry->GeneratePerformanceReport(Report, Duration))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Performance Report (%.0fs) ==="), Duration);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Average FPS: %.1f"), Report.AverageFPS);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Min FPS: %.1f"), Report.MinFPS);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Max FPS: %.1f"), Report.MaxFPS);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("1%% Low FPS: %.1f"), Report.OnePercentLowFPS);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Total Frames: %d"), Report.TotalFrames);
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Spike Count: %d"), Report.SpikeCount);
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to generate performance report"));
	}
}

void FDelveDeepTelemetryCommands::ExportCSV(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.ExportCSV <FilePath>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	FPerformanceReport Report;
	if (Telemetry->GeneratePerformanceReport(Report, 300.0f))
	{
		if (Telemetry->ExportReportToCSV(Report, Args[0]))
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Exported report to CSV: %s"), *Args[0]);
		}
		else
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to export report to CSV"));
		}
	}
}

void FDelveDeepTelemetryCommands::ExportJSON(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.ExportJSON <FilePath>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	FPerformanceReport Report;
	if (Telemetry->GeneratePerformanceReport(Report, 300.0f))
	{
		if (Telemetry->ExportReportToJSON(Report, Args[0]))
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Exported report to JSON: %s"), *Args[0]);
		}
		else
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to export report to JSON"));
		}
	}
}

// Profiling Session Commands

void FDelveDeepTelemetryCommands::StartProfiling(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.StartProfiling <SessionName>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const FName SessionName = FName(*Args[0]);

	if (Telemetry->StartProfilingSession(SessionName))
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Started profiling session '%s'"), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Failed to start profiling session '%s'"), *SessionName.ToString());
	}
}

void FDelveDeepTelemetryCommands::StopProfiling(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	if (Telemetry->StopProfilingSession())
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Stopped profiling session"));
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("No profiling session is active"));
	}
}

void FDelveDeepTelemetryCommands::ShowProfilingStatus(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	if (Telemetry->IsProfilingActive())
	{
		FProfilingSession Session;
		if (Telemetry->GetCurrentSession(Session))
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Profiling Session Active ==="));
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Session Name: %s"), *Session.SessionName.ToString());
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Duration: %.1fs"), Session.DurationSeconds);
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Frames Captured: %d"), Session.TotalFrames);
		}
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("No profiling session is currently active"));
	}
}

// Visualization Commands

void FDelveDeepTelemetryCommands::EnableOverlay(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	EOverlayMode Mode = EOverlayMode::Standard;

	if (Args.Num() > 0)
	{
		const FString ModeStr = Args[0].ToLower();
		if (ModeStr == TEXT("minimal"))
		{
			Mode = EOverlayMode::Minimal;
		}
		else if (ModeStr == TEXT("standard"))
		{
			Mode = EOverlayMode::Standard;
		}
		else if (ModeStr == TEXT("detailed"))
		{
			Mode = EOverlayMode::Detailed;
		}
		else
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Warning, TEXT("Unknown mode '%s', using Standard"), *Args[0]);
		}
	}

	Telemetry->EnablePerformanceOverlay(Mode);
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Performance overlay enabled (Mode: %d)"), static_cast<int32>(Mode));
}

void FDelveDeepTelemetryCommands::DisableOverlay(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	Telemetry->DisablePerformanceOverlay();
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Performance overlay disabled"));
}

void FDelveDeepTelemetryCommands::SetOverlayMode(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Usage: DelveDeep.Telemetry.SetOverlayMode <Minimal|Standard|Detailed>"));
		return;
	}

	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	EOverlayMode Mode = EOverlayMode::Standard;
	const FString ModeStr = Args[0].ToLower();

	if (ModeStr == TEXT("minimal"))
	{
		Mode = EOverlayMode::Minimal;
	}
	else if (ModeStr == TEXT("standard"))
	{
		Mode = EOverlayMode::Standard;
	}
	else if (ModeStr == TEXT("detailed"))
	{
		Mode = EOverlayMode::Detailed;
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Unknown mode '%s'. Use: Minimal, Standard, or Detailed"), *Args[0]);
		return;
	}

	Telemetry->SetOverlayMode(Mode);
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Overlay mode set to: %d"), static_cast<int32>(Mode));
}

// Gameplay Metrics Commands

void FDelveDeepTelemetryCommands::ShowGameplayMetrics(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Gameplay Metrics ==="));

	// Common entity types
	TArray<FName> EntityTypes = {
		TEXT("Monsters"),
		TEXT("Projectiles"),
		TEXT("Particles"),
		TEXT("Effects"),
		TEXT("Items"),
		TEXT("Pickups")
	};

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-15s %10s %10s %10s %10s"),
		TEXT("Entity Type"), TEXT("Current"), TEXT("Peak"), TEXT("Average"), TEXT("Limit"));

	for (const FName& EntityType : EntityTypes)
	{
		const int32 Current = Telemetry->GetEntityCount(EntityType);
		const int32 Peak = Telemetry->GetPeakEntityCount(EntityType);
		const float Average = Telemetry->GetAverageEntityCount(EntityType);
		const int32 Limit = Telemetry->GetRecommendedEntityLimit(EntityType);
		const bool bExceeding = Telemetry->IsEntityCountExceedingLimit(EntityType);

		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-15s %10d %10d %10.1f %10d %s"),
			*EntityType.ToString(),
			Current,
			Peak,
			Average,
			Limit,
			bExceeding ? TEXT("[OVER]") : TEXT(""));
	}
}

void FDelveDeepTelemetryCommands::ShowAssetLoads(const TArray<FString>& Args)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem();
	if (!Telemetry)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Error, TEXT("Telemetry subsystem not available"));
		return;
	}

	const TArray<FAssetLoadStatistics> AllStats = Telemetry->GetAllAssetLoadStatistics();
	const int32 TotalLoads = Telemetry->GetTotalAssetLoads();
	const int32 SlowLoads = Telemetry->GetTotalSlowLoads();

	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("=== Asset Loading Statistics ==="));
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Total Loads: %d"), TotalLoads);
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("Slow Loads (>100ms): %d"), SlowLoads);
	UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT(""));

	if (AllStats.Num() > 0)
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-15s %10s %10s %10s %10s %10s"),
			TEXT("Asset Type"), TEXT("Total"), TEXT("Sync"), TEXT("Async"), TEXT("Avg Time"), TEXT("Slow"));

		for (const FAssetLoadStatistics& Stats : AllStats)
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("%-15s %10d %10d %10d %10s %10d"),
				*Stats.AssetType.ToString(),
				Stats.TotalLoads,
				Stats.SynchronousLoads,
				Stats.AsynchronousLoads,
				*FormatTime(Stats.AverageLoadTimeMs),
				Stats.SlowLoadCount);
		}

		// Show slowest loads
		const TArray<FAssetLoadRecord> SlowestLoads = Telemetry->GetSlowestAssetLoads(5);
		if (SlowestLoads.Num() > 0)
		{
			UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("\nSlowest Asset Loads:"));
			for (const FAssetLoadRecord& Record : SlowestLoads)
			{
				UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("  %s: %s (%s)"),
					*FormatTime(Record.LoadTimeMs),
					*Record.AssetPath,
					*Record.AssetType.ToString());
			}
		}
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetryCommands, Display, TEXT("No asset loads recorded"));
	}
}
