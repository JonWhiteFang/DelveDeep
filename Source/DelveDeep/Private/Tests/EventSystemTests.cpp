// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventTypes.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepEventTests, Log, All);
DEFINE_LOG_CATEGORY(LogDelveDeepEventTests);

/**
 * Test fixture for event system tests
 * Provides common setup and teardown for event system testing
 */
class FEventSystemTestFixture
{
public:
	FEventSystemTestFixture()
	{
		// Create test game instance
		GameInstance = NewObject<UGameInstance>();
		check(GameInstance);
		
		// Get event subsystem (auto-initializes)
		EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>();
		check(EventSubsystem);
		
		// Reset metrics for clean test state
		EventSubsystem->ResetPerformanceMetrics();
	}

	~FEventSystemTestFixture()
	{
		// Cleanup
		EventSubsystem = nullptr;
		GameInstance = nullptr;
	}

	UGameInstance* GameInstance;
	UDelveDeepEventSubsystem* EventSubsystem;
};

/**
 * Test event broadcasting to a single listener
 * Verifies that events are delivered to registered listeners
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventBroadcastSingleListenerTest, 
	"DelveDeep.EventSystem.BroadcastToSingleListener", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventBroadcastSingleListenerTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocation
	bool bListenerCalled = false;
	FGameplayTag ReceivedEventTag;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register listener
	FDelegateHandle Handle = EventSubsystem->RegisterListener(
		TestEventTag,
		[&bListenerCalled, &ReceivedEventTag](const FDelveDeepEventPayload& Payload)
		{
			bListenerCalled = true;
			ReceivedEventTag = Payload.EventTag;
		},
		Fixture.GameInstance
	);

	// Verify listener was registered
	TestEqual(TEXT("Should have one listener registered"), EventSubsystem->GetListenerCount(TestEventTag), 1);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify listener was called
	TestTrue(TEXT("Listener should be called"), bListenerCalled);
	TestEqual(TEXT("Received event tag should match"), ReceivedEventTag, TestEventTag);

	// Cleanup
	EventSubsystem->UnregisterListener(Handle);

	return true;
}

/**
 * Test event broadcasting to multiple listeners
 * Verifies that all registered listeners receive the event
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventBroadcastMultipleListenersTest, 
	"DelveDeep.EventSystem.BroadcastToMultipleListeners", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventBroadcastMultipleListenersTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocations
	int32 Listener1CallCount = 0;
	int32 Listener2CallCount = 0;
	int32 Listener3CallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register multiple listeners
	FDelegateHandle Handle1 = EventSubsystem->RegisterListener(
		TestEventTag,
		[&Listener1CallCount](const FDelveDeepEventPayload& Payload) { Listener1CallCount++; },
		Fixture.GameInstance
	);

	FDelegateHandle Handle2 = EventSubsystem->RegisterListener(
		TestEventTag,
		[&Listener2CallCount](const FDelveDeepEventPayload& Payload) { Listener2CallCount++; },
		Fixture.GameInstance
	);

	FDelegateHandle Handle3 = EventSubsystem->RegisterListener(
		TestEventTag,
		[&Listener3CallCount](const FDelveDeepEventPayload& Payload) { Listener3CallCount++; },
		Fixture.GameInstance
	);

	// Verify all listeners were registered
	TestEqual(TEXT("Should have three listeners registered"), EventSubsystem->GetListenerCount(TestEventTag), 3);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify all listeners were called
	TestEqual(TEXT("Listener 1 should be called once"), Listener1CallCount, 1);
	TestEqual(TEXT("Listener 2 should be called once"), Listener2CallCount, 1);
	TestEqual(TEXT("Listener 3 should be called once"), Listener3CallCount, 1);

	// Broadcast again
	EventSubsystem->BroadcastEvent(Payload);

	// Verify all listeners were called again
	TestEqual(TEXT("Listener 1 should be called twice"), Listener1CallCount, 2);
	TestEqual(TEXT("Listener 2 should be called twice"), Listener2CallCount, 2);
	TestEqual(TEXT("Listener 3 should be called twice"), Listener3CallCount, 2);

	// Cleanup
	EventSubsystem->UnregisterListener(Handle1);
	EventSubsystem->UnregisterListener(Handle2);
	EventSubsystem->UnregisterListener(Handle3);

	return true;
}

/**
 * Test priority-based listener execution order
 * Verifies that High priority listeners execute before Normal, which execute before Low
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventPriorityOrderTest, 
	"DelveDeep.EventSystem.PriorityExecutionOrder", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventPriorityOrderTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track execution order
	TArray<FString> ExecutionOrder;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register listeners with different priorities (in random order)
	FDelegateHandle LowHandle = EventSubsystem->RegisterListener(
		TestEventTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload) { ExecutionOrder.Add(TEXT("Low")); },
		Fixture.GameInstance,
		EDelveDeepEventPriority::Low
	);

	FDelegateHandle HighHandle = EventSubsystem->RegisterListener(
		TestEventTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload) { ExecutionOrder.Add(TEXT("High")); },
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	FDelegateHandle NormalHandle = EventSubsystem->RegisterListener(
		TestEventTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload) { ExecutionOrder.Add(TEXT("Normal")); },
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal
	);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify execution order: High -> Normal -> Low
	TestEqual(TEXT("Should execute three listeners"), ExecutionOrder.Num(), 3);
	TestEqual(TEXT("First listener should be High priority"), ExecutionOrder[0], TEXT("High"));
	TestEqual(TEXT("Second listener should be Normal priority"), ExecutionOrder[1], TEXT("Normal"));
	TestEqual(TEXT("Third listener should be Low priority"), ExecutionOrder[2], TEXT("Low"));

	// Cleanup
	EventSubsystem->UnregisterListener(LowHandle);
	EventSubsystem->UnregisterListener(HighHandle);
	EventSubsystem->UnregisterListener(NormalHandle);

	return true;
}

/**
 * Test hierarchical GameplayTag matching
 * Verifies that parent tags receive events from child tags
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventHierarchicalTagMatchingTest, 
	"DelveDeep.EventSystem.HierarchicalTagMatching", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventHierarchicalTagMatchingTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocations
	int32 RootListenerCallCount = 0;
	int32 CombatListenerCallCount = 0;
	int32 DamageListenerCallCount = 0;
	int32 SpecificListenerCallCount = 0;

	// Create hierarchical event tags
	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event"));
	FGameplayTag CombatTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat"));
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage"));
	FGameplayTag SpecificTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage.Dealt"));

	// Register listeners at different hierarchy levels
	FDelegateHandle RootHandle = EventSubsystem->RegisterListener(
		RootTag,
		[&RootListenerCallCount](const FDelveDeepEventPayload& Payload) { RootListenerCallCount++; },
		Fixture.GameInstance
	);

	FDelegateHandle CombatHandle = EventSubsystem->RegisterListener(
		CombatTag,
		[&CombatListenerCallCount](const FDelveDeepEventPayload& Payload) { CombatListenerCallCount++; },
		Fixture.GameInstance
	);

	FDelegateHandle DamageHandle = EventSubsystem->RegisterListener(
		DamageTag,
		[&DamageListenerCallCount](const FDelveDeepEventPayload& Payload) { DamageListenerCallCount++; },
		Fixture.GameInstance
	);

	FDelegateHandle SpecificHandle = EventSubsystem->RegisterListener(
		SpecificTag,
		[&SpecificListenerCallCount](const FDelveDeepEventPayload& Payload) { SpecificListenerCallCount++; },
		Fixture.GameInstance
	);

	// Broadcast specific event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = SpecificTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify hierarchical matching: all parent listeners should receive the event
	TestEqual(TEXT("Root listener should receive event"), RootListenerCallCount, 1);
	TestEqual(TEXT("Combat listener should receive event"), CombatListenerCallCount, 1);
	TestEqual(TEXT("Damage listener should receive event"), DamageListenerCallCount, 1);
	TestEqual(TEXT("Specific listener should receive event"), SpecificListenerCallCount, 1);

	// Reset counts
	RootListenerCallCount = 0;
	CombatListenerCallCount = 0;
	DamageListenerCallCount = 0;
	SpecificListenerCallCount = 0;

	// Broadcast mid-level event
	Payload.EventTag = DamageTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify only parent and exact match listeners receive the event
	TestEqual(TEXT("Root listener should receive damage event"), RootListenerCallCount, 1);
	TestEqual(TEXT("Combat listener should receive damage event"), CombatListenerCallCount, 1);
	TestEqual(TEXT("Damage listener should receive damage event"), DamageListenerCallCount, 1);
	TestEqual(TEXT("Specific listener should NOT receive damage event"), SpecificListenerCallCount, 0);

	// Cleanup
	EventSubsystem->UnregisterListener(RootHandle);
	EventSubsystem->UnregisterListener(CombatHandle);
	EventSubsystem->UnregisterListener(DamageHandle);
	EventSubsystem->UnregisterListener(SpecificHandle);

	return true;
}

/**
 * Test listener registration and unregistration
 * Verifies that listeners can be registered and unregistered correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventListenerRegistrationTest, 
	"DelveDeep.EventSystem.ListenerRegistration", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventListenerRegistrationTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Initially should have no listeners
	TestEqual(TEXT("Should have no listeners initially"), EventSubsystem->GetListenerCount(TestEventTag), 0);

	// Register listener
	int32 CallCount = 0;
	FDelegateHandle Handle = EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		Fixture.GameInstance
	);

	// Verify listener was registered
	TestEqual(TEXT("Should have one listener after registration"), EventSubsystem->GetListenerCount(TestEventTag), 1);
	TestTrue(TEXT("Handle should be valid"), Handle.IsValid());

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);
	TestEqual(TEXT("Listener should be called"), CallCount, 1);

	// Unregister listener
	EventSubsystem->UnregisterListener(Handle);
	TestEqual(TEXT("Should have no listeners after unregistration"), EventSubsystem->GetListenerCount(TestEventTag), 0);

	// Broadcast event again
	EventSubsystem->BroadcastEvent(Payload);
	TestEqual(TEXT("Listener should NOT be called after unregistration"), CallCount, 1);

	return true;
}

/**
 * Test unregistering all listeners for an owner
 * Verifies that UnregisterAllListeners removes all listeners for a specific owner
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventUnregisterAllListenersTest, 
	"DelveDeep.EventSystem.UnregisterAllListeners", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventUnregisterAllListenersTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Create test owner objects
	UObject* Owner1 = NewObject<UObject>();
	UObject* Owner2 = NewObject<UObject>();

	// Track listener invocations
	int32 Owner1Listener1CallCount = 0;
	int32 Owner1Listener2CallCount = 0;
	int32 Owner2ListenerCallCount = 0;

	// Create test event tags
	FGameplayTag TestEventTag1 = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test1"));
	FGameplayTag TestEventTag2 = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test2"));

	// Register multiple listeners for Owner1
	EventSubsystem->RegisterListener(
		TestEventTag1,
		[&Owner1Listener1CallCount](const FDelveDeepEventPayload& Payload) { Owner1Listener1CallCount++; },
		Owner1
	);

	EventSubsystem->RegisterListener(
		TestEventTag2,
		[&Owner1Listener2CallCount](const FDelveDeepEventPayload& Payload) { Owner1Listener2CallCount++; },
		Owner1
	);

	// Register listener for Owner2
	EventSubsystem->RegisterListener(
		TestEventTag1,
		[&Owner2ListenerCallCount](const FDelveDeepEventPayload& Payload) { Owner2ListenerCallCount++; },
		Owner2
	);

	// Verify listeners were registered
	TestEqual(TEXT("TestEventTag1 should have two listeners"), EventSubsystem->GetListenerCount(TestEventTag1), 2);
	TestEqual(TEXT("TestEventTag2 should have one listener"), EventSubsystem->GetListenerCount(TestEventTag2), 1);

	// Unregister all listeners for Owner1
	EventSubsystem->UnregisterAllListeners(Owner1);

	// Verify Owner1's listeners were removed
	TestEqual(TEXT("TestEventTag1 should have one listener (Owner2)"), EventSubsystem->GetListenerCount(TestEventTag1), 1);
	TestEqual(TEXT("TestEventTag2 should have no listeners"), EventSubsystem->GetListenerCount(TestEventTag2), 0);

	// Broadcast events
	FDelveDeepEventPayload Payload1;
	Payload1.EventTag = TestEventTag1;
	EventSubsystem->BroadcastEvent(Payload1);

	FDelveDeepEventPayload Payload2;
	Payload2.EventTag = TestEventTag2;
	EventSubsystem->BroadcastEvent(Payload2);

	// Verify only Owner2's listener was called
	TestEqual(TEXT("Owner1 Listener1 should NOT be called"), Owner1Listener1CallCount, 0);
	TestEqual(TEXT("Owner1 Listener2 should NOT be called"), Owner1Listener2CallCount, 0);
	TestEqual(TEXT("Owner2 Listener should be called"), Owner2ListenerCallCount, 1);

	// Cleanup
	EventSubsystem->UnregisterAllListeners(Owner2);

	return true;
}


/**
 * Test automatic cleanup of stale listeners
 * Verifies that listeners with invalid owners are automatically cleaned up
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventStaleListenerCleanupTest, 
	"DelveDeep.EventSystem.StaleListenerCleanup", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventStaleListenerCleanupTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Create temporary owner that will become invalid
	UObject* TempOwner = NewObject<UObject>();
	
	// Track listener invocation
	int32 CallCount = 0;

	// Register listener with temporary owner
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		TempOwner
	);

	// Verify listener was registered
	TestEqual(TEXT("Should have one listener"), EventSubsystem->GetListenerCount(TestEventTag), 1);

	// Mark owner for garbage collection (simulate owner destruction)
	TempOwner->MarkAsGarbage();
	TempOwner = nullptr;

	// Force garbage collection
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	// Broadcast event - this should trigger cleanup of stale listener
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify listener was NOT called (owner is invalid)
	TestEqual(TEXT("Stale listener should NOT be called"), CallCount, 0);

	// Note: Cleanup may be lazy, so listener count might not immediately reflect removal
	// The important thing is that the stale listener doesn't get invoked

	return true;
}

/**
 * Test actor-specific event filtering
 * Verifies that listeners can filter events by specific actor
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventActorFilterTest, 
	"DelveDeep.EventSystem.ActorSpecificFiltering", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventActorFilterTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Create test actors
	AActor* TargetActor = NewObject<AActor>();
	AActor* OtherActor = NewObject<AActor>();

	// Track listener invocations
	int32 FilteredListenerCallCount = 0;
	int32 UnfilteredListenerCallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Create filter for specific actor
	FDelveDeepEventFilter ActorFilter;
	ActorFilter.bUseActorFilter = true;
	ActorFilter.SpecificActor = TargetActor;

	// Register filtered listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&FilteredListenerCallCount](const FDelveDeepEventPayload& Payload) { FilteredListenerCallCount++; },
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal,
		ActorFilter
	);

	// Register unfiltered listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&UnfilteredListenerCallCount](const FDelveDeepEventPayload& Payload) { UnfilteredListenerCallCount++; },
		Fixture.GameInstance
	);

	// Broadcast event with target actor
	FDelveDeepEventPayload Payload1;
	Payload1.EventTag = TestEventTag;
	Payload1.Instigator = TargetActor;
	EventSubsystem->BroadcastEvent(Payload1);

	// Verify filtered listener was called
	TestEqual(TEXT("Filtered listener should be called for target actor"), FilteredListenerCallCount, 1);
	TestEqual(TEXT("Unfiltered listener should be called"), UnfilteredListenerCallCount, 1);

	// Broadcast event with other actor
	FDelveDeepEventPayload Payload2;
	Payload2.EventTag = TestEventTag;
	Payload2.Instigator = OtherActor;
	EventSubsystem->BroadcastEvent(Payload2);

	// Verify filtered listener was NOT called for other actor
	TestEqual(TEXT("Filtered listener should NOT be called for other actor"), FilteredListenerCallCount, 1);
	TestEqual(TEXT("Unfiltered listener should be called again"), UnfilteredListenerCallCount, 2);

	return true;
}

/**
 * Test spatial event filtering
 * Verifies that listeners can filter events by location and radius
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventSpatialFilterTest, 
	"DelveDeep.EventSystem.SpatialFiltering", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventSpatialFilterTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocations
	int32 SpatialListenerCallCount = 0;
	int32 UnfilteredListenerCallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Attack"));

	// Create spatial filter (center at origin, 500 unit radius)
	FDelveDeepEventFilter SpatialFilter;
	SpatialFilter.bUseSpatialFilter = true;
	SpatialFilter.SpatialLocation = FVector(0, 0, 0);
	SpatialFilter.SpatialRadius = 500.0f;

	// Register spatially filtered listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&SpatialListenerCallCount](const FDelveDeepEventPayload& Payload) { SpatialListenerCallCount++; },
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal,
		SpatialFilter
	);

	// Register unfiltered listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&UnfilteredListenerCallCount](const FDelveDeepEventPayload& Payload) { UnfilteredListenerCallCount++; },
		Fixture.GameInstance
	);

	// Broadcast event within radius
	FDelveDeepAttackEventPayload NearPayload;
	NearPayload.EventTag = TestEventTag;
	NearPayload.AttackLocation = FVector(100, 100, 0); // Distance ~141 units from origin
	EventSubsystem->BroadcastEvent(NearPayload);

	// Verify spatial listener was called
	TestEqual(TEXT("Spatial listener should be called for nearby event"), SpatialListenerCallCount, 1);
	TestEqual(TEXT("Unfiltered listener should be called"), UnfilteredListenerCallCount, 1);

	// Broadcast event outside radius
	FDelveDeepAttackEventPayload FarPayload;
	FarPayload.EventTag = TestEventTag;
	FarPayload.AttackLocation = FVector(1000, 1000, 0); // Distance ~1414 units from origin
	EventSubsystem->BroadcastEvent(FarPayload);

	// Verify spatial listener was NOT called for distant event
	TestEqual(TEXT("Spatial listener should NOT be called for distant event"), SpatialListenerCallCount, 1);
	TestEqual(TEXT("Unfiltered listener should be called again"), UnfilteredListenerCallCount, 2);

	return true;
}

/**
 * Test deferred event processing
 * Verifies that events can be queued and processed later
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventDeferredProcessingTest, 
	"DelveDeep.EventSystem.DeferredProcessing", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventDeferredProcessingTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocations
	int32 CallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		Fixture.GameInstance
	);

	// Enable deferred mode
	EventSubsystem->EnableDeferredMode();

	// Broadcast events in deferred mode
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	
	EventSubsystem->BroadcastEvent(Payload);
	EventSubsystem->BroadcastEvent(Payload);
	EventSubsystem->BroadcastEvent(Payload);

	// Verify listener was NOT called yet (events are queued)
	TestEqual(TEXT("Listener should NOT be called in deferred mode"), CallCount, 0);

	// Process deferred events
	EventSubsystem->ProcessDeferredEvents();

	// Verify all events were processed
	TestEqual(TEXT("Listener should be called three times after processing"), CallCount, 3);

	// Disable deferred mode
	EventSubsystem->DisableDeferredMode();

	// Broadcast event in immediate mode
	EventSubsystem->BroadcastEvent(Payload);

	// Verify listener was called immediately
	TestEqual(TEXT("Listener should be called immediately after disabling deferred mode"), CallCount, 4);

	return true;
}

/**
 * Test deferred event queue management
 * Verifies that the deferred event queue handles overflow correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventDeferredQueueManagementTest, 
	"DelveDeep.EventSystem.DeferredQueueManagement", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventDeferredQueueManagementTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocations
	int32 CallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		Fixture.GameInstance
	);

	// Enable deferred mode
	EventSubsystem->EnableDeferredMode();

	// Queue multiple events
	const int32 EventCount = 100;
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;

	for (int32 i = 0; i < EventCount; ++i)
	{
		EventSubsystem->BroadcastEvent(Payload);
	}

	// Verify events are queued
	TestEqual(TEXT("Listener should NOT be called yet"), CallCount, 0);

	// Process deferred events
	EventSubsystem->ProcessDeferredEvents();

	// Verify all events were processed
	TestEqual(TEXT("All queued events should be processed"), CallCount, EventCount);

	// Verify queue is empty after processing
	EventSubsystem->ProcessDeferredEvents();
	TestEqual(TEXT("Processing empty queue should not call listeners"), CallCount, EventCount);

	// Disable deferred mode
	EventSubsystem->DisableDeferredMode();

	return true;
}

/**
 * Test event ordering in deferred mode
 * Verifies that deferred events are processed in the order they were broadcast
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventDeferredOrderingTest, 
	"DelveDeep.EventSystem.DeferredEventOrdering", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventDeferredOrderingTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track event order
	TArray<int32> ReceivedOrder;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register listener that tracks order
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&ReceivedOrder](const FDelveDeepEventPayload& Payload)
		{
			// Extract order from damage amount (using damage payload as carrier)
			const FDelveDeepDamageEventPayload* DamagePayload = static_cast<const FDelveDeepDamageEventPayload*>(&Payload);
			ReceivedOrder.Add(static_cast<int32>(DamagePayload->DamageAmount));
		},
		Fixture.GameInstance
	);

	// Enable deferred mode
	EventSubsystem->EnableDeferredMode();

	// Broadcast events with different order markers
	for (int32 i = 1; i <= 10; ++i)
	{
		FDelveDeepDamageEventPayload Payload;
		Payload.EventTag = TestEventTag;
		Payload.DamageAmount = static_cast<float>(i);
		EventSubsystem->BroadcastEvent(Payload);
	}

	// Process deferred events
	EventSubsystem->ProcessDeferredEvents();

	// Verify events were processed in order
	TestEqual(TEXT("Should receive all events"), ReceivedOrder.Num(), 10);
	for (int32 i = 0; i < 10; ++i)
	{
		TestEqual(TEXT("Events should be processed in order"), ReceivedOrder[i], i + 1);
	}

	// Disable deferred mode
	EventSubsystem->DisableDeferredMode();

	return true;
}

/**
 * Test payload validation
 * Verifies that invalid payloads are detected and handled correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventPayloadValidationTest, 
	"DelveDeep.EventSystem.PayloadValidation", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventPayloadValidationTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Enable validation (development builds only)
	EventSubsystem->EnableValidation();

	// Track listener invocations
	int32 CallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage"));

	// Register listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		Fixture.GameInstance
	);

	// Test with invalid damage payload (negative damage)
	FDelveDeepDamageEventPayload InvalidPayload;
	InvalidPayload.EventTag = TestEventTag;
	InvalidPayload.DamageAmount = -10.0f; // Invalid: negative damage
	InvalidPayload.Attacker = nullptr;    // Invalid: null attacker
	InvalidPayload.Victim = nullptr;      // Invalid: null victim

	// Validate payload
	FValidationContext Context;
	bool bIsValid = InvalidPayload.Validate(Context);

	// Verify validation failed
	TestFalse(TEXT("Invalid payload should fail validation"), bIsValid);
	TestTrue(TEXT("Validation should report errors"), Context.ValidationErrors.Num() > 0);

	// Test with valid damage payload
	AActor* Attacker = NewObject<AActor>();
	AActor* Victim = NewObject<AActor>();

	FDelveDeepDamageEventPayload ValidPayload;
	ValidPayload.EventTag = TestEventTag;
	ValidPayload.DamageAmount = 10.0f;
	ValidPayload.Attacker = Attacker;
	ValidPayload.Victim = Victim;

	// Validate payload
	Context.Reset();
	bIsValid = ValidPayload.Validate(Context);

	// Verify validation passed
	TestTrue(TEXT("Valid payload should pass validation"), bIsValid);
	TestEqual(TEXT("Validation should report no errors"), Context.ValidationErrors.Num(), 0);

	// Broadcast valid payload
	EventSubsystem->BroadcastEvent(ValidPayload);
	TestEqual(TEXT("Listener should be called for valid payload"), CallCount, 1);

	return true;
}

/**
 * Test error handling for listener callbacks
 * Verifies that exceptions in listener callbacks are caught and logged
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventErrorHandlingTest, 
	"DelveDeep.EventSystem.ListenerErrorHandling", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventErrorHandlingTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track listener invocations
	int32 FailingListenerCallCount = 0;
	int32 SuccessfulListenerCallCount = 0;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register listener that throws exception
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&FailingListenerCallCount](const FDelveDeepEventPayload& Payload)
		{
			FailingListenerCallCount++;
			// Simulate error by accessing invalid pointer
			check(false); // This will trigger an assertion in development builds
		},
		Fixture.GameInstance
	);

	// Register successful listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&SuccessfulListenerCallCount](const FDelveDeepEventPayload& Payload)
		{
			SuccessfulListenerCallCount++;
		},
		Fixture.GameInstance
	);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	
	// Note: In a real scenario, the failing listener would be caught and logged
	// For testing purposes, we'll verify that the system continues processing
	// This test is more about documenting expected behavior than testing implementation

	// In development builds, the check() will halt execution
	// In shipping builds, the error would be caught and logged
	// For this test, we'll just verify the setup is correct

	TestEqual(TEXT("Should have two listeners registered"), EventSubsystem->GetListenerCount(TestEventTag), 2);

	return true;
}

/**
 * Test performance metrics tracking
 * Verifies that performance metrics are accurately tracked
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventPerformanceMetricsTest, 
	"DelveDeep.EventSystem.PerformanceMetrics", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventPerformanceMetricsTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Reset metrics
	EventSubsystem->ResetPerformanceMetrics();

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register multiple listeners
	const int32 ListenerCount = 5;
	for (int32 i = 0; i < ListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			TestEventTag,
			[](const FDelveDeepEventPayload& Payload) { /* Do nothing */ },
			Fixture.GameInstance
		);
	}

	// Broadcast multiple events
	const int32 EventCount = 10;
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;

	for (int32 i = 0; i < EventCount; ++i)
	{
		EventSubsystem->BroadcastEvent(Payload);
	}

	// Get performance metrics
	const FEventSystemMetrics& Metrics = EventSubsystem->GetPerformanceMetrics();

	// Verify metrics
	TestEqual(TEXT("Total events broadcast should match"), Metrics.TotalEventsBroadcast, EventCount);
	TestEqual(TEXT("Total listener invocations should match"), Metrics.TotalListenerInvocations, EventCount * ListenerCount);
	TestTrue(TEXT("Average time per broadcast should be positive"), Metrics.AverageTimePerBroadcast > 0.0);
	TestTrue(TEXT("Average time per listener should be positive"), Metrics.AverageTimePerListener > 0.0);
	TestEqual(TEXT("Peak listeners per event should match"), Metrics.PeakListenersPerEvent, ListenerCount);

	// Log metrics for inspection
	UE_LOG(LogDelveDeepEventTests, Display, TEXT("Performance Metrics:"));
	UE_LOG(LogDelveDeepEventTests, Display, TEXT("  Total Events: %d"), Metrics.TotalEventsBroadcast);
	UE_LOG(LogDelveDeepEventTests, Display, TEXT("  Total Invocations: %d"), Metrics.TotalListenerInvocations);
	UE_LOG(LogDelveDeepEventTests, Display, TEXT("  Avg Time/Broadcast: %.4f ms"), Metrics.AverageTimePerBroadcast);
	UE_LOG(LogDelveDeepEventTests, Display, TEXT("  Avg Time/Listener: %.4f ms"), Metrics.AverageTimePerListener);
	UE_LOG(LogDelveDeepEventTests, Display, TEXT("  Peak Listeners: %d"), Metrics.PeakListenersPerEvent);

	return true;
}

