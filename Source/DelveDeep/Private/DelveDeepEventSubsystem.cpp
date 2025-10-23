// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogDelveDeepEvents);

// Stat group for event system profiling
DECLARE_STATS_GROUP(TEXT("DelveDeep.Events"), STATGROUP_DelveDeepEvents, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Broadcast Event"), STAT_BroadcastEvent, STATGROUP_DelveDeepEvents);
DECLARE_CYCLE_STAT(TEXT("Invoke Listeners"), STAT_InvokeListeners, STATGROUP_DelveDeepEvents);

void UDelveDeepEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem initializing..."));
	
	// Initialize event registry
	EventRegistry.Empty();
	HandleToListenerMap.Empty();
	NextHandleId = 1;
	
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem initialized successfully"));
}

void UDelveDeepEventSubsystem::Deinitialize()
{
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem shutting down..."));
	
	// Clear all listeners and registry
	EventRegistry.Empty();
	HandleToListenerMap.Empty();
	
	Super::Deinitialize();
}

FDelegateHandle UDelveDeepEventSubsystem::RegisterListener(
	FGameplayTag EventTag,
	TFunction<void(const FDelveDeepEventPayload&)> Callback,
	UObject* Owner,
	EDelveDeepEventPriority Priority,
	const FDelveDeepEventFilter& Filter)
{
	// Validate inputs
	if (!EventTag.IsValid())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("RegisterListener: Invalid EventTag"));
		return FDelegateHandle();
	}

	if (!Callback)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("RegisterListener: Invalid Callback for tag %s"), *EventTag.ToString());
		return FDelegateHandle();
	}

	if (!Owner)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("RegisterListener: Invalid Owner for tag %s"), *EventTag.ToString());
		return FDelegateHandle();
	}

	// Generate unique handle
	FDelegateHandle Handle = GenerateUniqueHandle();

	// Create listener
	FDelveDeepEventListener Listener;
	Listener.Callback = Callback;
	Listener.Owner = Owner;
	Listener.Priority = Priority;
	Listener.Filter = Filter;
	Listener.RegistrationTime = FDateTime::Now();
	Listener.InvocationCount = 0;

	// Get or create listener list for this event tag
	FEventListenerList& ListenerList = EventRegistry.FindOrAdd(EventTag);

	// Add listener to appropriate priority array
	TArray<FDelveDeepEventListener>& PriorityArray = ListenerList.GetListenerArrayForPriority(Priority);
	int32 Index = PriorityArray.Add(Listener);

	// Store handle-to-location mapping
	FListenerLocation Location;
	Location.EventTag = EventTag;
	Location.Priority = Priority;
	Location.IndexInArray = Index;
	HandleToListenerMap.Add(Handle, Location);

	UE_LOG(LogDelveDeepEvents, Verbose, TEXT("Registered listener for tag %s (Priority: %d, Total listeners: %d)"),
		*EventTag.ToString(), static_cast<int32>(Priority), ListenerList.GetTotalListenerCount());

	return Handle;
}

FDelegateHandle UDelveDeepEventSubsystem::GenerateUniqueHandle()
{
	// Create a unique handle using an incrementing ID
	// FDelegateHandle expects a uint64 ID
	uint64 HandleId = static_cast<uint64>(NextHandleId++);
	return FDelegateHandle(HandleId);
}

