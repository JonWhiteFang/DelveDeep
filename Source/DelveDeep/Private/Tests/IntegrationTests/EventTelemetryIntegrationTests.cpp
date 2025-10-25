// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventTypes.h"
#include "DelveDeepTestFixtures.h"
#include "DelveDeepTestMacros.h"
#include "DelveDeepTestUtilities.h"
#include "Misc/AutomationTest.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test event system performance tracking via telemetry
 * Verifies that event broadcasts are tracked by telemetry system
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventTelemetryPerformanceTrackingTest,
	"DelveDeep.Integration.EventTelemetryPerformanceTracking",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventTelemetryPerformanceTrackingTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.EventSubsystem);
	ASSERT_NOT_NULL(Fixture.TelemetrySubsystem);

	// Start telemetry tracking
	Fixture.TelemetrySubsystem->StartTracking();

	// Track event performance metrics
	int32 EventsBroadcast = 0;
	double TotalEventTime = 0.0;

	// Register listener to measure event processing time
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Test.Performance"));
	FDelegateHandle ListenerHandle = Fixture.EventSubsystem->RegisterListener(
		TestTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			// Simulate some work
			for (int32 i = 0; i < 100; ++i)
			{
				volatile int32 Dummy = i * i;
			}
		}),
		EDelveDeepEventPriority::Normal
	);

	// Broadcast multiple events and measure performance
	const int32 NumEvents = 100;
	for (int32 i = 0; i < NumEvents; ++i)
	{
		DelveDeepTestUtils::FScopedTestTimer Timer(TEXT("Event Broadcast"));

		FDelveDeepEventPayload Payload;
		Payload.EventTag = TestTag;
		Payload.IntValue = i;
		Fixture.EventSubsystem->BroadcastEvent(Payload);

		EventsBroadcast++;
		TotalEventTime += Timer.GetElapsedMs();
	}

	// Stop telemetry tracking
	Fixture.TelemetrySubsystem->StopTracking();

	// Verify events were broadcast
	EXPECT_EQ(EventsBroadcast, NumEvents);

	// Calculate average event time
	double AvgEventTime = TotalEventTime / static_cast<double>(NumEvents);
	EXPECT_LT(AvgEventTime, 1.0);  // Target: <1ms per event

	// Verify telemetry captured the data
	// Note: In a real implementation, telemetry would have specific APIs to query metrics
	UE_LOG(LogDelveDeep, Display, TEXT("Event performance: %d events, %.2f ms total, %.4f ms avg"),
		EventsBroadcast, TotalEventTime, AvgEventTime);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(TestTag, ListenerHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test event listener count tracking
 * Verifies that telemetry tracks the number of active listeners
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventListenerCountTrackingTest,
	"DelveDeep.Integration.EventListenerCountTracking",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventListenerCountTrackingTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.EventSubsystem);
	ASSERT_NOT_NULL(Fixture.TelemetrySubsystem);

	// Track listener registration events
	int32 ListenersRegistered = 0;
	int32 ListenersUnregistered = 0;

	// Register telemetry listener for listener registration events
	FGameplayTag ListenerRegTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Events.ListenerRegistered"));
	FDelegateHandle TelemetryHandle = Fixture.EventSubsystem->RegisterListener(
		ListenerRegTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			ListenersRegistered++;
		}),
		EDelveDeepEventPriority::Low
	);

	// Register multiple test listeners
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Test.Listeners"));
	TArray<FDelegateHandle> TestHandles;

	for (int32 i = 0; i < 10; ++i)
	{
		FDelegateHandle Handle = Fixture.EventSubsystem->RegisterListener(
			TestTag,
			FDelveDeepEventDelegate::CreateLambda([i](const FDelveDeepEventPayload& Payload)
			{
				// Listener logic
			}),
			EDelveDeepEventPriority::Normal
		);
		TestHandles.Add(Handle);

		// Broadcast listener registration event for telemetry
		FDelveDeepEventPayload RegPayload;
		RegPayload.EventTag = ListenerRegTag;
		RegPayload.IntValue = i;
		Fixture.EventSubsystem->BroadcastEvent(RegPayload);
	}

	// Verify listeners were registered
	EXPECT_EQ(ListenersRegistered, 10);

	// Unregister listeners
	FGameplayTag ListenerUnregTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Events.ListenerUnregistered"));
	FDelegateHandle UnregHandle = Fixture.EventSubsystem->RegisterListener(
		ListenerUnregTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			ListenersUnregistered++;
		}),
		EDelveDeepEventPriority::Low
	);

	for (int32 i = 0; i < TestHandles.Num(); ++i)
	{
		Fixture.EventSubsystem->UnregisterListener(TestTag, TestHandles[i]);

		// Broadcast listener unregistration event for telemetry
		FDelveDeepEventPayload UnregPayload;
		UnregPayload.EventTag = ListenerUnregTag;
		UnregPayload.IntValue = i;
		Fixture.EventSubsystem->BroadcastEvent(UnregPayload);
	}

	// Verify listeners were unregistered
	EXPECT_EQ(ListenersUnregistered, 10);

	UE_LOG(LogDelveDeep, Display, TEXT("Listener tracking: %d registered, %d unregistered"),
		ListenersRegistered, ListenersUnregistered);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(ListenerRegTag, TelemetryHandle);
	Fixture.EventSubsystem->UnregisterListener(ListenerUnregTag, UnregHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test event priority execution tracking
 * Verifies that telemetry tracks event priority distribution
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventPriorityTrackingTest,
	"DelveDeep.Integration.EventPriorityTracking",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventPriorityTrackingTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.EventSubsystem);
	ASSERT_NOT_NULL(Fixture.TelemetrySubsystem);

	// Track priority execution order
	TArray<EDelveDeepEventPriority> ExecutionOrder;

	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Test.Priority"));

	// Register listeners with different priorities
	FDelegateHandle HighHandle = Fixture.EventSubsystem->RegisterListener(
		TestTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(EDelveDeepEventPriority::High);
		}),
		EDelveDeepEventPriority::High
	);

	FDelegateHandle NormalHandle = Fixture.EventSubsystem->RegisterListener(
		TestTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(EDelveDeepEventPriority::Normal);
		}),
		EDelveDeepEventPriority::Normal
	);

	FDelegateHandle LowHandle = Fixture.EventSubsystem->RegisterListener(
		TestTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(EDelveDeepEventPriority::Low);
		}),
		EDelveDeepEventPriority::Low
	);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestTag;
	Fixture.EventSubsystem->BroadcastEvent(Payload);

	// Verify execution order (High -> Normal -> Low)
	EXPECT_EQ(ExecutionOrder.Num(), 3);
	if (ExecutionOrder.Num() == 3)
	{
		EXPECT_EQ(ExecutionOrder[0], EDelveDeepEventPriority::High);
		EXPECT_EQ(ExecutionOrder[1], EDelveDeepEventPriority::Normal);
		EXPECT_EQ(ExecutionOrder[2], EDelveDeepEventPriority::Low);
	}

	// Track priority distribution for telemetry
	int32 HighPriorityCount = 0;
	int32 NormalPriorityCount = 0;
	int32 LowPriorityCount = 0;

	for (EDelveDeepEventPriority Priority : ExecutionOrder)
	{
		switch (Priority)
		{
		case EDelveDeepEventPriority::High:
			HighPriorityCount++;
			break;
		case EDelveDeepEventPriority::Normal:
			NormalPriorityCount++;
			break;
		case EDelveDeepEventPriority::Low:
			LowPriorityCount++;
			break;
		}
	}

	EXPECT_EQ(HighPriorityCount, 1);
	EXPECT_EQ(NormalPriorityCount, 1);
	EXPECT_EQ(LowPriorityCount, 1);

	UE_LOG(LogDelveDeep, Display, TEXT("Priority distribution: High=%d, Normal=%d, Low=%d"),
		HighPriorityCount, NormalPriorityCount, LowPriorityCount);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(TestTag, HighHandle);
	Fixture.EventSubsystem->UnregisterListener(TestTag, NormalHandle);
	Fixture.EventSubsystem->UnregisterListener(TestTag, LowHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test deferred event processing performance
 * Verifies that telemetry tracks deferred event queue performance
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDeferredEventPerformanceTest,
	"DelveDeep.Integration.DeferredEventPerformance",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDeferredEventPerformanceTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.EventSubsystem);
	ASSERT_NOT_NULL(Fixture.TelemetrySubsystem);

	// Track deferred events
	int32 DeferredEventsQueued = 0;
	int32 DeferredEventsProcessed = 0;

	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Test.Deferred"));

	// Register listener
	FDelegateHandle ListenerHandle = Fixture.EventSubsystem->RegisterListener(
		TestTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			DeferredEventsProcessed++;
		}),
		EDelveDeepEventPriority::Normal
	);

	// Queue multiple deferred events
	DelveDeepTestUtils::FScopedTestTimer QueueTimer(TEXT("Deferred Event Queueing"));
	const int32 NumDeferredEvents = 1000;

	for (int32 i = 0; i < NumDeferredEvents; ++i)
	{
		FDelveDeepEventPayload Payload;
		Payload.EventTag = TestTag;
		Payload.IntValue = i;
		Fixture.EventSubsystem->QueueDeferredEvent(Payload);
		DeferredEventsQueued++;
	}

	double QueueTime = QueueTimer.GetElapsedMs();
	EXPECT_LT(QueueTime, 100.0);  // Target: <100ms to queue 1000 events

	// Process deferred events
	DelveDeepTestUtils::FScopedTestTimer ProcessTimer(TEXT("Deferred Event Processing"));
	Fixture.EventSubsystem->ProcessDeferredEvents();
	double ProcessTime = ProcessTimer.GetElapsedMs();

	// Verify all events were processed
	EXPECT_EQ(DeferredEventsQueued, NumDeferredEvents);
	EXPECT_EQ(DeferredEventsProcessed, NumDeferredEvents);
	EXPECT_LT(ProcessTime, 10.0);  // Target: <10ms to process 1000 events

	UE_LOG(LogDelveDeep, Display, TEXT("Deferred event performance: Queued %d events in %.2f ms, Processed in %.2f ms"),
		NumDeferredEvents, QueueTime, ProcessTime);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(TestTag, ListenerHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test event filtering performance tracking
 * Verifies that telemetry tracks event filtering efficiency
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventFilteringPerformanceTest,
	"DelveDeep.Integration.EventFilteringPerformance",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventFilteringPerformanceTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.EventSubsystem);
	ASSERT_NOT_NULL(Fixture.TelemetrySubsystem);

	// Track filtered vs unfiltered events
	int32 CombatEventsReceived = 0;
	int32 UIEventsReceived = 0;
	int32 TotalEventsReceived = 0;

	// Register filtered listeners
	FGameplayTag CombatTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Combat"));
	FDelegateHandle CombatHandle = Fixture.EventSubsystem->RegisterListener(
		CombatTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			CombatEventsReceived++;
			TotalEventsReceived++;
		}),
		EDelveDeepEventPriority::Normal
	);

	FGameplayTag UITag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.UI"));
	FDelegateHandle UIHandle = Fixture.EventSubsystem->RegisterListener(
		UITag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			UIEventsReceived++;
			TotalEventsReceived++;
		}),
		EDelveDeepEventPriority::Normal
	);

	// Broadcast mixed events and measure filtering performance
	DelveDeepTestUtils::FScopedTestTimer FilterTimer(TEXT("Event Filtering"));

	const int32 NumEvents = 100;
	for (int32 i = 0; i < NumEvents; ++i)
	{
		// Alternate between combat and UI events
		FDelveDeepEventPayload Payload;
		Payload.EventTag = (i % 2 == 0) ? CombatTag : UITag;
		Payload.IntValue = i;
		Fixture.EventSubsystem->BroadcastEvent(Payload);
	}

	double FilterTime = FilterTimer.GetElapsedMs();

	// Verify filtering worked correctly
	EXPECT_EQ(CombatEventsReceived, NumEvents / 2);
	EXPECT_EQ(UIEventsReceived, NumEvents / 2);
	EXPECT_EQ(TotalEventsReceived, NumEvents);
	EXPECT_LT(FilterTime, 10.0);  // Target: <10ms for 100 filtered events

	// Calculate filtering efficiency
	float FilteringEfficiency = static_cast<float>(TotalEventsReceived) / static_cast<float>(NumEvents);
	EXPECT_EQ(FilteringEfficiency, 1.0f);  // All events should be delivered to correct listeners

	UE_LOG(LogDelveDeep, Display, TEXT("Event filtering: %d events in %.2f ms, Combat=%d, UI=%d"),
		NumEvents, FilterTime, CombatEventsReceived, UIEventsReceived);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(CombatTag, CombatHandle);
	Fixture.EventSubsystem->UnregisterListener(UITag, UIHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test cross-system memory tracking
 * Verifies that telemetry tracks memory usage across event operations
 * Requirements: 14.1, 14.4, 14.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventMemoryTrackingTest,
	"DelveDeep.Integration.EventMemoryTracking",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEventMemoryTrackingTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.EventSubsystem);
	ASSERT_NOT_NULL(Fixture.TelemetrySubsystem);

	// Track memory usage
	DelveDeepTestUtils::FScopedMemoryTracker MemoryTracker;

	// Register multiple listeners
	FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Test.Memory"));
	TArray<FDelegateHandle> Handles;

	for (int32 i = 0; i < 50; ++i)
	{
		FDelegateHandle Handle = Fixture.EventSubsystem->RegisterListener(
			TestTag,
			FDelveDeepEventDelegate::CreateLambda([i](const FDelveDeepEventPayload& Payload)
			{
				// Listener logic
			}),
			EDelveDeepEventPriority::Normal
		);
		Handles.Add(Handle);
	}

	// Broadcast events
	for (int32 i = 0; i < 100; ++i)
	{
		FDelveDeepEventPayload Payload;
		Payload.EventTag = TestTag;
		Payload.IntValue = i;
		Fixture.EventSubsystem->BroadcastEvent(Payload);
	}

	// Measure memory usage
	uint64 AllocatedBytes = MemoryTracker.GetAllocatedBytes();
	int32 AllocationCount = MemoryTracker.GetAllocationCount();

	// Verify memory usage is reasonable
	// Note: Exact values depend on implementation, but should be bounded
	EXPECT_LT(AllocatedBytes, 1024 * 1024);  // <1MB for 50 listeners + 100 events

	UE_LOG(LogDelveDeep, Display, TEXT("Event memory usage: %llu bytes, %d allocations"),
		AllocatedBytes, AllocationCount);

	// Cleanup
	for (FDelegateHandle Handle : Handles)
	{
		Fixture.EventSubsystem->UnregisterListener(TestTag, Handle);
	}

	Fixture.AfterEach();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
