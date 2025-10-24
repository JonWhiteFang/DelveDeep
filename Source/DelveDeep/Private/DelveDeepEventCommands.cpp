// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventCommands.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

// Console command declarations
static FAutoConsoleCommand ListListenersCmd(
	TEXT("DelveDeep.Events.ListListeners"),
	TEXT("Lists all listeners for a specific event tag. Usage: DelveDeep.Events.ListListeners <EventTag>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ListListeners)
);

static FAutoConsoleCommand ListAllListenersCmd(
	TEXT("DelveDeep.Events.ListAllListeners"),
	TEXT("Lists all registered listeners across all event tags"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ListAllListeners)
);

static FAutoConsoleCommand ShowMetricsCmd(
	TEXT("DelveDeep.Events.ShowMetrics"),
	TEXT("Shows performance metrics for the event system"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ShowMetrics)
);

static FAutoConsoleCommand ResetMetricsCmd(
	TEXT("DelveDeep.Events.ResetMetrics"),
	TEXT("Resets performance metrics to zero"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ResetMetrics)
);

static FAutoConsoleCommand ShowEventHistoryCmd(
	TEXT("DelveDeep.Events.ShowEventHistory"),
	TEXT("Shows recent event history"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ShowEventHistory)
);

static FAutoConsoleCommand EnableEventLoggingCmd(
	TEXT("DelveDeep.Events.EnableEventLogging"),
	TEXT("Enables detailed event logging"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::EnableEventLogging)
);

static FAutoConsoleCommand DisableEventLoggingCmd(
	TEXT("DelveDeep.Events.DisableEventLogging"),
	TEXT("Disables detailed event logging"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::DisableEventLogging)
);

static FAutoConsoleCommand BroadcastTestEventCmd(
	TEXT("DelveDeep.Events.BroadcastTestEvent"),
	TEXT("Broadcasts a test event. Usage: DelveDeep.Events.BroadcastTestEvent <EventTag>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::BroadcastTestEvent)
);

static FAutoConsoleCommand ValidateAllPayloadsCmd(
	TEXT("DelveDeep.Events.ValidateAllPayloads"),
	TEXT("Validates all event payload types"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ValidateAllPayloads)
);

static FAutoConsoleCommand EnableValidationCmd(
	TEXT("DelveDeep.Events.EnableValidation"),
	TEXT("Enables payload validation for event broadcasting (development builds only)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::EnableValidation)
);

static FAutoConsoleCommand DisableValidationCmd(
	TEXT("DelveDeep.Events.DisableValidation"),
	TEXT("Disables payload validation for event broadcasting (development builds only)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::DisableValidation)
);

static FAutoConsoleCommand ClearAllListenersCmd(
	TEXT("DelveDeep.Events.ClearAllListeners"),
	TEXT("Clears all registered listeners (use with caution!)"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::ClearAllListeners)
);

static FAutoConsoleCommand DumpEventRegistryCmd(
	TEXT("DelveDeep.Events.DumpEventRegistry"),
	TEXT("Dumps the event registry structure"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&FDelveDeepEventCommands::DumpEventRegistry)
);

void FDelveDeepEventCommands::RegisterCommands()
{
	// Commands are registered automatically via FAutoConsoleCommand
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event system console commands registered"));
}

void FDelveDeepEventCommands::UnregisterCommands()
{
	// Commands are unregistered automatically
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event system console commands unregistered"));
}

UDelveDeepEventSubsystem* FDelveDeepEventCommands::GetEventSubsystem()
{
	if (!GEngine || !GEngine->GetWorld())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("Cannot get event subsystem: No valid world"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorld();
	if (!World || !World->GetGameInstance())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("Cannot get event subsystem: No valid game instance"));
		return nullptr;
	}

	UDelveDeepEventSubsystem* EventSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepEventSubsystem>();
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("Cannot get event subsystem: Subsystem not found"));
		return nullptr;
	}

	return EventSubsystem;
}

void FDelveDeepEventCommands::ListListeners(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	if (Args.Num() == 0)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("Usage: DelveDeep.Events.ListListeners <EventTag>"));
		return;
	}

	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
	if (!EventTag.IsValid())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("Invalid event tag: %s"), *Args[0]);
		return;
	}

	int32 ListenerCount = EventSubsystem->GetListenerCount(EventTag);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Tag: %s"), *EventTag.ToString());
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Total Listeners: %d"), ListenerCount);
}

void FDelveDeepEventCommands::ListAllListeners(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	UE_LOG(LogDelveDeepEvents, Display, TEXT("=== All Registered Event Listeners ==="));
	UE_LOG(LogDelveDeepEvents, Display, TEXT("(Note: Detailed listener enumeration requires additional implementation)"));
}

