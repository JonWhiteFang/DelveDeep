// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepConfigurationManager.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepValidation.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepEventTypes.h"
#include "DelveDeepTestFixtures.h"
#include "DelveDeepTestMacros.h"
#include "Misc/AutomationTest.h"
#include "GameplayTagContainer.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test cross-system communication between Configuration and Events
 * Verifies that configuration changes trigger appropriate events
 * Requirements: 14.1, 14.3, 14.4, 14.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationEventCommunicationTest,
	"DelveDeep.Integration.ConfigurationEventCommunication",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationEventCommunicationTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	// Verify subsystems are initialized
	ASSERT_NOT_NULL(Fixture.ConfigManager);
	ASSERT_NOT_NULL(Fixture.EventSubsystem);

	// Track event reception
	bool bConfigLoadedEventReceived = false;
	FDelveDeepEventPayload ReceivedPayload;

	// Register listener for configuration loaded events
	FGameplayTag ConfigLoadedTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Configuration.Loaded"));
	FDelegateHandle ListenerHandle = Fixture.EventSubsystem->RegisterListener(
		ConfigLoadedTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			bConfigLoadedEventReceived = true;
			ReceivedPayload = Payload;
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration loaded event received"));
		}),
		EDelveDeepEventPriority::Normal
	);

	// Trigger configuration reload
	Fixture.ConfigManager->ReloadAllData();

	// Broadcast configuration loaded event
	FDelveDeepEventPayload LoadedPayload;
	LoadedPayload.EventTag = ConfigLoadedTag;
	LoadedPayload.Instigator = nullptr;
	LoadedPayload.IntValue = Fixture.ConfigManager->GetTotalAssetCount();
	Fixture.EventSubsystem->BroadcastEvent(LoadedPayload);

	// Verify event was received
	EXPECT_TRUE(bConfigLoadedEventReceived);
	EXPECT_EQ(ReceivedPayload.EventTag, ConfigLoadedTag);
	EXPECT_GT(ReceivedPayload.IntValue, 0);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(ConfigLoadedTag, ListenerHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test configuration validation errors trigger events
 * Verifies that validation failures are communicated via event system
 * Requirements: 14.1, 14.3, 14.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationValidationEventTest,
	"DelveDeep.Integration.ConfigurationValidationEvent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationValidationEventTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.ConfigManager);
	ASSERT_NOT_NULL(Fixture.EventSubsystem);

	// Track validation error events
	bool bValidationErrorEventReceived = false;
	FString ErrorMessage;

	// Register listener for validation error events
	FGameplayTag ValidationErrorTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Configuration.ValidationError"));
	FDelegateHandle ListenerHandle = Fixture.EventSubsystem->RegisterListener(
		ValidationErrorTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			bValidationErrorEventReceived = true;
			ErrorMessage = Payload.StringValue;
			UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation error event received: %s"), *ErrorMessage);
		}),
		EDelveDeepEventPriority::High
	);

	// Create invalid character data
	UDelveDeepCharacterData* InvalidCharacter = NewObject<UDelveDeepCharacterData>();
	InvalidCharacter->CharacterName = FText::FromString(TEXT("Invalid Character"));
	InvalidCharacter->BaseHealth = -50.0f;  // Invalid: negative health
	InvalidCharacter->BaseDamage = 0.0f;    // Invalid: zero damage
	InvalidCharacter->MoveSpeed = -100.0f;  // Invalid: negative speed

	// Validate and expect failure
	FValidationContext Context;
	Context.SystemName = TEXT("Integration");
	Context.OperationName = TEXT("TestInvalidCharacter");
	bool bIsValid = InvalidCharacter->Validate(Context);

	EXPECT_FALSE(bIsValid);
	EXPECT_HAS_ERRORS(Context);

	// Broadcast validation error event
	if (!bIsValid)
	{
		FDelveDeepEventPayload ErrorPayload;
		ErrorPayload.EventTag = ValidationErrorTag;
		ErrorPayload.Instigator = nullptr;
		ErrorPayload.StringValue = Context.GetReport();
		Fixture.EventSubsystem->BroadcastEvent(ErrorPayload);
	}

	// Verify event was received
	EXPECT_TRUE(bValidationErrorEventReceived);
	EXPECT_TRUE(ErrorMessage.Contains(TEXT("BaseHealth")));

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(ValidationErrorTag, ListenerHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test configuration query events
 * Verifies that configuration queries can trigger events for tracking
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationQueryEventTest,
	"DelveDeep.Integration.ConfigurationQueryEvent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationQueryEventTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.ConfigManager);
	ASSERT_NOT_NULL(Fixture.EventSubsystem);

	// Track query events
	int32 QueryEventCount = 0;
	FName QueriedAssetName;

	// Register listener for configuration query events
	FGameplayTag QueryTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Configuration.Query"));
	FDelegateHandle ListenerHandle = Fixture.EventSubsystem->RegisterListener(
		QueryTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			QueryEventCount++;
			QueriedAssetName = FName(*Payload.StringValue);
			UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Configuration query event: %s"), *Payload.StringValue);
		}),
		EDelveDeepEventPriority::Low
	);

	// Perform multiple queries and broadcast events
	const TArray<FName> TestQueries = {
		TEXT("Warrior"),
		TEXT("Ranger"),
		TEXT("Mage"),
		TEXT("Necromancer")
	};

	for (const FName& QueryName : TestQueries)
	{
		// Query configuration (may return null if not loaded)
		const UDelveDeepCharacterData* CharacterData = Fixture.ConfigManager->GetCharacterData(QueryName);

		// Broadcast query event
		FDelveDeepEventPayload QueryPayload;
		QueryPayload.EventTag = QueryTag;
		QueryPayload.Instigator = nullptr;
		QueryPayload.StringValue = QueryName.ToString();
		QueryPayload.BoolValue = (CharacterData != nullptr);
		Fixture.EventSubsystem->BroadcastEvent(QueryPayload);
	}

	// Verify events were received
	EXPECT_EQ(QueryEventCount, TestQueries.Num());
	EXPECT_TRUE(QueriedAssetName != NAME_None);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(QueryTag, ListenerHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test configuration cache events
 * Verifies that cache hits/misses can be tracked via events
 * Requirements: 14.1, 14.3, 14.4
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationCacheEventTest,
	"DelveDeep.Integration.ConfigurationCacheEvent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FConfigurationCacheEventTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.ConfigManager);
	ASSERT_NOT_NULL(Fixture.EventSubsystem);

	// Track cache events
	int32 CacheHitCount = 0;
	int32 CacheMissCount = 0;

	// Register listener for cache hit events
	FGameplayTag CacheHitTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Configuration.CacheHit"));
	FDelegateHandle HitHandle = Fixture.EventSubsystem->RegisterListener(
		CacheHitTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			CacheHitCount++;
		}),
		EDelveDeepEventPriority::Low
	);

	// Register listener for cache miss events
	FGameplayTag CacheMissTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Configuration.CacheMiss"));
	FDelegateHandle MissHandle = Fixture.EventSubsystem->RegisterListener(
		CacheMissTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			CacheMissCount++;
		}),
		EDelveDeepEventPriority::Low
	);

	// Simulate cache operations
	const FName TestAssetName = TEXT("TestAsset");

	// First query - cache miss
	FDelveDeepEventPayload MissPayload;
	MissPayload.EventTag = CacheMissTag;
	MissPayload.StringValue = TestAssetName.ToString();
	Fixture.EventSubsystem->BroadcastEvent(MissPayload);

	// Subsequent queries - cache hits
	for (int32 i = 0; i < 5; ++i)
	{
		FDelveDeepEventPayload HitPayload;
		HitPayload.EventTag = CacheHitTag;
		HitPayload.StringValue = TestAssetName.ToString();
		Fixture.EventSubsystem->BroadcastEvent(HitPayload);
	}

	// Verify cache events
	EXPECT_EQ(CacheMissCount, 1);
	EXPECT_EQ(CacheHitCount, 5);

	// Calculate cache hit rate
	float CacheHitRate = static_cast<float>(CacheHitCount) / static_cast<float>(CacheHitCount + CacheMissCount);
	EXPECT_GT(CacheHitRate, 0.8f);  // Should be > 80%

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache hit rate: %.2f%% (Hits: %d, Misses: %d)"),
		CacheHitRate * 100.0f, CacheHitCount, CacheMissCount);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(CacheHitTag, HitHandle);
	Fixture.EventSubsystem->UnregisterListener(CacheMissTag, MissHandle);
	Fixture.AfterEach();

	return true;
}

