// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventTypes.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"
#include "DelveDeepTestMacros.h"
#include "DelveDeepTestFixtures.h"
#include "DelveDeepAsyncTestCommands.h"

#if WITH_DEV_AUTOMATION_TESTS

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepEventTests, Log, All);
DEFINE_LOG_CATEGORY(LogDelveDeepEventTests);

/**
 * Test: Event broadcasting to a single listener
 * Verifies that events are delivered to registered listeners
 * Requirements: 1.1, 12.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventBroadcastSingleListenerTest, 
	"DelveDeep.Events.BroadcastToSingleListener", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventBroadcastSingleListenerTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

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
	EXPECT_EQ(EventSubsystem->GetListenerCount(TestEventTag), 1);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify listener was called
	EXPECT_TRUE(bListenerCalled);
	EXPECT_EQ(ReceivedEventTag, TestEventTag);

	// Cleanup
	EventSubsystem->UnregisterListener(Handle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test: Event broadcasting to multiple listeners
 * Verifies that all registered listeners receive the event
 * Requirements: 1.1, 12.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventBroadcastMultipleListenersTest, 
	"DelveDeep.Events.BroadcastToMultipleListeners", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventBroadcastMultipleListenersTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

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
	EXPECT_EQ(EventSubsystem->GetListenerCount(TestEventTag), 3);

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify all listeners were called
	EXPECT_EQ(Listener1CallCount, 1);
	EXPECT_EQ(Listener2CallCount, 1);
	EXPECT_EQ(Listener3CallCount, 1);

	// Broadcast again
	EventSubsystem->BroadcastEvent(Payload);

	// Verify all listeners were called again
	EXPECT_EQ(Listener1CallCount, 2);
	EXPECT_EQ(Listener2CallCount, 2);
	EXPECT_EQ(Listener3CallCount, 2);

	// Cleanup
	EventSubsystem->UnregisterListener(Handle1);
	EventSubsystem->UnregisterListener(Handle2);
	EventSubsystem->UnregisterListener(Handle3);
	Fixture.AfterEach();

	return true;
}

/**
 * Test: Priority-based listener execution order
 * Verifies that High priority listeners execute before Normal, which execute before Low
 * Requirements: 1.1, 12.3
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventPriorityOrderTest, 
	"DelveDeep.Events.PriorityExecutionOrder", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventPriorityOrderTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

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
	EXPECT_EQ(ExecutionOrder.Num(), 3);
	EXPECT_STR_EQ(ExecutionOrder[0], TEXT("High"));
	EXPECT_STR_EQ(ExecutionOrder[1], TEXT("Normal"));
	EXPECT_STR_EQ(ExecutionOrder[2], TEXT("Low"));

	// Cleanup
	EventSubsystem->UnregisterListener(LowHandle);
	EventSubsystem->UnregisterListener(HighHandle);
	EventSubsystem->UnregisterListener(NormalHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test: Hierarchical GameplayTag matching
 * Verifies that parent tags receive events from child tags
 * Requirements: 1.1, 12.2
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventHierarchicalTagMatchingTest, 
	"DelveDeep.Events.HierarchicalTagMatching", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventHierarchicalTagMatchingTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

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
	EXPECT_EQ(RootListenerCallCount, 1);
	EXPECT_EQ(CombatListenerCallCount, 1);
	EXPECT_EQ(DamageListenerCallCount, 1);
	EXPECT_EQ(SpecificListenerCallCount, 1);

	// Reset counts
	RootListenerCallCount = 0;
	CombatListenerCallCount = 0;
	DamageListenerCallCount = 0;
	SpecificListenerCallCount = 0;

	// Broadcast mid-level event
	Payload.EventTag = DamageTag;
	EventSubsystem->BroadcastEvent(Payload);

	// Verify only parent and exact match listeners receive the event
	EXPECT_EQ(RootListenerCallCount, 1);
	EXPECT_EQ(CombatListenerCallCount, 1);
	EXPECT_EQ(DamageListenerCallCount, 1);
	EXPECT_EQ(SpecificListenerCallCount, 0);

	// Cleanup
	EventSubsystem->UnregisterListener(RootHandle);
	EventSubsystem->UnregisterListener(CombatHandle);
	EventSubsystem->UnregisterListener(DamageHandle);
	EventSubsystem->UnregisterListener(SpecificHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test: Listener registration and unregistration
 * Verifies that listeners can be registered and unregistered correctly
 * Requirements: 1.1, 12.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventListenerRegistrationTest, 
	"DelveDeep.Events.ListenerRegistration", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventListenerRegistrationTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Test"));

	// Initially should have no listeners
	EXPECT_EQ(EventSubsystem->GetListenerCount(TestEventTag), 0);

	// Register listener
	int32 CallCount = 0;
	FDelegateHandle Handle = EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		Fixture.GameInstance
	);

	// Verify listener was registered
	EXPECT_EQ(EventSubsystem->GetListenerCount(TestEventTag), 1);
	EXPECT_TRUE(Handle.IsValid());

	// Broadcast event
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;
	EventSubsystem->BroadcastEvent(Payload);
	EXPECT_EQ(CallCount, 1);

	// Unregister listener
	EventSubsystem->UnregisterListener(Handle);
	EXPECT_EQ(EventSubsystem->GetListenerCount(TestEventTag), 0);

	// Broadcast event again
	EventSubsystem->BroadcastEvent(Payload);
	EXPECT_EQ(CallCount, 1);

	Fixture.AfterEach();
	return true;
}

/**
 * Test: Async event processing with deferred mode
 * Verifies that events can be queued and processed asynchronously
 * Requirements: 4.1, 12.1
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDelveDeepEventAsyncDeferredProcessingTest, 
	"DelveDeep.Events.AsyncDeferredProcessing", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventAsyncDeferredProcessingTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

	// Track listener invocations
	TSharedPtr<int32> CallCount = MakeShared<int32>(0);

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Async.Test"));

	// Register listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[CallCount](const FDelveDeepEventPayload& Payload) { (*CallCount)++; },
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
	EXPECT_EQ(*CallCount, 0);

	// Wait for next frame and process deferred events
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedExecutionCommand(
		[EventSubsystem]() { EventSubsystem->ProcessDeferredEvents(); },
		0.1f,
		FPlatformTime::Seconds()
	));

	// Wait for processing to complete
	ADD_WAIT_FOR_CONDITION([CallCount]() { return *CallCount == 3; }, 5.0f);

	// Verify all events were processed
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedExecutionCommand(
		[this, CallCount, EventSubsystem, &Fixture]()
		{
			EXPECT_EQ(*CallCount, 3);
			
			// Disable deferred mode
			EventSubsystem->DisableDeferredMode();
			Fixture.AfterEach();
		},
		0.1f,
		FPlatformTime::Seconds()
	));

	return true;
}

/**
 * Test: Async event with timer-based callback
 * Verifies that timer-based events work correctly with async testing
 * Requirements: 4.1, 4.2, 12.1
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FDelveDeepEventAsyncTimerTest, 
	"DelveDeep.Events.AsyncTimerCallback", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventAsyncTimerTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

	// Track callback invocation
	TSharedPtr<bool> bCallbackInvoked = MakeShared<bool>(false);

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Async.Timer"));

	// Register listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[bCallbackInvoked](const FDelveDeepEventPayload& Payload) { *bCallbackInvoked = true; },
		Fixture.GameInstance
	);

	// Schedule delayed event broadcast
	ADD_DELAYED_EXECUTION(
		[EventSubsystem, TestEventTag]()
		{
			FDelveDeepEventPayload Payload;
			Payload.EventTag = TestEventTag;
			EventSubsystem->BroadcastEvent(Payload);
		},
		0.5f
	);

	// Wait for callback to be invoked
	ADD_VERIFY_CALLBACK(bCallbackInvoked);

	// Verify callback was invoked
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedExecutionCommand(
		[this, bCallbackInvoked, &Fixture]()
		{
			EXPECT_TRUE(*bCallbackInvoked);
			Fixture.AfterEach();
		},
		0.1f,
		FPlatformTime::Seconds()
	));

	return true;
}

/**
 * Performance Test: Event broadcast performance with 50 listeners
 * Verifies that event broadcasting meets performance targets (<1ms for 50 listeners)
 * Requirements: 12.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventBroadcastPerformanceTest, 
	"DelveDeep.Events.Performance.BroadcastTime", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::PerfFilter)

bool FDelveDeepEventBroadcastPerformanceTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Performance.Test"));

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
	EXPECT_LT(BroadcastTimeMs, 1.0);

	UE_LOG(LogDelveDeepEventTests, Display, TEXT("Broadcast time for %d listeners: %.4f ms"), 
		ListenerCount, BroadcastTimeMs);

	Fixture.AfterEach();
	return true;
}

/**
 * Performance Test: Deferred event processing performance
 * Verifies that deferred processing meets performance targets (<10ms for 1000 events)
 * Requirements: 12.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventDeferredPerformanceTest, 
	"DelveDeep.Events.Performance.DeferredProcessing", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::PerfFilter)

bool FDelveDeepEventDeferredPerformanceTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Performance.Deferred"));

	// Register listener
	int32 CallCount = 0;
	EventSubsystem->RegisterListener(
		TestEventTag,
		[&CallCount](const FDelveDeepEventPayload& Payload) { CallCount++; },
		Fixture.GameInstance
	);

	// Enable deferred mode
	EventSubsystem->EnableDeferredMode();

	// Queue 1000 events
	const int32 EventCount = 1000;
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;

	for (int32 i = 0; i < EventCount; ++i)
	{
		EventSubsystem->BroadcastEvent(Payload);
	}

	// Measure processing time
	double StartTime = FPlatformTime::Seconds();
	EventSubsystem->ProcessDeferredEvents();
	double EndTime = FPlatformTime::Seconds();

	double ProcessingTimeMs = (EndTime - StartTime) * 1000.0;

	// Verify all events were processed
	EXPECT_EQ(CallCount, EventCount);

	// Verify performance target (<10ms for 1000 events)
	EXPECT_LT(ProcessingTimeMs, 10.0);

	UE_LOG(LogDelveDeepEventTests, Display, TEXT("Deferred processing time for %d events: %.4f ms"), 
		EventCount, ProcessingTimeMs);

	// Disable deferred mode
	EventSubsystem->DisableDeferredMode();
	Fixture.AfterEach();

	return true;
}

/**
 * Performance Test: Listener invocation overhead
 * Measures the overhead per listener invocation
 * Requirements: 12.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventListenerOverheadTest, 
	"DelveDeep.Events.Performance.ListenerOverhead", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::PerfFilter)

bool FDelveDeepEventListenerOverheadTest::RunTest(const FString& Parameters)
{
	// Setup fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();
	
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.GetSubsystem<UDelveDeepEventSubsystem>();
	ASSERT_NOT_NULL(EventSubsystem);

	// Create test event tag
	FGameplayTag TestEventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Performance.Overhead"));

	// Register single listener
	EventSubsystem->RegisterListener(
		TestEventTag,
		[](const FDelveDeepEventPayload& Payload) { /* Minimal work */ },
		Fixture.GameInstance
	);

	// Measure time for 1000 broadcasts
	const int32 BroadcastCount = 1000;
	FDelveDeepEventPayload Payload;
	Payload.EventTag = TestEventTag;

	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < BroadcastCount; ++i)
	{
		EventSubsystem->BroadcastEvent(Payload);
	}
	double EndTime = FPlatformTime::Seconds();

	double TotalTimeMs = (EndTime - StartTime) * 1000.0;
	double AvgTimePerInvocation = TotalTimeMs / BroadcastCount;

	// Verify overhead is minimal (<0.1ms per listener invocation)
	EXPECT_LT(AvgTimePerInvocation, 0.1);

	UE_LOG(LogDelveDeepEventTests, Display, TEXT("Average listener invocation time: %.4f ms"), 
		AvgTimePerInvocation);

	Fixture.AfterEach();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