void UDelveDeepEventSubsystem::UnregisterListener(FDelegateHandle Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("UnregisterListener: Invalid handle"));
		return;
	}

	// Find the listener location
	FListenerLocation* Location = HandleToListenerMap.Find(Handle);
	if (!Location)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("UnregisterListener: Handle not found in registry"));
		return;
	}

	// Find the listener list for this event tag
	FEventListenerList* ListenerList = EventRegistry.Find(Location->EventTag);
	if (!ListenerList)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("UnregisterListener: Event tag %s not found in registry"), 
			*Location->EventTag.ToString());
		HandleToListenerMap.Remove(Handle);
		return;
	}

	// Get the appropriate priority array
	TArray<FDelveDeepEventListener>& PriorityArray = ListenerList->GetListenerArrayForPriority(Location->Priority);

	// Validate index
	if (!PriorityArray.IsValidIndex(Location->IndexInArray))
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("UnregisterListener: Invalid index %d for tag %s"),
			Location->IndexInArray, *Location->EventTag.ToString());
		HandleToListenerMap.Remove(Handle);
		return;
	}

	// Remove the listener
	RemoveListenerFromArray(PriorityArray, Location->IndexInArray, Location->EventTag, Location->Priority);

	// Remove the handle mapping
	HandleToListenerMap.Remove(Handle);

	// Update indices for all listeners after the removed one
	for (auto& Pair : HandleToListenerMap)
	{
		FListenerLocation& OtherLocation = Pair.Value;
		if (OtherLocation.EventTag == Location->EventTag && 
			OtherLocation.Priority == Location->Priority &&
			OtherLocation.IndexInArray > Location->IndexInArray)
		{
			OtherLocation.IndexInArray--;
		}
	}

	UE_LOG(LogDelveDeepEvents, Verbose, TEXT("Unregistered listener for tag %s (Priority: %d, Remaining: %d)"),
		*Location->EventTag.ToString(), static_cast<int32>(Location->Priority), 
		ListenerList->GetTotalListenerCount());
}

void UDelveDeepEventSubsystem::UnregisterAllListeners(UObject* Owner)
{
	if (!Owner)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("UnregisterAllListeners: Invalid owner"));
		return;
	}

	int32 RemovedCount = 0;

	// Collect handles to remove (can't modify map while iterating)
	TArray<FDelegateHandle> HandlesToRemove;
	for (const auto& Pair : HandleToListenerMap)
	{
		const FListenerLocation& Location = Pair.Value;
		FEventListenerList* ListenerList = EventRegistry.Find(Location.EventTag);
		if (ListenerList)
		{
			const TArray<FDelveDeepEventListener>& PriorityArray = 
				ListenerList->GetListenerArrayForPriority(Location.Priority);
			
			if (PriorityArray.IsValidIndex(Location.IndexInArray))
			{
				const FDelveDeepEventListener& Listener = PriorityArray[Location.IndexInArray];
				if (Listener.Owner.Get() == Owner)
				{
					HandlesToRemove.Add(Pair.Key);
				}
			}
		}
	}

	// Remove all collected handles
	for (const FDelegateHandle& Handle : HandlesToRemove)
	{
		UnregisterListener(Handle);
		RemovedCount++;
	}

	if (RemovedCount > 0)
	{
		UE_LOG(LogDelveDeepEvents, Display, TEXT("Unregistered %d listeners for owner %s"),
			RemovedCount, *Owner->GetName());
	}
}

void UDelveDeepEventSubsystem::CleanupStaleListeners()
{
	int32 RemovedCount = 0;

	// Iterate through all event tags
	for (auto& Pair : EventRegistry)
	{
		FGameplayTag EventTag = Pair.Key;
		FEventListenerList& ListenerList = Pair.Value;

		// Check each priority level
		for (int32 PriorityIndex = 0; PriorityIndex < 3; ++PriorityIndex)
		{
			EDelveDeepEventPriority Priority = static_cast<EDelveDeepEventPriority>(PriorityIndex);
			TArray<FDelveDeepEventListener>& PriorityArray = 
				ListenerList.GetListenerArrayForPriority(Priority);

			// Remove stale listeners (iterate backwards to avoid index issues)
			for (int32 i = PriorityArray.Num() - 1; i >= 0; --i)
			{
				if (!PriorityArray[i].IsOwnerValid())
				{
					// Find and remove the handle mapping
					for (auto It = HandleToListenerMap.CreateIterator(); It; ++It)
					{
						const FListenerLocation& Location = It.Value();
						if (Location.EventTag == EventTag && 
							Location.Priority == Priority && 
							Location.IndexInArray == i)
						{
							It.RemoveCurrent();
							break;
						}
					}

					// Remove the listener
					PriorityArray.RemoveAt(i);
					RemovedCount++;

					// Update indices for remaining listeners
					for (auto& HandlePair : HandleToListenerMap)
					{
						FListenerLocation& Location = HandlePair.Value;
						if (Location.EventTag == EventTag && 
							Location.Priority == Priority && 
							Location.IndexInArray > i)
						{
							Location.IndexInArray--;
						}
					}
				}
			}
		}
	}

	if (RemovedCount > 0)
	{
		UE_LOG(LogDelveDeepEvents, Verbose, TEXT("Cleaned up %d stale listeners"), RemovedCount);
	}
}