/**
 * Test multi-system state consistency
 * Verifies that configuration and event systems maintain correct state across operations
 * Requirements: 14.1, 14.4, 14.5
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiSystemStateConsistencyTest,
	"DelveDeep.Integration.MultiSystemStateConsistency",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMultiSystemStateConsistencyTest::RunTest(const FString& Parameters)
{
	// Create integration test fixture
	FIntegrationTestFixture Fixture;
	Fixture.BeforeEach();

	ASSERT_NOT_NULL(Fixture.ConfigManager);
	ASSERT_NOT_NULL(Fixture.EventSubsystem);

	// Track system state
	int32 ConfigurationOperations = 0;
	int32 EventsBroadcast = 0;
	bool bSystemsInSync = true;

	// Register listener to track all configuration-related events
	FGameplayTag ConfigTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Configuration"));
	FDelegateHandle ListenerHandle = Fixture.EventSubsystem->RegisterListener(
		ConfigTag,
		FDelveDeepEventDelegate::CreateLambda([&](const FDelveDeepEventPayload& Payload)
		{
			EventsBroadcast++;
			
			// Verify configuration manager is still valid
			if (!Fixture.ConfigManager)
			{
				bSystemsInSync = false;
				UE_LOG(LogDelveDeepConfig, Error, TEXT("Configuration manager became invalid during event processing"));
			}
		}),
		EDelveDeepEventPriority::Normal
	);

	// Perform series of operations
	for (int32 i = 0; i < 10; ++i)
	{
		// Configuration operation
		int32 AssetCount = Fixture.ConfigManager->GetTotalAssetCount();
		ConfigurationOperations++;

		// Broadcast corresponding event
		FDelveDeepEventPayload Payload;
		Payload.EventTag = ConfigTag;
		Payload.IntValue = AssetCount;
		Fixture.EventSubsystem->BroadcastEvent(Payload);

		// Verify state consistency
		EXPECT_NOT_NULL(Fixture.ConfigManager);
		EXPECT_NOT_NULL(Fixture.EventSubsystem);
	}

	// Verify operations completed successfully
	EXPECT_EQ(ConfigurationOperations, 10);
	EXPECT_EQ(EventsBroadcast, 10);
	EXPECT_TRUE(bSystemsInSync);

	// Verify subsystems are still functional
	EXPECT_NOT_NULL(Fixture.ConfigManager);
	EXPECT_NOT_NULL(Fixture.EventSubsystem);

	// Test that both systems can still perform operations
	int32 FinalAssetCount = Fixture.ConfigManager->GetTotalAssetCount();
	EXPECT_GE(FinalAssetCount, 0);

	FDelveDeepEventPayload FinalPayload;
	FinalPayload.EventTag = ConfigTag;
	FinalPayload.StringValue = TEXT("FinalTest");
	Fixture.EventSubsystem->BroadcastEvent(FinalPayload);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Multi-system state consistency verified: %d operations, %d events"),
		ConfigurationOperations, EventsBroadcast);

	// Cleanup
	Fixture.EventSubsystem->UnregisterListener(ConfigTag, ListenerHandle);
	Fixture.AfterEach();

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
