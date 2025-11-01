// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventTypes.h"
#include "GameplayTagsManager.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepEventIntegrationTests, Log, All);
DEFINE_LOG_CATEGORY(LogDelveDeepEventIntegrationTests);

/**
 * Test fixture for event system integration tests
 * Simulates multiple game systems communicating via events
 */
class FEventSystemIntegrationTestFixture
{
public:
	FEventSystemIntegrationTestFixture()
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

	~FEventSystemIntegrationTestFixture()
	{
		// Cleanup
		EventSubsystem = nullptr;
		GameInstance = nullptr;
	}

	// Simulated game systems
	struct FCombatSystem
	{
		int32 DamageEventsReceived = 0;
		int32 AttackEventsReceived = 0;
		int32 KillEventsReceived = 0;
		float TotalDamageDealt = 0.0f;
		int32 TotalKills = 0;
	};

	struct FUISystem
	{
		int32 HealthChangeEventsReceived = 0;
		int32 DamageEventsReceived = 0;
		float LastHealthValue = 0.0f;
		float LastDamageAmount = 0.0f;
	};

	struct FProgressionSystem
	{
		int32 KillEventsReceived = 0;
		int32 ExperienceGained = 0;
		int32 LevelUps = 0;
	};

	struct FAISystem
	{
		int32 AttackEventsReceived = 0;
		int32 DamageEventsReceived = 0;
		TArray<FVector> AttackLocations;
	};

	UGameInstance* GameInstance;
	UDelveDeepEventSubsystem* EventSubsystem;
	FCombatSystem CombatSystem;
	FUISystem UISystem;
	FProgressionSystem ProgressionSystem;
	FAISystem AISystem;
};