void FDelveDeepEventCommands::ShowMetrics(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	const FEventSystemMetrics& Metrics = EventSubsystem->GetPerformanceMetrics();

	UE_LOG(LogDelveDeepEvents, Display, TEXT("=== Event System Performance Metrics ==="));
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Total Events Broadcast: %d"), Metrics.TotalEventsBroadcast);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Total Listener Invocations: %d"), Metrics.TotalListenerInvocations);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Failed Listener Invocations: %d"), Metrics.FailedListenerInvocations);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Average Time Per Broadcast: %.4f ms"), Metrics.AverageTimePerBroadcast);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Average Time Per Listener: %.4f ms"), Metrics.AverageTimePerListener);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Total System Overhead: %.2f ms"), Metrics.TotalEventSystemOverhead);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Deferred Events Processed: %d"), Metrics.DeferredEventsProcessed);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Peak Listeners Per Event: %d"), Metrics.PeakListenersPerEvent);
}

void FDelveDeepEventCommands::ResetMetrics(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	EventSubsystem->ResetPerformanceMetrics();
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Performance metrics reset"));
}

void FDelveDeepEventCommands::ShowEventHistory(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	TArray<FEventRecord> History = EventSubsystem->GetEventHistory();

	UE_LOG(LogDelveDeepEvents, Display, TEXT("=== Event History (Last %d Events) ==="), History.Num());

	for (int32 i = 0; i < History.Num(); ++i)
	{
		const FEventRecord& Record = History[i];
		UE_LOG(LogDelveDeepEvents, Display, 
			TEXT("[%d] %s | Tag: %s | Listeners: %d | Failed: %d | Time: %.2f ms"),
			i + 1,
			*Record.Timestamp.ToString(),
			*Record.EventTag.ToString(),
			Record.ListenerCount,
			Record.FailedListenerCount,
			Record.ProcessingTime);
	}

	if (History.Num() == 0)
	{
		UE_LOG(LogDelveDeepEvents, Display, TEXT("No events in history"));
	}
}

void FDelveDeepEventCommands::EnableEventLogging(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	EventSubsystem->EnableEventLogging();
}

void FDelveDeepEventCommands::DisableEventLogging(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	EventSubsystem->DisableEventLogging();
}

void FDelveDeepEventCommands::BroadcastTestEvent(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	if (Args.Num() == 0)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("Usage: DelveDeep.Events.BroadcastTestEvent <EventTag>"));
		return;
	}

	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
	if (!EventTag.IsValid())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("Invalid event tag: %s"), *Args[0]);
		return;
	}

	// Create a test payload
	FDelveDeepEventPayload TestPayload;
	TestPayload.EventTag = EventTag;
	TestPayload.Timestamp = FDateTime::Now();
	TestPayload.Instigator = nullptr;

	UE_LOG(LogDelveDeepEvents, Display, TEXT("Broadcasting test event: %s"), *EventTag.ToString());
	EventSubsystem->BroadcastEvent(TestPayload);
}

void FDelveDeepEventCommands::ValidateAllPayloads(const TArray<FString>& Args)
{
	UE_LOG(LogDelveDeepEvents, Display, TEXT("=== Validating Event Payload Types ==="));

	// Test base payload
	FDelveDeepEventPayload BasePayload;
	BasePayload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));
	
	FValidationContext Context;
	Context.SystemName = TEXT("EventCommands");
	Context.OperationName = TEXT("ValidatePayloads");

	bool bValid = BasePayload.Validate(Context);
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Base Payload: %s"), bValid ? TEXT("VALID") : TEXT("INVALID"));
	
	if (!bValid)
	{
		UE_LOG(LogDelveDeepEvents, Display, TEXT("Validation Report: %s"), *Context.GetReport());
	}

	UE_LOG(LogDelveDeepEvents, Display, TEXT("Payload validation complete"));
}

void FDelveDeepEventCommands::EnableValidation(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	EventSubsystem->EnableValidation();
}

void FDelveDeepEventCommands::DisableValidation(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	EventSubsystem->DisableValidation();
}

void FDelveDeepEventCommands::ClearAllListeners(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	UE_LOG(LogDelveDeepEvents, Warning, TEXT("ClearAllListeners is not yet implemented"));
	UE_LOG(LogDelveDeepEvents, Warning, TEXT("This would require exposing internal registry access"));
}

void FDelveDeepEventCommands::DumpEventRegistry(const TArray<FString>& Args)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem();
	if (!EventSubsystem)
	{
		return;
	}

	UE_LOG(LogDelveDeepEvents, Display, TEXT("=== Event Registry Dump ==="));
	UE_LOG(LogDelveDeepEvents, Display, TEXT("(Note: Detailed registry dump requires additional implementation)"));
}