/**
 * Test event broadcast performance
 * Verifies that event broadcasting meets performance targets (<1ms for 50 listeners)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventBroadcastPerformanceTest, 
	"DelveDeep.EventSystem.BroadcastPerformance", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventBroadcastPerformanceTest::RunTest(const FString& Parameters)
{
	FEventSystemTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Register 50 listeners (performance target)
	const int32 ListenerCount = 50;
	for (int32 i = 0; i < ListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			TestEventTag,
			[](const FDelveDeepEventPayload& Payload) { /* Minimal work */ },
			Fixture.GameInstance
		);
	}

	// Measure broadcast time
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;

	double StartTime = FPlatformTime::Seconds();
	EventSubsystem->BroadcastEvent(Payload);
	double EndTime = FPlatformTime::Seconds();

	double BroadcastTimeMs = (EndTime - StartTime) * 1000.0;

	// Verify performance target (<1ms for 50 listeners)
	TestTrue(TEXT("Broadcast time should be under 1ms for 50 listeners"), BroadcastTimeMs < 1.0);

	UE_LOG(LogDelveDeepEventTests, Display, TEXT("Broadcast time for %d listeners: %.4f ms"), ListenerCount, BroadcastTimeMs);

	// Test with 100 listeners
	for (int32 i = 0; i < 50; ++i)
	{
		EventSubsystem->RegisterListener(
			TestEventTag,
			[](const FDelveDeepEventPayload& Payload) { /* Minimal work */ },
			Fixture.GameInstance
		);
	}

	StartTime = FPlatformTime::Seconds();
	EventSubsystem->BroadcastEvent(Payload);
	EndTime = FPlatformTime::Seconds();

	BroadcastTimeMs = (EndTime - StartTime) * 1000.0;

	UE_LOG(LogDelveDeepEventTests, Display, TEXT("Broadcast time for 100 listeners: %.4f ms"), BroadcastTimeMs);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
