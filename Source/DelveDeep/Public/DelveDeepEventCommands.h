// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Console commands for the DelveDeep Event System.
 * Provides debugging and testing functionality for the event subsystem.
 */
class DELVEDEEP_API FDelveDeepEventCommands
{
public:
	/** Registers all event system console commands */
	static void RegisterCommands();

	/** Unregisters all event system console commands */
	static void UnregisterCommands();

private:
	/** Lists all listeners for a specific event tag */
	static void ListListeners(const TArray<FString>& Args);

	/** Lists all registered listeners across all event tags */
	static void ListAllListeners(const TArray<FString>& Args);

	/** Shows performance metrics for the event system */
	static void ShowMetrics(const TArray<FString>& Args);

	/** Resets performance metrics */
	static void ResetMetrics(const TArray<FString>& Args);

	/** Shows event history */
	static void ShowEventHistory(const TArray<FString>& Args);

	/** Enables event logging */
	static void EnableEventLogging(const TArray<FString>& Args);

	/** Disables event logging */
	static void DisableEventLogging(const TArray<FString>& Args);

	/** Broadcasts a test event */
	static void BroadcastTestEvent(const TArray<FString>& Args);

	/** Validates all event payloads */
	static void ValidateAllPayloads(const TArray<FString>& Args);

	/** Enables payload validation (development builds only) */
	static void EnableValidation(const TArray<FString>& Args);

	/** Disables payload validation (development builds only) */
	static void DisableValidation(const TArray<FString>& Args);

	/** Clears all registered listeners */
	static void ClearAllListeners(const TArray<FString>& Args);

	/** Dumps the event registry structure */
	static void DumpEventRegistry(const TArray<FString>& Args);

	/** Helper to get the event subsystem from the world */
	static class UDelveDeepEventSubsystem* GetEventSubsystem();
};
