// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Console commands for the DelveDeep Telemetry System
 * 
 * Provides comprehensive console command interface for:
 * - Frame performance monitoring
 * - System profiling and budget tracking
 * - Memory tracking and leak detection
 * - Performance baseline management
 * - Report generation and export
 * - Profiling session control
 * - Performance overlay control
 * - Asset loading statistics
 * 
 * All commands are prefixed with "DelveDeep.Telemetry."
 */
class DELVEDEEP_API FDelveDeepTelemetryCommands
{
public:
	/**
	 * Register all telemetry console commands
	 */
	static void RegisterCommands();

	/**
	 * Unregister all telemetry console commands
	 */
	static void UnregisterCommands();

public:
	// Frame tracking commands
	static void ShowFPS(const TArray<FString>& Args);
	static void ShowFrameStats(const TArray<FString>& Args);
	static void ResetFrameStats(const TArray<FString>& Args);

	// System profiling commands
	static void ShowSystemStats(const TArray<FString>& Args);
	static void ShowBudgets(const TArray<FString>& Args);
	static void ResetBudgets(const TArray<FString>& Args);

	// Memory tracking commands
	static void ShowMemory(const TArray<FString>& Args);
	static void ShowMemoryHistory(const TArray<FString>& Args);
	static void CheckMemoryLeaks(const TArray<FString>& Args);

	// Baseline management commands
	static void CaptureBaseline(const TArray<FString>& Args);
	static void CompareBaseline(const TArray<FString>& Args);
	static void ListBaselines(const TArray<FString>& Args);
	static void SaveBaseline(const TArray<FString>& Args);
	static void LoadBaseline(const TArray<FString>& Args);
	static void DeleteBaseline(const TArray<FString>& Args);

	// Reporting commands
	static void GenerateReport(const TArray<FString>& Args);
	static void ExportCSV(const TArray<FString>& Args);
	static void ExportJSON(const TArray<FString>& Args);

	// Profiling session commands
	static void StartProfiling(const TArray<FString>& Args);
	static void StopProfiling(const TArray<FString>& Args);
	static void ShowProfilingStatus(const TArray<FString>& Args);

	// Visualization commands
	static void EnableOverlay(const TArray<FString>& Args);
	static void DisableOverlay(const TArray<FString>& Args);
	static void SetOverlayMode(const TArray<FString>& Args);

	// Gameplay metrics commands
	static void ShowGameplayMetrics(const TArray<FString>& Args);
	static void ShowAssetLoads(const TArray<FString>& Args);

private:
	// Helper functions
	static class UDelveDeepTelemetrySubsystem* GetTelemetrySubsystem();
	static FString FormatBytes(uint64 Bytes);
	static FString FormatTime(float Milliseconds);
};
