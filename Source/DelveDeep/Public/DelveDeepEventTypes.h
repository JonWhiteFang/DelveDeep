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
