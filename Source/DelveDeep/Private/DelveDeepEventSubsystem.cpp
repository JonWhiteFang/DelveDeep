// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventCommands.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogDelveDeepEvents);

// Stat group for event system profiling
DECLARE_STATS_GROUP(TEXT("DelveDeep.Events"), STATGROUP_DelveDeepEvents, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Broadcast Event"), STAT_BroadcastEvent, STATGROUP_DelveDeepEvents);
DECLARE_CYCLE_STAT(TEXT("Invoke Listeners"), STAT_InvokeListeners, STATGROUP_DelveDeepEvents);
DECLARE_CYCLE_STAT(TEXT("Process Deferred"), STAT_ProcessDeferred, STATGROUP_DelveDeepEvents);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Listeners"), STAT_ActiveListeners, STATGROUP_DelveDeepEvents);
DECLARE_DWORD_COUNTER_STAT(TEXT("Events Per Frame"), STAT_EventsPerFrame, STATGROUP_DelveDeepEvents);

void UDelveDeepEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem initializing..."));
	
	// Initialize event registry
	EventRegistry.Empty();
	HandleToListenerMap.Empty();
	NextHandleId = 1;
	
	// Initialize deferred event system
	DeferredEventQueue.Empty();
	DeferredEventQueue.Reserve(MaxDeferredEvents);
	bDeferredMode = false;
	
	// Initialize performance metrics
	PerformanceMetrics.Reset();
	
	// Initialize event history
	EventHistory.Empty();
	EventHistory.Reserve(MaxHistorySize);
	HistoryIndex = 0;
	bEventLoggingEnabled = false;
	
	// Register console commands
	FDelveDeepEventCommands::RegisterCommands();
	
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem initialized successfully"));
}

void UDelveDeepEventSubsystem::Deinitialize()
{
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem shutting down..."));
	
	// Process any remaining deferred events
	if (DeferredEventQueue.Num() > 0)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("Processing %d remaining deferred events during shutdown"),
			DeferredEventQueue.Num());
		ProcessDeferredEvents();
	}
	
	// Clear all listeners and registry
	EventRegistry.Empty();
	HandleToListenerMap.Empty();
	DeferredEventQueue.Empty();
	EventHistory.Empty();
	
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

	// Update active listener stat
	int32 TotalListeners = 0;
	for (const auto& RegistryPair : EventRegistry)
	{
		TotalListeners += RegistryPair.Value.GetTotalListenerCount();
	}
	SET_DWORD_STAT(STAT_ActiveListeners, TotalListeners);

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

	// Update active listener stat
	int32 TotalListeners = 0;
	for (const auto& RegistryPair : EventRegistry)
	{
		TotalListeners += RegistryPair.Value.GetTotalListenerCount();
	}
	SET_DWORD_STAT(STAT_ActiveListeners, TotalListeners);
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
	// If in deferred mode, queue the event instead of broadcasting immediately
	if (bDeferredMode)
	{
		BroadcastEventDeferred(Payload);
		return;
	}

	// Otherwise broadcast immediately
	BroadcastEventImmediate(Payload);
}

void UDelveDeepEventSubsystem::BroadcastEventDeferred(const FDelveDeepEventPayload& Payload)
{
	// Check queue capacity
	if (DeferredEventQueue.Num() >= MaxDeferredEvents)
	{
		UE_LOG(LogDelveDeepEvents, Error, 
			TEXT("Deferred event queue overflow! Discarding oldest event. Queue size: %d"),
			DeferredEventQueue.Num());
		
		// Remove oldest event
		DeferredEventQueue.RemoveAt(0);
	}
	else if (DeferredEventQueue.Num() >= MaxDeferredEvents * 0.8f)
	{
		// Warn when queue reaches 80% capacity
		UE_LOG(LogDelveDeepEvents, Warning,
			TEXT("Deferred event queue at 80%% capacity: %d/%d events"),
			DeferredEventQueue.Num(), MaxDeferredEvents);
	}

	// Create a copy of the payload and queue it
	TSharedPtr<FDelveDeepEventPayload> PayloadCopy = MakeShared<FDelveDeepEventPayload>(Payload);
	DeferredEventQueue.Add(PayloadCopy);

	UE_LOG(LogDelveDeepEvents, VeryVerbose, TEXT("Queued deferred event %s (Queue size: %d)"),
		*Payload.EventTag.ToString(), DeferredEventQueue.Num());
}

void UDelveDeepEventSubsystem::EnableDeferredMode()
{
	if (bDeferredMode)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("Deferred mode already enabled"));
		return;
	}

	bDeferredMode = true;
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Deferred event processing mode enabled"));
}

void UDelveDeepEventSubsystem::DisableDeferredMode()
{
	if (!bDeferredMode)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("Deferred mode already disabled"));
		return;
	}

	bDeferredMode = false;
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Deferred event processing mode disabled"));
}