/**
 * Integration Test: Combat System to UI and Progression Systems
 * Tests event flow from combat system broadcasting damage/kill events
 * to UI system updating health bars and progression system awarding experience
 * Requirements: 1.1, 2.1, 3.1, 4.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventCombatToUIProgressionIntegrationTest, 
	"DelveDeep.EventSystem.Integration.CombatToUIProgression", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventCombatToUIProgressionIntegrationTest::RunTest(const FString& Parameters)
{
	FEventSystemIntegrationTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Setup event tags
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage.Dealt"));
	FGameplayTag HealthChangeTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Character.Health.Changed"));
	FGameplayTag KillTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Kill.Enemy"));

	// Register UI system listener for damage events (High priority - needs to update immediately)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			const FDelveDeepDamageEventPayload* DamagePayload = static_cast<const FDelveDeepDamageEventPayload*>(&Payload);
			Fixture.UISystem.DamageEventsReceived++;
			Fixture.UISystem.LastDamageAmount = DamagePayload->DamageAmount;
			UE_LOG(LogDelveDeepEventIntegrationTests, Verbose, TEXT("UI: Received damage event (%.2f damage)"), 
				DamagePayload->DamageAmount);
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	// Register UI system listener for health change events (High priority)
	EventSubsystem->RegisterListener(
		HealthChangeTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			const FDelveDeepHealthChangeEventPayload* HealthPayload = static_cast<const FDelveDeepHealthChangeEventPayload*>(&Payload);
			Fixture.UISystem.HealthChangeEventsReceived++;
			Fixture.UISystem.LastHealthValue = HealthPayload->NewHealth;
			UE_LOG(LogDelveDeepEventIntegrationTests, Verbose, TEXT("UI: Health changed to %.2f"), 
				HealthPayload->NewHealth);
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	// Register progression system listener for kill events (Normal priority)
	EventSubsystem->RegisterListener(
		KillTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			const FDelveDeepKillEventPayload* KillPayload = static_cast<const FDelveDeepKillEventPayload*>(&Payload);
			Fixture.ProgressionSystem.KillEventsReceived++;
			Fixture.ProgressionSystem.ExperienceGained += KillPayload->ExperienceAwarded;
			
			// Simulate level up every 100 XP
			if (Fixture.ProgressionSystem.ExperienceGained >= 100)
			{
				Fixture.ProgressionSystem.LevelUps++;
			}
			
			UE_LOG(LogDelveDeepEventIntegrationTests, Verbose, TEXT("Progression: Kill event (+%d XP, total: %d)"), 
				KillPayload->ExperienceAwarded, Fixture.ProgressionSystem.ExperienceGained);
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal
	);

	// Simulate combat scenario: Player attacks enemy
	AActor* Player = NewObject<AActor>();
	AActor* Enemy = NewObject<AActor>();

	// 1. Combat system broadcasts damage event
	FDelveDeepDamageEventPayload DamagePayload;
	DamagePayload.EventTag = DamageTag;
	DamagePayload.Attacker = Player;
	DamagePayload.Victim = Enemy;
	DamagePayload.DamageAmount = 25.0f;
	EventSubsystem->BroadcastEvent(DamagePayload);

	// Verify UI received damage event
	TestEqual(TEXT("UI should receive damage event"), Fixture.UISystem.DamageEventsReceived, 1);
	TestEqual(TEXT("UI should record damage amount"), Fixture.UISystem.LastDamageAmount, 25.0f);

	// 2. Combat system broadcasts health change event for enemy
	FDelveDeepHealthChangeEventPayload HealthPayload;
	HealthPayload.EventTag = HealthChangeTag;
	HealthPayload.Character = Enemy;
	HealthPayload.PreviousHealth = 100.0f;
	HealthPayload.NewHealth = 75.0f;
	HealthPayload.MaxHealth = 100.0f;
	EventSubsystem->BroadcastEvent(HealthPayload);

	// Verify UI received health change event
	TestEqual(TEXT("UI should receive health change event"), Fixture.UISystem.HealthChangeEventsReceived, 1);
	TestEqual(TEXT("UI should record new health value"), Fixture.UISystem.LastHealthValue, 75.0f);

	// 3. Simulate multiple attacks until enemy dies
	for (int32 i = 0; i < 3; ++i)
	{
		DamagePayload.DamageAmount = 25.0f;
		EventSubsystem->BroadcastEvent(DamagePayload);
		
		HealthPayload.PreviousHealth = HealthPayload.NewHealth;
		HealthPayload.NewHealth -= 25.0f;
		EventSubsystem->BroadcastEvent(HealthPayload);
	}

	// Verify UI received all events
	TestEqual(TEXT("UI should receive 4 total damage events"), Fixture.UISystem.DamageEventsReceived, 4);
	TestEqual(TEXT("UI should receive 4 total health change events"), Fixture.UISystem.HealthChangeEventsReceived, 4);
	TestEqual(TEXT("UI should show enemy at 0 health"), Fixture.UISystem.LastHealthValue, 0.0f);

	// 4. Combat system broadcasts kill event
	FDelveDeepKillEventPayload KillPayload;
	KillPayload.EventTag = KillTag;
	KillPayload.Killer = Player;
	KillPayload.Victim = Enemy;
	KillPayload.ExperienceAwarded = 50;
	EventSubsystem->BroadcastEvent(KillPayload);

	// Verify progression system received kill event
	TestEqual(TEXT("Progression should receive kill event"), Fixture.ProgressionSystem.KillEventsReceived, 1);
	TestEqual(TEXT("Progression should award experience"), Fixture.ProgressionSystem.ExperienceGained, 50);

	// 5. Simulate killing another enemy to trigger level up
	FDelveDeepKillEventPayload SecondKillPayload;
	SecondKillPayload.EventTag = KillTag;
	SecondKillPayload.Killer = Player;
	SecondKillPayload.Victim = NewObject<AActor>();
	SecondKillPayload.ExperienceAwarded = 60;
	EventSubsystem->BroadcastEvent(SecondKillPayload);

	// Verify progression system leveled up
	TestEqual(TEXT("Progression should receive second kill event"), Fixture.ProgressionSystem.KillEventsReceived, 2);
	TestEqual(TEXT("Progression should have 110 total XP"), Fixture.ProgressionSystem.ExperienceGained, 110);
	TestEqual(TEXT("Progression should trigger level up"), Fixture.ProgressionSystem.LevelUps, 1);

	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("Combat to UI/Progression integration test complete"));
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  UI: %d damage events, %d health events"), 
		Fixture.UISystem.DamageEventsReceived, Fixture.UISystem.HealthChangeEventsReceived);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Progression: %d kills, %d XP, %d level ups"), 
		Fixture.ProgressionSystem.KillEventsReceived, Fixture.ProgressionSystem.ExperienceGained, 
		Fixture.ProgressionSystem.LevelUps);

	return true;
}

/**
 * Integration Test: Cross-System Communication with 3+ Systems
 * Tests event flow between Combat, UI, Progression, and AI systems
 * Verifies that all systems can communicate without direct dependencies
 * Requirements: 1.1, 2.1, 3.1, 4.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventCrossSystemCommunicationTest, 
	"DelveDeep.EventSystem.Integration.CrossSystemCommunication", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventCrossSystemCommunicationTest::RunTest(const FString& Parameters)
{
	FEventSystemIntegrationTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Setup event tags
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Attack.Melee"));
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage.Dealt"));
	FGameplayTag HealthChangeTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Character.Health.Changed"));
	FGameplayTag KillTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Kill.Enemy"));

	// Register Combat system listeners (tracks all combat events)
	EventSubsystem->RegisterListener(
		FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat")),
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			if (Payload.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage"))))
			{
				Fixture.CombatSystem.DamageEventsReceived++;
				const FDelveDeepDamageEventPayload* DamagePayload = static_cast<const FDelveDeepDamageEventPayload*>(&Payload);
				Fixture.CombatSystem.TotalDamageDealt += DamagePayload->DamageAmount;
			}
			else if (Payload.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Attack"))))
			{
				Fixture.CombatSystem.AttackEventsReceived++;
			}
			else if (Payload.EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Kill"))))
			{
				Fixture.CombatSystem.KillEventsReceived++;
				Fixture.CombatSystem.TotalKills++;
			}
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal
	);

	// Register UI system listeners (High priority)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			Fixture.UISystem.DamageEventsReceived++;
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	EventSubsystem->RegisterListener(
		HealthChangeTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			Fixture.UISystem.HealthChangeEventsReceived++;
			const FDelveDeepHealthChangeEventPayload* HealthPayload = static_cast<const FDelveDeepHealthChangeEventPayload*>(&Payload);
			Fixture.UISystem.LastHealthValue = HealthPayload->NewHealth;
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	// Register Progression system listeners
	EventSubsystem->RegisterListener(
		KillTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			Fixture.ProgressionSystem.KillEventsReceived++;
			const FDelveDeepKillEventPayload* KillPayload = static_cast<const FDelveDeepKillEventPayload*>(&Payload);
			Fixture.ProgressionSystem.ExperienceGained += KillPayload->ExperienceAwarded;
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal
	);

	// Register AI system listeners (spatial filtering for nearby attacks)
	FDelveDeepEventFilter SpatialFilter;
	SpatialFilter.bUseSpatialFilter = true;
	SpatialFilter.SpatialLocation = FVector(0, 0, 0);
	SpatialFilter.SpatialRadius = 1000.0f;

	EventSubsystem->RegisterListener(
		AttackTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			Fixture.AISystem.AttackEventsReceived++;
			const FDelveDeepAttackEventPayload* AttackPayload = static_cast<const FDelveDeepAttackEventPayload*>(&Payload);
			Fixture.AISystem.AttackLocations.Add(AttackPayload->AttackLocation);
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal,
		SpatialFilter
	);

	EventSubsystem->RegisterListener(
		DamageTag,
		[&Fixture](const FDelveDeepEventPayload& Payload)
		{
			Fixture.AISystem.DamageEventsReceived++;
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Low
	);

	// Simulate complex combat scenario
	AActor* Player = NewObject<AActor>();
	AActor* Enemy1 = NewObject<AActor>();
	AActor* Enemy2 = NewObject<AActor>();

	// Scenario 1: Player attacks Enemy1
	FDelveDeepAttackEventPayload AttackPayload;
	AttackPayload.EventTag = AttackTag;
	AttackPayload.Attacker = Player;
	AttackPayload.AttackLocation = FVector(100, 100, 0);
	AttackPayload.AttackRadius = 50.0f;
	EventSubsystem->BroadcastEvent(AttackPayload);

	// Verify all systems received attack event
	TestEqual(TEXT("Combat system should track attack"), Fixture.CombatSystem.AttackEventsReceived, 1);
	TestEqual(TEXT("AI system should detect nearby attack"), Fixture.AISystem.AttackEventsReceived, 1);
	TestEqual(TEXT("AI should record attack location"), Fixture.AISystem.AttackLocations.Num(), 1);

	// Scenario 2: Attack deals damage to Enemy1
	FDelveDeepDamageEventPayload DamagePayload;
	DamagePayload.EventTag = DamageTag;
	DamagePayload.Attacker = Player;
	DamagePayload.Victim = Enemy1;
	DamagePayload.DamageAmount = 30.0f;
	EventSubsystem->BroadcastEvent(DamagePayload);

	// Verify all systems received damage event
	TestEqual(TEXT("Combat system should track damage"), Fixture.CombatSystem.DamageEventsReceived, 1);
	TestEqual(TEXT("UI system should receive damage event"), Fixture.UISystem.DamageEventsReceived, 1);
	TestEqual(TEXT("AI system should detect damage"), Fixture.AISystem.DamageEventsReceived, 1);
	TestEqual(TEXT("Combat system should track total damage"), Fixture.CombatSystem.TotalDamageDealt, 30.0f);

	// Scenario 3: Enemy1 health changes
	FDelveDeepHealthChangeEventPayload HealthPayload;
	HealthPayload.EventTag = HealthChangeTag;
	HealthPayload.Character = Enemy1;
	HealthPayload.PreviousHealth = 100.0f;
	HealthPayload.NewHealth = 70.0f;
	HealthPayload.MaxHealth = 100.0f;
	EventSubsystem->BroadcastEvent(HealthPayload);

	// Verify UI received health change
	TestEqual(TEXT("UI should receive health change"), Fixture.UISystem.HealthChangeEventsReceived, 1);
	TestEqual(TEXT("UI should show correct health"), Fixture.UISystem.LastHealthValue, 70.0f);

	// Scenario 4: Multiple attacks and damage to kill Enemy1
	for (int32 i = 0; i < 2; ++i)
	{
		AttackPayload.AttackLocation = FVector(150 + i * 50, 150, 0);
		EventSubsystem->BroadcastEvent(AttackPayload);
		
		DamagePayload.DamageAmount = 35.0f;
		EventSubsystem->BroadcastEvent(DamagePayload);
		
		HealthPayload.PreviousHealth = HealthPayload.NewHealth;
		HealthPayload.NewHealth -= 35.0f;
		EventSubsystem->BroadcastEvent(HealthPayload);
	}

	// Scenario 5: Enemy1 dies
	FDelveDeepKillEventPayload KillPayload;
	KillPayload.EventTag = KillTag;
	KillPayload.Killer = Player;
	KillPayload.Victim = Enemy1;
	KillPayload.ExperienceAwarded = 75;
	EventSubsystem->BroadcastEvent(KillPayload);

	// Verify all systems processed kill event
	TestEqual(TEXT("Combat system should track kill"), Fixture.CombatSystem.KillEventsReceived, 1);
	TestEqual(TEXT("Progression should receive kill event"), Fixture.ProgressionSystem.KillEventsReceived, 1);
	TestEqual(TEXT("Progression should award XP"), Fixture.ProgressionSystem.ExperienceGained, 75);

	// Scenario 6: Player attacks Enemy2 (different location)
	AttackPayload.AttackLocation = FVector(500, 500, 0);
	AttackPayload.Victim = Enemy2;
	EventSubsystem->BroadcastEvent(AttackPayload);

	DamagePayload.Victim = Enemy2;
	DamagePayload.DamageAmount = 40.0f;
	EventSubsystem->BroadcastEvent(DamagePayload);

	// Verify final state across all systems
	TestEqual(TEXT("Combat: Total attacks"), Fixture.CombatSystem.AttackEventsReceived, 4);
	TestEqual(TEXT("Combat: Total damage events"), Fixture.CombatSystem.DamageEventsReceived, 4);
	TestEqual(TEXT("Combat: Total damage dealt"), Fixture.CombatSystem.TotalDamageDealt, 140.0f);
	TestEqual(TEXT("Combat: Total kills"), Fixture.CombatSystem.TotalKills, 1);
	
	TestEqual(TEXT("UI: Total damage events"), Fixture.UISystem.DamageEventsReceived, 4);
	TestEqual(TEXT("UI: Total health events"), Fixture.UISystem.HealthChangeEventsReceived, 3);
	
	TestEqual(TEXT("Progression: Total kills"), Fixture.ProgressionSystem.KillEventsReceived, 1);
	TestEqual(TEXT("Progression: Total XP"), Fixture.ProgressionSystem.ExperienceGained, 75);
	
	TestEqual(TEXT("AI: Total attacks detected"), Fixture.AISystem.AttackEventsReceived, 4);
	TestEqual(TEXT("AI: Total damage detected"), Fixture.AISystem.DamageEventsReceived, 4);
	TestEqual(TEXT("AI: Attack locations tracked"), Fixture.AISystem.AttackLocations.Num(), 4);

	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("Cross-system communication test complete"));
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Combat: %d attacks, %d damage, %.2f total damage, %d kills"), 
		Fixture.CombatSystem.AttackEventsReceived, Fixture.CombatSystem.DamageEventsReceived, 
		Fixture.CombatSystem.TotalDamageDealt, Fixture.CombatSystem.TotalKills);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  UI: %d damage, %d health"), 
		Fixture.UISystem.DamageEventsReceived, Fixture.UISystem.HealthChangeEventsReceived);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Progression: %d kills, %d XP"), 
		Fixture.ProgressionSystem.KillEventsReceived, Fixture.ProgressionSystem.ExperienceGained);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  AI: %d attacks, %d damage"), 
		Fixture.AISystem.AttackEventsReceived, Fixture.AISystem.DamageEventsReceived);

	return true;
}

/**
 * Integration Test: Event Ordering Across Multiple Systems
 * Tests that events are processed in the correct order across systems
 * Verifies priority-based execution and event sequencing
 * Requirements: 1.1, 2.1, 3.1, 4.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventOrderingAcrossSystemsTest, 
	"DelveDeep.EventSystem.Integration.EventOrdering", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventOrderingAcrossSystemsTest::RunTest(const FString& Parameters)
{
	FEventSystemIntegrationTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Track execution order across systems
	TArray<FString> ExecutionOrder;

	// Setup event tag
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage.Dealt"));

	// Register listeners with different priorities from different systems
	// UI System - High Priority (should execute first)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(TEXT("UI-High"));
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	// Combat System - High Priority (should execute second, after UI)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(TEXT("Combat-High"));
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::High
	);

	// Progression System - Normal Priority (should execute third)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(TEXT("Progression-Normal"));
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal
	);

	// AI System - Normal Priority (should execute fourth, after Progression)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(TEXT("AI-Normal"));
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Normal
	);

	// Statistics System - Low Priority (should execute last)
	EventSubsystem->RegisterListener(
		DamageTag,
		[&ExecutionOrder](const FDelveDeepEventPayload& Payload)
		{
			ExecutionOrder.Add(TEXT("Statistics-Low"));
		},
		Fixture.GameInstance,
		EDelveDeepEventPriority::Low
	);

	// Broadcast damage event
	FDelveDeepDamageEventPayload DamagePayload;
	DamagePayload.EventTag = DamageTag;
	DamagePayload.Attacker = NewObject<AActor>();
	DamagePayload.Victim = NewObject<AActor>();
	DamagePayload.DamageAmount = 50.0f;
	EventSubsystem->BroadcastEvent(DamagePayload);

	// Verify execution order
	TestEqual(TEXT("Should execute 5 listeners"), ExecutionOrder.Num(), 5);
	
	// Verify High priority listeners execute first
	TestEqual(TEXT("First listener should be UI-High"), ExecutionOrder[0], TEXT("UI-High"));
	TestEqual(TEXT("Second listener should be Combat-High"), ExecutionOrder[1], TEXT("Combat-High"));
	
	// Verify Normal priority listeners execute second
	TestEqual(TEXT("Third listener should be Progression-Normal"), ExecutionOrder[2], TEXT("Progression-Normal"));
	TestEqual(TEXT("Fourth listener should be AI-Normal"), ExecutionOrder[3], TEXT("AI-Normal"));
	
	// Verify Low priority listener executes last
	TestEqual(TEXT("Fifth listener should be Statistics-Low"), ExecutionOrder[4], TEXT("Statistics-Low"));

	// Test with multiple events to verify consistent ordering
	ExecutionOrder.Empty();
	
	for (int32 i = 0; i < 3; ++i)
	{
		DamagePayload.DamageAmount = 10.0f * (i + 1);
		EventSubsystem->BroadcastEvent(DamagePayload);
	}

	// Verify ordering is consistent across multiple events
	TestEqual(TEXT("Should execute 15 listeners (5 per event)"), ExecutionOrder.Num(), 15);
	
	// Check first event ordering
	TestEqual(TEXT("Event 1: First should be UI-High"), ExecutionOrder[0], TEXT("UI-High"));
	TestEqual(TEXT("Event 1: Last should be Statistics-Low"), ExecutionOrder[4], TEXT("Statistics-Low"));
	
	// Check second event ordering
	TestEqual(TEXT("Event 2: First should be UI-High"), ExecutionOrder[5], TEXT("UI-High"));
	TestEqual(TEXT("Event 2: Last should be Statistics-Low"), ExecutionOrder[9], TEXT("Statistics-Low"));
	
	// Check third event ordering
	TestEqual(TEXT("Event 3: First should be UI-High"), ExecutionOrder[10], TEXT("UI-High"));
	TestEqual(TEXT("Event 3: Last should be Statistics-Low"), ExecutionOrder[14], TEXT("Statistics-Low"));

	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("Event ordering test complete"));
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Execution order verified across %d events"), 3);

	return true;
}

/**
 * Integration Test: Realistic Event Load Scenario
 * Tests system performance with realistic event load from multiple systems
 * Simulates a typical combat encounter with multiple enemies
 * Requirements: 1.1, 2.1, 3.1, 4.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventRealisticLoadScenarioTest, 
	"DelveDeep.EventSystem.Integration.RealisticLoadScenario", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventRealisticLoadScenarioTest::RunTest(const FString& Parameters)
{
	FEventSystemIntegrationTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Setup event tags
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Attack"));
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage"));
	FGameplayTag HealthChangeTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Character.Health"));
	FGameplayTag KillTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Kill"));

	// Register realistic number of listeners per system
	// UI System: 3 listeners (health bar, damage numbers, kill notifications)
	for (int32 i = 0; i < 3; ++i)
	{
		EventSubsystem->RegisterListener(
			DamageTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) { Fixture.UISystem.DamageEventsReceived++; },
			Fixture.GameInstance,
			EDelveDeepEventPriority::High
		);
	}

	for (int32 i = 0; i < 2; ++i)
	{
		EventSubsystem->RegisterListener(
			HealthChangeTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) { Fixture.UISystem.HealthChangeEventsReceived++; },
			Fixture.GameInstance,
			EDelveDeepEventPriority::High
		);
	}

	// Combat System: 4 listeners (damage tracking, combo system, hit effects, sound)
	for (int32 i = 0; i < 4; ++i)
	{
		EventSubsystem->RegisterListener(
			AttackTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) { Fixture.CombatSystem.AttackEventsReceived++; },
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	for (int32 i = 0; i < 3; ++i)
	{
		EventSubsystem->RegisterListener(
			DamageTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) 
			{ 
				Fixture.CombatSystem.DamageEventsReceived++;
				const FDelveDeepDamageEventPayload* DamagePayload = static_cast<const FDelveDeepDamageEventPayload*>(&Payload);
				Fixture.CombatSystem.TotalDamageDealt += DamagePayload->DamageAmount;
			},
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	// Progression System: 2 listeners (XP tracking, achievement tracking)
	for (int32 i = 0; i < 2; ++i)
	{
		EventSubsystem->RegisterListener(
			KillTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) 
			{ 
				Fixture.ProgressionSystem.KillEventsReceived++;
				const FDelveDeepKillEventPayload* KillPayload = static_cast<const FDelveDeepKillEventPayload*>(&Payload);
				Fixture.ProgressionSystem.ExperienceGained += KillPayload->ExperienceAwarded;
			},
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	// AI System: 3 listeners (threat detection, behavior response, group coordination)
	for (int32 i = 0; i < 3; ++i)
	{
		EventSubsystem->RegisterListener(
			AttackTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) { Fixture.AISystem.AttackEventsReceived++; },
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	for (int32 i = 0; i < 2; ++i)
	{
		EventSubsystem->RegisterListener(
			DamageTag,
			[&Fixture](const FDelveDeepEventPayload& Payload) { Fixture.AISystem.DamageEventsReceived++; },
			Fixture.GameInstance,
			EDelveDeepEventPriority::Low
		);
	}

	// Total listeners: 3+2+4+3+2+3+2 = 19 listeners

	// Simulate realistic combat scenario: Player vs 5 enemies
	AActor* Player = NewObject<AActor>();
	TArray<AActor*> Enemies;
	for (int32 i = 0; i < 5; ++i)
	{
		Enemies.Add(NewObject<AActor>());
	}

	// Measure performance
	double StartTime = FPlatformTime::Seconds();
	int32 TotalEventsGenerated = 0;

	// Simulate combat: Each enemy takes 3 hits to kill
	for (AActor* Enemy : Enemies)
	{
		for (int32 Hit = 0; Hit < 3; ++Hit)
		{
			// Attack event
			FDelveDeepAttackEventPayload AttackPayload;
			AttackPayload.EventTag = AttackTag;
			AttackPayload.Attacker = Player;
			AttackPayload.AttackLocation = FVector(100 * Hit, 100, 0);
			EventSubsystem->BroadcastEvent(AttackPayload);
			TotalEventsGenerated++;

			// Damage event
			FDelveDeepDamageEventPayload DamagePayload;
			DamagePayload.EventTag = DamageTag;
			DamagePayload.Attacker = Player;
			DamagePayload.Victim = Enemy;
			DamagePayload.DamageAmount = 35.0f;
			EventSubsystem->BroadcastEvent(DamagePayload);
			TotalEventsGenerated++;

			// Health change event
			FDelveDeepHealthChangeEventPayload HealthPayload;
			HealthPayload.EventTag = HealthChangeTag;
			HealthPayload.Character = Enemy;
			HealthPayload.PreviousHealth = 100.0f - (Hit * 35.0f);
			HealthPayload.NewHealth = 100.0f - ((Hit + 1) * 35.0f);
			HealthPayload.MaxHealth = 100.0f;
			EventSubsystem->BroadcastEvent(HealthPayload);
			TotalEventsGenerated++;
		}

		// Kill event
		FDelveDeepKillEventPayload KillPayload;
		KillPayload.EventTag = KillTag;
		KillPayload.Killer = Player;
		KillPayload.Victim = Enemy;
		KillPayload.ExperienceAwarded = 50;
		EventSubsystem->BroadcastEvent(KillPayload);
		TotalEventsGenerated++;
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalTime = (EndTime - StartTime) * 1000.0; // Convert to ms
	double AvgTimePerEvent = TotalTime / TotalEventsGenerated;

	// Verify all systems processed events correctly
	TestEqual(TEXT("UI should receive all damage events"), Fixture.UISystem.DamageEventsReceived, 15 * 3); // 15 damage events * 3 listeners
	TestEqual(TEXT("UI should receive all health events"), Fixture.UISystem.HealthChangeEventsReceived, 15 * 2); // 15 health events * 2 listeners
	TestEqual(TEXT("Combat should receive all attack events"), Fixture.CombatSystem.AttackEventsReceived, 15 * 4); // 15 attacks * 4 listeners
	TestEqual(TEXT("Combat should receive all damage events"), Fixture.CombatSystem.DamageEventsReceived, 15 * 3); // 15 damage * 3 listeners
	TestEqual(TEXT("Combat should track total damage"), Fixture.CombatSystem.TotalDamageDealt, 35.0f * 15 * 3); // 35 damage * 15 hits * 3 listeners
	TestEqual(TEXT("Progression should receive all kill events"), Fixture.ProgressionSystem.KillEventsReceived, 5 * 2); // 5 kills * 2 listeners
	TestEqual(TEXT("Progression should award correct XP"), Fixture.ProgressionSystem.ExperienceGained, 50 * 5 * 2); // 50 XP * 5 kills * 2 listeners
	TestEqual(TEXT("AI should receive all attack events"), Fixture.AISystem.AttackEventsReceived, 15 * 3); // 15 attacks * 3 listeners
	TestEqual(TEXT("AI should receive all damage events"), Fixture.AISystem.DamageEventsReceived, 15 * 2); // 15 damage * 2 listeners

	// Verify performance
	TestTrue(TEXT("Average time per event should be reasonable (<1ms)"), AvgTimePerEvent < 1.0);

	// Get performance metrics
	const FEventSystemMetrics& Metrics = EventSubsystem->GetPerformanceMetrics();

	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("Realistic load scenario complete"));
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Total events: %d"), TotalEventsGenerated);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Total time: %.2f ms"), TotalTime);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Avg time per event: %.4f ms"), AvgTimePerEvent);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Total listener invocations: %d"), Metrics.TotalListenerInvocations);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  UI: %d damage, %d health"), 
		Fixture.UISystem.DamageEventsReceived, Fixture.UISystem.HealthChangeEventsReceived);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Combat: %d attacks, %d damage, %.2f total damage"), 
		Fixture.CombatSystem.AttackEventsReceived, Fixture.CombatSystem.DamageEventsReceived, 
		Fixture.CombatSystem.TotalDamageDealt);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Progression: %d kills, %d XP"), 
		Fixture.ProgressionSystem.KillEventsReceived, Fixture.ProgressionSystem.ExperienceGained);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  AI: %d attacks, %d damage"), 
		Fixture.AISystem.AttackEventsReceived, Fixture.AISystem.DamageEventsReceived);

	return true;
}

/**
 * Integration Test: Performance with Realistic Listener Counts
 * Tests system performance with realistic listener counts per event type
 * Verifies that the system maintains performance targets with production-like load
 * Requirements: 1.1, 2.1, 3.1, 4.1
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepEventRealisticListenerCountsTest, 
	"DelveDeep.EventSystem.Integration.RealisticListenerCounts", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDelveDeepEventRealisticListenerCountsTest::RunTest(const FString& Parameters)
{
	FEventSystemIntegrationTestFixture Fixture;
	UDelveDeepEventSubsystem* EventSubsystem = Fixture.EventSubsystem;

	// Setup event tags for different event types
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage"));
	FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Character.Health"));
	FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Attack"));
	FGameplayTag KillTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Kill"));
	FGameplayTag ExperienceTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Progression.Experience"));

	// Register realistic listener counts for each event type
	// Damage events: 8-12 listeners (UI, combat, AI, sound, VFX, camera shake, etc.)
	const int32 DamageListenerCount = 10;
	for (int32 i = 0; i < DamageListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			DamageTag,
			[](const FDelveDeepEventPayload& Payload) 
			{ 
				// Simulate minimal work
				volatile float Temp = Payload.Timestamp.GetTicks() * 0.001f;
			},
			Fixture.GameInstance,
			(i < 3) ? EDelveDeepEventPriority::High : 
			(i < 7) ? EDelveDeepEventPriority::Normal : 
			EDelveDeepEventPriority::Low
		);
	}

	// Health events: 5-8 listeners (UI health bars, status effects, etc.)
	const int32 HealthListenerCount = 6;
	for (int32 i = 0; i < HealthListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			HealthTag,
			[](const FDelveDeepEventPayload& Payload) 
			{ 
				volatile float Temp = Payload.Timestamp.GetTicks() * 0.001f;
			},
			Fixture.GameInstance,
			(i < 2) ? EDelveDeepEventPriority::High : EDelveDeepEventPriority::Normal
		);
	}

	// Attack events: 6-10 listeners (combat, AI, animation, sound, etc.)
	const int32 AttackListenerCount = 8;
	for (int32 i = 0; i < AttackListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			AttackTag,
			[](const FDelveDeepEventPayload& Payload) 
			{ 
				volatile float Temp = Payload.Timestamp.GetTicks() * 0.001f;
			},
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	// Kill events: 4-6 listeners (progression, UI, achievements, statistics, etc.)
	const int32 KillListenerCount = 5;
	for (int32 i = 0; i < KillListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			KillTag,
			[](const FDelveDeepEventPayload& Payload) 
			{ 
				volatile float Temp = Payload.Timestamp.GetTicks() * 0.001f;
			},
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	// Experience events: 3-5 listeners (progression, UI, achievements, etc.)
	const int32 ExperienceListenerCount = 4;
	for (int32 i = 0; i < ExperienceListenerCount; ++i)
	{
		EventSubsystem->RegisterListener(
			ExperienceTag,
			[](const FDelveDeepEventPayload& Payload) 
			{ 
				volatile float Temp = Payload.Timestamp.GetTicks() * 0.001f;
			},
			Fixture.GameInstance,
			EDelveDeepEventPriority::Normal
		);
	}

	// Total listeners: 10 + 6 + 8 + 5 + 4 = 33 listeners

	// Test performance with realistic event mix
	const int32 EventsPerType = 100;
	TArray<double> BroadcastTimes;
	BroadcastTimes.Reserve(EventsPerType * 5);

	// Broadcast damage events
	for (int32 i = 0; i < EventsPerType; ++i)
	{
		FDelveDeepDamageEventPayload Payload;
		Payload.EventTag = DamageTag;
		Payload.Attacker = NewObject<AActor>();
		Payload.Victim = NewObject<AActor>();
		Payload.DamageAmount = 25.0f;

		double StartTime = FPlatformTime::Seconds();
		EventSubsystem->BroadcastEvent(Payload);
		double EndTime = FPlatformTime::Seconds();
		BroadcastTimes.Add((EndTime - StartTime) * 1000.0);
	}

	// Broadcast health events
	for (int32 i = 0; i < EventsPerType; ++i)
	{
		FDelveDeepHealthChangeEventPayload Payload;
		Payload.EventTag = HealthTag;
		Payload.Character = NewObject<AActor>();
		Payload.PreviousHealth = 100.0f;
		Payload.NewHealth = 75.0f;
		Payload.MaxHealth = 100.0f;

		double StartTime = FPlatformTime::Seconds();
		EventSubsystem->BroadcastEvent(Payload);
		double EndTime = FPlatformTime::Seconds();
		BroadcastTimes.Add((EndTime - StartTime) * 1000.0);
	}

	// Broadcast attack events
	for (int32 i = 0; i < EventsPerType; ++i)
	{
		FDelveDeepAttackEventPayload Payload;
		Payload.EventTag = AttackTag;
		Payload.Attacker = NewObject<AActor>();
		Payload.AttackLocation = FVector(100, 100, 0);

		double StartTime = FPlatformTime::Seconds();
		EventSubsystem->BroadcastEvent(Payload);
		double EndTime = FPlatformTime::Seconds();
		BroadcastTimes.Add((EndTime - StartTime) * 1000.0);
	}

	// Broadcast kill events
	for (int32 i = 0; i < EventsPerType; ++i)
	{
		FDelveDeepKillEventPayload Payload;
		Payload.EventTag = KillTag;
		Payload.Killer = NewObject<AActor>();
		Payload.Victim = NewObject<AActor>();
		Payload.ExperienceAwarded = 50;

		double StartTime = FPlatformTime::Seconds();
		EventSubsystem->BroadcastEvent(Payload);
		double EndTime = FPlatformTime::Seconds();
		BroadcastTimes.Add((EndTime - StartTime) * 1000.0);
	}

	// Broadcast experience events
	for (int32 i = 0; i < EventsPerType; ++i)
	{
		FDelveDeepEventPayload Payload;
		Payload.EventTag = ExperienceTag;

		double StartTime = FPlatformTime::Seconds();
		EventSubsystem->BroadcastEvent(Payload);
		double EndTime = FPlatformTime::Seconds();
		BroadcastTimes.Add((EndTime - StartTime) * 1000.0);
	}

	// Calculate statistics
	double TotalTime = 0.0;
	double MinTime = DBL_MAX;
	double MaxTime = 0.0;

	for (double Time : BroadcastTimes)
	{
		TotalTime += Time;
		MinTime = FMath::Min(MinTime, Time);
		MaxTime = FMath::Max(MaxTime, Time);
	}

	double AvgTime = TotalTime / BroadcastTimes.Num();

	// Verify performance targets
	TestTrue(TEXT("Average broadcast time should be <1ms"), AvgTime < 1.0);
	TestTrue(TEXT("Max broadcast time should be reasonable (<5ms)"), MaxTime < 5.0);

	// Get performance metrics
	const FEventSystemMetrics& Metrics = EventSubsystem->GetPerformanceMetrics();

	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("Realistic listener counts test complete"));
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Total listeners: %d"), 
		DamageListenerCount + HealthListenerCount + AttackListenerCount + KillListenerCount + ExperienceListenerCount);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Total events: %d"), EventsPerType * 5);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Avg broadcast time: %.4f ms"), AvgTime);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Min broadcast time: %.4f ms"), MinTime);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Max broadcast time: %.4f ms"), MaxTime);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Total listener invocations: %d"), Metrics.TotalListenerInvocations);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("  Listener counts per event type:"));
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("    Damage: %d"), DamageListenerCount);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("    Health: %d"), HealthListenerCount);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("    Attack: %d"), AttackListenerCount);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("    Kill: %d"), KillListenerCount);
	UE_LOG(LogDelveDeepEventIntegrationTests, Display, TEXT("    Experience: %d"), ExperienceListenerCount);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
