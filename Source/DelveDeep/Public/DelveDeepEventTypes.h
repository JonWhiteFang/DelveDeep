// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventTypes.generated.h"

/**
 * Priority levels for event listeners.
 * Determines the order in which listeners are invoked when an event is broadcast.
 * High priority listeners execute first, followed by Normal, then Low.
 */
UENUM(BlueprintType)
enum class EDelveDeepEventPriority : uint8
{
	High    UMETA(DisplayName = "High Priority"),
	Normal  UMETA(DisplayName = "Normal Priority"),
	Low     UMETA(DisplayName = "Low Priority")
};

/**
 * Filter criteria for event listeners.
 * Allows listeners to receive only events that match specific criteria.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepEventFilter
{
	GENERATED_BODY()

	/** If set, only receive events involving this specific actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TWeakObjectPtr<AActor> SpecificActor;

	/** Whether to use actor-specific filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bUseActorFilter = false;

	/** Center location for spatial filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FVector SpatialLocation = FVector::ZeroVector;

	/** Radius for spatial filtering (events within this radius will be received) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	float SpatialRadius = 0.0f;

	/** Whether to use spatial filtering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bUseSpatialFilter = false;

	/**
	 * Checks if an event payload passes this filter.
	 * @param Payload The event payload to check
	 * @return True if the payload passes the filter, false otherwise
	 */
	bool PassesFilter(const FDelveDeepEventPayload& Payload) const;

	/**
	 * Checks if an event payload passes this filter (with specific location).
	 * @param Payload The event payload to check
	 * @param EventLocation The location of the event (for spatial filtering)
	 * @return True if the payload passes the filter, false otherwise
	 */
	bool PassesFilter(const FDelveDeepEventPayload& Payload, const FVector& EventLocation) const;
};

/**
 * Represents a single event listener registration.
 * Contains the callback function, owner reference, priority, and filter.
 */
USTRUCT()
struct DELVEDEEP_API FDelveDeepEventListener
{
	GENERATED_BODY()

	/** Callback function to invoke when event is broadcast */
	TFunction<void(const FDelveDeepEventPayload&)> Callback;

	/** Weak pointer to the object that registered this listener (for automatic cleanup) */
	UPROPERTY()
	TWeakObjectPtr<UObject> Owner;

	/** Priority level for this listener */
	UPROPERTY()
	EDelveDeepEventPriority Priority = EDelveDeepEventPriority::Normal;

	/** Filter criteria for this listener */
	UPROPERTY()
	FDelveDeepEventFilter Filter;

	/** Timestamp when this listener was registered */
	FDateTime RegistrationTime;

	/** Number of times this listener has been invoked */
	int32 InvocationCount = 0;

	FDelveDeepEventListener()
		: RegistrationTime(FDateTime::Now())
	{}

	/** Check if the owner is still valid */
	bool IsOwnerValid() const
	{
		return Owner.IsValid();
	}
};

/**
 * List of event listeners organized by priority.
 * Maintains separate arrays for High, Normal, and Low priority listeners.
 */
USTRUCT()
struct DELVEDEEP_API FEventListenerList
{
	GENERATED_BODY()

	/** High priority listeners (executed first) */
	UPROPERTY()
	TArray<FDelveDeepEventListener> HighPriorityListeners;

	/** Normal priority listeners (executed second) */
	UPROPERTY()
	TArray<FDelveDeepEventListener> NormalPriorityListeners;

	/** Low priority listeners (executed last) */
	UPROPERTY()
	TArray<FDelveDeepEventListener> LowPriorityListeners;

	/**
	 * Gets the total number of listeners across all priorities.
	 * @return Total listener count
	 */
	int32 GetTotalListenerCount() const
	{
		return HighPriorityListeners.Num() + NormalPriorityListeners.Num() + LowPriorityListeners.Num();
	}

	/**
	 * Gets the appropriate listener array for a given priority.
	 * @param Priority The priority level
	 * @return Reference to the listener array for that priority
	 */
	TArray<FDelveDeepEventListener>& GetListenerArrayForPriority(EDelveDeepEventPriority Priority)
	{
		switch (Priority)
		{
		case EDelveDeepEventPriority::High:
			return HighPriorityListeners;
		case EDelveDeepEventPriority::Low:
			return LowPriorityListeners;
		case EDelveDeepEventPriority::Normal:
		default:
			return NormalPriorityListeners;
		}
	}

