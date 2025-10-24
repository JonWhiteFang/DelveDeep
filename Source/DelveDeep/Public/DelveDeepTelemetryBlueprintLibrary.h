// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelveDeepPerformanceOverlay.h"
#include "DelveDeepTelemetryBlueprintLibrary.generated.h"

/**
 * Blueprint Function Library for DelveDeep Telemetry System
 * 
 * Provides Blueprint-accessible functions for performance monitoring and telemetry.
 * All functions require a WorldContext object to access the telemetry subsystem.
 */
UCLASS()
class DELVEDEEP_API UDelveDeepTelemetryBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the telemetry subsystem from world context
	 * @param WorldContextObject World context object (usually 'self' in Blueprint)
	 * @return Telemetry subsystem instance, or nullptr if not available
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static class UDelveDeepTelemetrySubsystem* GetTelemetrySubsystem(
		const UObject* WorldContextObject);

	/**
	 * Get current frames per second
	 * @param WorldContextObject World context object
	 * @return Current FPS value
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static float GetCurrentFPS(const UObject* WorldContextObject);

	/**
	 * Get average FPS over recent frames
	 * @param WorldContextObject World context object
	 * @return Average FPS value
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static float GetAverageFPS(const UObject* WorldContextObject);

	/**
	 * Get 1% low FPS (99th percentile worst frame times)
	 * @param WorldContextObject World context object
	 * @return 1% low FPS
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static float GetOnePercentLowFPS(const UObject* WorldContextObject);

	/**
	 * Capture a performance baseline with the given name
	 * @param WorldContextObject World context object
	 * @param BaselineName Name for this baseline
	 * @return True if baseline was captured successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static bool CapturePerformanceBaseline(
		const UObject* WorldContextObject,
		FName BaselineName);

	/**
	 * Enable the performance overlay
	 * @param WorldContextObject World context object
	 * @param Mode Display mode (Minimal, Standard, or Detailed)
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static void EnablePerformanceOverlay(
		const UObject* WorldContextObject,
		EOverlayMode Mode = EOverlayMode::Standard);

	/**
	 * Disable the performance overlay
	 * @param WorldContextObject World context object
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static void DisablePerformanceOverlay(const UObject* WorldContextObject);

	/**
	 * Check if the performance overlay is enabled
	 * @param WorldContextObject World context object
	 * @return True if overlay is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsPerformanceOverlayEnabled(const UObject* WorldContextObject);

	/**
	 * Start a profiling session with detailed metric collection
	 * @param WorldContextObject World context object
	 * @param SessionName Name for this profiling session
	 * @return True if session was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static bool StartProfilingSession(
		const UObject* WorldContextObject,
		FName SessionName);

	/**
	 * Stop the current profiling session
	 * @param WorldContextObject World context object
	 * @return True if session was stopped successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static bool StopProfilingSession(const UObject* WorldContextObject);

	/**
	 * Check if a profiling session is currently active
	 * @param WorldContextObject World context object
	 * @return True if session is active
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static bool IsProfilingActive(const UObject* WorldContextObject);

	/**
	 * Track entity count for a specific type
	 * @param WorldContextObject World context object
	 * @param EntityType Type of entity (e.g., "Monsters", "Projectiles")
	 * @param Count Current count
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static void TrackEntityCount(
		const UObject* WorldContextObject,
		FName EntityType,
		int32 Count);

	/**
	 * Get current entity count
	 * @param WorldContextObject World context object
	 * @param EntityType Type of entity
	 * @return Current count
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static int32 GetEntityCount(
		const UObject* WorldContextObject,
		FName EntityType);

	/**
	 * Record an asset load operation
	 * @param WorldContextObject World context object
	 * @param AssetPath Path to the loaded asset
	 * @param LoadTimeMs Load time in milliseconds
	 * @param AssetSize Asset size in bytes
	 * @param bSynchronous Whether this was a synchronous load
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry",
		meta = (WorldContext = "WorldContextObject"))
	static void RecordAssetLoad(
		const UObject* WorldContextObject,
		const FString& AssetPath,
		float LoadTimeMs,
		int64 AssetSize,
		bool bSynchronous = true);
};