void UDelveDeepEventSubsystem::ProcessDeferredEvents()
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessDeferred);

	if (DeferredEventQueue.Num() == 0)
	{
		UE_LOG(LogDelveDeepEvents, VeryVerbose, TEXT("No deferred events to process"));
		return;
	}

	const int32 EventCount = DeferredEventQueue.Num();
	const double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogDelveDeepEvents, Display, TEXT("Processing %d deferred events..."), EventCount);

	// Process all queued events in order
	for (const TSharedPtr<FDelveDeepEventPayload>& PayloadPtr : DeferredEventQueue)
	{
		if (PayloadPtr.IsValid())
		{
			BroadcastEventImmediate(*PayloadPtr);
		}
	}

	// Clear the queue
	DeferredEventQueue.Empty();

	const double EndTime = FPlatformTime::Seconds();
	const double Duration = (EndTime - StartTime) * 1000.0; // Convert to milliseconds

	// Record deferred processing metrics
	{
		FScopeLock Lock(&MetricsMutex);
		PerformanceMetrics.RecordDeferredProcessing(EventCount);
	}

	UE_LOG(LogDelveDeepEvents, Display, 
		TEXT("Processed %d deferred events in %.2f ms (%.2f ms per event)"),
		EventCount, Duration, Duration / EventCount);

	// Warn if processing took too long
	if (Duration > 10.0)
	{
		UE_LOG(LogDelveDeepEvents, Warning,
			TEXT("Deferred event processing exceeded 10ms target: %.2f ms"), Duration);
	}
}

void UDelveDeepEventSubsystem::BroadcastEventImmediate(const FDelveDeepEventPayload& Payload)
{
	SCOPE_CYCLE_COUNTER(STAT_BroadcastEvent);
	INC_DWORD_STAT(STAT_EventsPerFrame);

	const double BroadcastStartTime = FPlatformTime::Seconds();
	int32 TotalListenersInvoked = 0;
	int32 FailedListeners = 0;

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

	// Track network-relevant events
	if (Payload.ShouldReplicate())
	{
		NetworkRelevantEventTags.Add(Payload.EventTag);
	}

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

					const double ListenerStartTime = FPlatformTime::Seconds();
					bool bListenerFailed = false;

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
						bListenerFailed = true;
						FailedListeners++;
					}
					catch (...)
					{
						UE_LOG(LogDelveDeepEvents, Error, 
							TEXT("Unknown exception in listener callback for tag %s"),
							*Tag.ToString());
						bListenerFailed = true;
						FailedListeners++;
					}

					const double ListenerEndTime = FPlatformTime::Seconds();
					const double ListenerDuration = (ListenerEndTime - ListenerStartTime) * 1000.0; // ms

					// Warn if listener took too long
					if (ListenerDuration > SlowListenerThresholdMs)
					{
						UE_LOG(LogDelveDeepEvents, Warning,
							TEXT("Slow listener callback for tag %s: %.2f ms (threshold: %.2f ms)"),
							*Tag.ToString(), ListenerDuration, SlowListenerThresholdMs);
					}
				}
			}
		}
	}

	// Record broadcast metrics
	const double BroadcastEndTime = FPlatformTime::Seconds();
	const double BroadcastDuration = BroadcastEndTime - BroadcastStartTime;
	const double BroadcastDurationMs = BroadcastDuration * 1000.0;
	
	{
		FScopeLock Lock(&MetricsMutex);
		PerformanceMetrics.RecordBroadcast(BroadcastDuration, TotalListenersInvoked, FailedListeners);
	}

	// Add to event history
	FString PayloadSummary = GeneratePayloadSummary(Payload);
	FEventRecord Record(Payload.EventTag, TotalListenersInvoked, BroadcastDurationMs, FailedListeners, PayloadSummary);
	AddToHistory(Record);

	// Log event if logging is enabled
	if (bEventLoggingEnabled)
	{
		UE_LOG(LogDelveDeepEvents, Display,
			TEXT("Event: %s | Listeners: %d | Failed: %d | Time: %.2f ms | Payload: %s"),
			*Payload.EventTag.ToString(), TotalListenersInvoked, FailedListeners, 
			BroadcastDurationMs, *PayloadSummary);
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

const FEventSystemMetrics& UDelveDeepEventSubsystem::GetPerformanceMetrics() const
{
	FScopeLock Lock(&MetricsMutex);
	return PerformanceMetrics;
}

void UDelveDeepEventSubsystem::ResetPerformanceMetrics()
{
	FScopeLock Lock(&MetricsMutex);
	PerformanceMetrics.Reset();
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Performance metrics reset"));
}

TArray<FEventRecord> UDelveDeepEventSubsystem::GetEventHistory() const
{
	FScopeLock Lock(&MetricsMutex);
	return EventHistory;
}

void UDelveDeepEventSubsystem::EnableEventLogging()
{
	bEventLoggingEnabled = true;
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event logging enabled"));
}

void UDelveDeepEventSubsystem::DisableEventLogging()
{
	bEventLoggingEnabled = false;
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event logging disabled"));
}

TArray<FGameplayTag> UDelveDeepEventSubsystem::GetNetworkRelevantEvents() const
{
	TArray<FGameplayTag> Result;
	NetworkRelevantEventTags.GenerateValueArray(Result);
	return Result;
}

void UDelveDeepEventSubsystem::AddToHistory(const FEventRecord& Record)
{
	FScopeLock Lock(&MetricsMutex);

	// If history is not yet full, just add
	if (EventHistory.Num() < MaxHistorySize)
	{
		EventHistory.Add(Record);
	}
	else
	{
		// Use circular buffer - overwrite oldest entry
		EventHistory[HistoryIndex] = Record;
		HistoryIndex = (HistoryIndex + 1) % MaxHistorySize;
	}
}

FString UDelveDeepEventSubsystem::GeneratePayloadSummary(const FDelveDeepEventPayload& Payload) const
{
	// Generate a brief summary of the payload for debugging
	FString Summary = FString::Printf(TEXT("Tag: %s"), *Payload.EventTag.ToString());

	if (Payload.Instigator.IsValid())
	{
		Summary += FString::Printf(TEXT(", Instigator: %s"), *Payload.Instigator->GetName());
	}

	// Add timestamp
	Summary += FString::Printf(TEXT(", Time: %s"), *Payload.Timestamp.ToString());

	return Summary;
}
