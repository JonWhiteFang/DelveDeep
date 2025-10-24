// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DelveDeepStats.h"
#include "DelveDeepFramePerformanceTracker.h"
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

private:
	// Frame tracking
	FFramePerformanceTracker FrameTracker;

	// Telemetry state
	bool bTelemetryEnabled = true;
	bool bInitialized = false;
};