	/**
	 * Gets the appropriate listener array for a given priority (const version).
	 * @param Priority The priority level
	 * @return Const reference to the listener array for that priority
	 */
	const TArray<FDelveDeepEventListener>& GetListenerArrayForPriority(EDelveDeepEventPriority Priority) const
	{
		switch (Priority)
		{
		case EDelveDeepEventPriority::High:
			return HighPriorityListeners;
		case EDelveDeepEventPriority::Low:
			return LowPriorityListeners;
		case EDelveDeepEventPriority::Normal:
		default:
			return NormalPriorityListeners;
		}
	}
};

/**
 * Performance metrics for the event system.
 * Tracks broadcast counts, timing, and listener invocations.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FEventSystemMetrics
{
	GENERATED_BODY()

	/** Total number of events broadcast */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalEventsBroadcast = 0;

	/** Total number of listener invocations */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalListenerInvocations = 0;

	/** Average time per event broadcast (in milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	double AverageTimePerBroadcast = 0.0;

	/** Average time per listener callback (in milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	double AverageTimePerListener = 0.0;

	/** Total event system overhead (in milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	double TotalEventSystemOverhead = 0.0;

	/** Number of deferred events processed */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 DeferredEventsProcessed = 0;

	/** Number of listener callbacks that failed */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 FailedListenerInvocations = 0;

	/** Peak number of listeners for a single event */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 PeakListenersPerEvent = 0;

	/** Resets all metrics to zero */
	void Reset()
	{
		TotalEventsBroadcast = 0;
		TotalListenerInvocations = 0;
		AverageTimePerBroadcast = 0.0;
		AverageTimePerListener = 0.0;
		TotalEventSystemOverhead = 0.0;
		DeferredEventsProcessed = 0;
		FailedListenerInvocations = 0;
		PeakListenersPerEvent = 0;
	}

	/**
	 * Records a broadcast event and updates metrics.
	 * @param Duration Time taken for the broadcast (in seconds)
	 * @param ListenerCount Number of listeners invoked
	 * @param FailedCount Number of listeners that failed
	 */
	void RecordBroadcast(double Duration, int32 ListenerCount, int32 FailedCount = 0)
	{
		TotalEventsBroadcast++;
		TotalListenerInvocations += ListenerCount;
		FailedListenerInvocations += FailedCount;

		// Convert duration to milliseconds
		const double DurationMs = Duration * 1000.0;
		TotalEventSystemOverhead += DurationMs;

		// Update average time per broadcast
		AverageTimePerBroadcast = TotalEventSystemOverhead / TotalEventsBroadcast;

		// Update average time per listener
		if (TotalListenerInvocations > 0)
		{
			AverageTimePerListener = TotalEventSystemOverhead / TotalListenerInvocations;
		}

		// Update peak listeners
		if (ListenerCount > PeakListenersPerEvent)
		{
			PeakListenersPerEvent = ListenerCount;
		}
	}

	/**
	 * Records deferred event processing.
	 * @param EventCount Number of deferred events processed
	 */
	void RecordDeferredProcessing(int32 EventCount)
	{
		DeferredEventsProcessed += EventCount;
	}
};

/**
 * Record of a broadcast event for debugging and history tracking.
 * Contains event metadata and timing information.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FEventRecord
{
	GENERATED_BODY()

	/** The event tag that was broadcast */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FGameplayTag EventTag;

	/** Timestamp when the event was broadcast */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FDateTime Timestamp;

	/** Number of listeners that received this event */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	int32 ListenerCount = 0;

	/** Time taken to process this event (in milliseconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	double ProcessingTime = 0.0;

	/** Number of listeners that failed during this event */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	int32 FailedListenerCount = 0;

	/** Summary of the event payload (for debugging) */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FString PayloadSummary;

	FEventRecord()
		: Timestamp(FDateTime::Now())
	{}

	FEventRecord(const FGameplayTag& InEventTag, int32 InListenerCount, double InProcessingTime, int32 InFailedCount, const FString& InPayloadSummary)
		: EventTag(InEventTag)
		, Timestamp(FDateTime::Now())
		, ListenerCount(InListenerCount)
		, ProcessingTime(InProcessingTime)
		, FailedListenerCount(InFailedCount)
		, PayloadSummary(InPayloadSummary)
	{}
};
