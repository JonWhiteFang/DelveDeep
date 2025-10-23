// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "DelveDeepEventTypes.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepEvents, Log, All);

/**
 * Location information for a registered listener.
 * Used for O(1) handle-to-listener lookup.
 */
struct FListenerLocation
{
	FGameplayTag EventTag;
	EDelveDeepEventPriority Priority;
	int32 IndexInArray;
};

/**
 * Centralized event system subsystem for DelveDeep.
 * Provides a gameplay event bus using GameplayTags for loose coupling between systems.
 * Enables event-driven communication without direct dependencies.
 */
UCLASS(BlueprintType)
class DELVEDEEP_API UDelveDeepEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Registers a listener for a specific event tag.
	 * @param EventTag The GameplayTag identifying the event type to listen for
	 * @param Callback The function to invoke when the event is broadcast
	 * @param Owner The object registering the listener (for automatic cleanup)
	 * @param Priority The priority level for this listener (High, Normal, or Low)
	 * @param Filter Optional filter criteria for this listener
	 * @return Handle that can be used to unregister the listener
	 */
	FDelegateHandle RegisterListener(
		FGameplayTag EventTag,
		TFunction<void(const FDelveDeepEventPayload&)> Callback,
		UObject* Owner,
		EDelveDeepEventPriority Priority = EDelveDeepEventPriority::Normal,
		const FDelveDeepEventFilter& Filter = FDelveDeepEventFilter()
	);

	/**
	 * Unregisters a listener using its handle.
	 * @param Handle The handle returned from RegisterListener
	 */
	void UnregisterListener(FDelegateHandle Handle);

	/**
	 * Unregisters all listeners owned by a specific object.
	 * @param Owner The object whose listeners should be unregistered
	 */
	void UnregisterAllListeners(UObject* Owner);

	/**
	 * Broadcasts an event to all registered listeners.
	 * Listeners are invoked in priority order (High -> Normal -> Low).
	 * @param Payload The event payload containing event data
	 */
	void BroadcastEvent(const FDelveDeepEventPayload& Payload);

	/**
	 * Broadcasts an event in deferred mode (queues for later processing).
	 * Events are queued and processed when ProcessDeferredEvents() is called.
	 * @param Payload The event payload containing event data
	 */
	void BroadcastEventDeferred(const FDelveDeepEventPayload& Payload);

	/**
	 * Enables deferred event processing mode.
	 * All subsequent BroadcastEvent calls will queue events instead of processing immediately.
	 */
	void EnableDeferredMode();

	/**
	 * Disables deferred event processing mode.
	 * Events will be processed immediately again.
	 */
	void DisableDeferredMode();

	/**
	 * Processes all queued deferred events in order.
	 * Events are processed in the order they were broadcast.
	 */
	void ProcessDeferredEvents();

	/**
	 * Gets the number of listeners registered for a specific event tag.
	 * @param EventTag The event tag to query
	 * @return Number of listeners registered for this tag
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	int32 GetListenerCount(FGameplayTag EventTag) const;

	/**
	 * Gets the current performance metrics for the event system.
	 * @return Reference to the performance metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	const FEventSystemMetrics& GetPerformanceMetrics() const;

	/**
	 * Resets all performance metrics to zero.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	void ResetPerformanceMetrics();

	/**
	 * Gets the event history (last N events).
	 * @return Array of event records
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	TArray<FEventRecord> GetEventHistory() const;

	/**
	 * Enables event logging for debugging.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	void EnableEventLogging();

	/**
	 * Disables event logging.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	void DisableEventLogging();

	/**
	 * Gets all event tags that are marked as network-relevant.
	 * This is for future multiplayer support.
	 * @return Array of GameplayTags for network-relevant events
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events")
	TArray<FGameplayTag> GetNetworkRelevantEvents() const;

private:
	/** Event registry: Maps GameplayTag to listener lists */
	UPROPERTY()
	TMap<FGameplayTag, FEventListenerList> EventRegistry;

	/** Maps delegate handles to listener locations for O(1) lookup */
	TMap<FDelegateHandle, FListenerLocation> HandleToListenerMap;

	/** Counter for generating unique delegate handles */
	int32 NextHandleId = 1;

	/** Queue for deferred events */
	TArray<TSharedPtr<FDelveDeepEventPayload>> DeferredEventQueue;

	/** Whether deferred mode is currently enabled */
	bool bDeferredMode = false;

	/** Maximum number of events that can be queued */
	static constexpr int32 MaxDeferredEvents = 1000;

	/** Performance metrics for the event system */
	UPROPERTY()
	FEventSystemMetrics PerformanceMetrics;

	/** Mutex for thread-safe metrics access */
	mutable FCriticalSection MetricsMutex;

	/** Threshold for slow listener warning (in milliseconds) */
	static constexpr double SlowListenerThresholdMs = 5.0;

	/** Event history (circular buffer for last N events) */
	TArray<FEventRecord> EventHistory;

	/** Maximum number of events to keep in history */
	static constexpr int32 MaxHistorySize = 100;

	/** Current index in the circular buffer */
	int32 HistoryIndex = 0;

	/** Whether event logging is enabled */
	bool bEventLoggingEnabled = false;

	/** Set of event tags that are marked as network-relevant */
	TSet<FGameplayTag> NetworkRelevantEventTags;

	/**
	 * Generates a unique delegate handle.
	 * @return A new unique FDelegateHandle
	 */
	FDelegateHandle GenerateUniqueHandle();

	/**
	 * Removes stale listeners with invalid owners from the registry.
	 * Called during low-activity frames for lazy cleanup.
	 */
	void CleanupStaleListeners();

	/**
	 * Removes a listener from a specific priority array.
	 * @param ListenerArray The array to remove from
	 * @param Index The index of the listener to remove
	 * @param EventTag The event tag (for logging)
	 * @param Priority The priority level (for logging)
	 */
	void RemoveListenerFromArray(
		TArray<FDelveDeepEventListener>& ListenerArray,
		int32 Index,
		const FGameplayTag& EventTag,
		EDelveDeepEventPriority Priority
	);

	/**
	 * Internal method to broadcast an event immediately.
	 * @param Payload The event payload to broadcast
	 */
	void BroadcastEventImmediate(const FDelveDeepEventPayload& Payload);

	/**
	 * Records metrics for a listener invocation.
	 * @param Duration Time taken for the listener callback (in seconds)
	 * @param bFailed Whether the listener callback failed
	 */
	void RecordListenerInvocation(double Duration, bool bFailed = false);

	/**
	 * Adds an event to the history buffer.
	 * @param Record The event record to add
	 */
	void AddToHistory(const FEventRecord& Record);

	/**
	 * Generates a summary string for an event payload.
	 * @param Payload The event payload
	 * @return Summary string
	 */
	FString GeneratePayloadSummary(const FDelveDeepEventPayload& Payload) const;
};