void UDelveDeepEventSubsystem::RemoveListenerFromArray(
	TArray<FDelveDeepEventListener>& ListenerArray,
	int32 Index,
	const FGameplayTag& EventTag,
	EDelveDeepEventPriority Priority)
{
	if (!ListenerArray.IsValidIndex(Index))
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("RemoveListenerFromArray: Invalid index %d"), Index);
		return;
	}

	ListenerArray.RemoveAt(Index);
}

void UDelveDeepEventSubsystem::BroadcastEvent(const FDelveDeepEventPayload& Payload)
{
	SCOPE_CYCLE_COUNTER(STAT_BroadcastEvent);

	// Validate event tag
	if (!Payload.EventTag.IsValid())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("BroadcastEvent: Invalid event tag"));
		return;
	}

	// Validate payload in development builds
#if !UE_BUILD_SHIPPING
	FValidationContext ValidationContext;
	if (!Payload.Validate(ValidationContext))
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("BroadcastEvent: Payload validation failed for tag %s: %s"),
			*Payload.EventTag.ToString(), *ValidationContext.GetReport());
		return;
	}
#endif

	// Find all matching event tags (including parent tags for hierarchical matching)
	TArray<FGameplayTag> MatchingTags;
	MatchingTags.Add(Payload.EventTag);

	// Add parent tags for hierarchical matching
	FGameplayTag CurrentTag = Payload.EventTag;
	while (CurrentTag.RequestDirectParent().IsValid())
	{
		CurrentTag = CurrentTag.RequestDirectParent();
		MatchingTags.Add(CurrentTag);
	}

	int32 TotalListenersInvoked = 0;

	// Broadcast to all matching tags
	for (const FGameplayTag& Tag : MatchingTags)
	{
		FEventListenerList* ListenerList = EventRegistry.Find(Tag);
		if (!ListenerList)
		{
			continue;
		}

		// Invoke listeners in priority order: High -> Normal -> Low
		const TArray<EDelveDeepEventPriority> PriorityOrder = {
			EDelveDeepEventPriority::High,
			EDelveDeepEventPriority::Normal,
			EDelveDeepEventPriority::Low
		};

		for (EDelveDeepEventPriority Priority : PriorityOrder)
		{
			TArray<FDelveDeepEventListener>& PriorityArray = 
				ListenerList->GetListenerArrayForPriority(Priority);

			// Iterate through listeners (make a copy to handle modifications during iteration)
			TArray<FDelveDeepEventListener> ListenersCopy = PriorityArray;
			for (FDelveDeepEventListener& Listener : ListenersCopy)
			{
				// Check if owner is still valid
				if (!Listener.IsOwnerValid())
				{
					continue;
				}

				// Apply filter
				if (!Listener.Filter.PassesFilter(Payload))
				{
					continue;
				}

				// Invoke listener callback
				if (Listener.Callback)
				{
					SCOPE_CYCLE_COUNTER(STAT_InvokeListeners);

					try
					{
						Listener.Callback(Payload);
						TotalListenersInvoked++;
					}
					catch (const std::exception& e)
					{
						UE_LOG(LogDelveDeepEvents, Error, 
							TEXT("Exception in listener callback for tag %s: %s"),
							*Tag.ToString(), ANSI_TO_TCHAR(e.what()));
					}
					catch (...)
					{
						UE_LOG(LogDelveDeepEvents, Error, 
							TEXT("Unknown exception in listener callback for tag %s"),
							*Tag.ToString());
					}
				}
			}
		}
	}

	UE_LOG(LogDelveDeepEvents, VeryVerbose, TEXT("Broadcast event %s to %d listeners"),
		*Payload.EventTag.ToString(), TotalListenersInvoked);
}

int32 UDelveDeepEventSubsystem::GetListenerCount(FGameplayTag EventTag) const
{
	if (!EventTag.IsValid())
	{
		return 0;
	}

	const FEventListenerList* ListenerList = EventRegistry.Find(EventTag);
	if (!ListenerList)
	{
		return 0;
	}

	return ListenerList->GetTotalListenerCount();
}
