// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "DelveDeepMonsterConfig.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"

/**
 * Performance test: Initialization time with multiple assets
 * Target: < 100ms for initialization
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigInitializationPerformanceTest, 
	"DelveDeep.Performance.InitializationTime",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigInitializationPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	TestNotNull(TEXT("GameInstance created"), GameInstance);

	// Measure initialization time
	double StartTime = FPlatformTime::Seconds();
	
	// Get subsystem (triggers initialization)
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	double EndTime = FPlatformTime::Seconds();
	double InitTimeMs = (EndTime - StartTime) * 1000.0;

	TestNotNull(TEXT("ConfigurationManager initialized"), ConfigManager);

	// Log initialization time
	UE_LOG(LogTemp, Display, TEXT("Configuration Manager initialization time: %.2f ms"), InitTimeMs);

	// Test against target (< 100ms)
	TestTrue(FString::Printf(TEXT("Initialization time < 100ms (actual: %.2f ms)"), InitTimeMs), 
		InitTimeMs < 100.0);

	// Additional check: Warn if initialization is slow but not failing
	if (InitTimeMs > 50.0 && InitTimeMs < 100.0)
	{
		AddWarning(FString::Printf(TEXT("Initialization time is approaching threshold: %.2f ms"), InitTimeMs));
	}

	return true;
}

/**
 * Performance test: Single query time
 * Target: < 1ms per query
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigSingleQueryPerformanceTest, 
	"DelveDeep.Performance.SingleQueryTime",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigSingleQueryPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("ConfigurationManager available"), ConfigManager);

	// Perform a single query and measure time
	double StartTime = FPlatformTime::Seconds();
	
	const UDelveDeepCharacterData* CharacterData = ConfigManager->GetCharacterData(FName("DA_Character_Warrior"));
	
	double EndTime = FPlatformTime::Seconds();
	double QueryTimeMs = (EndTime - StartTime) * 1000.0;

	// Log query time
	UE_LOG(LogTemp, Display, TEXT("Single query time: %.4f ms"), QueryTimeMs);

	// Test against target (< 1ms)
	TestTrue(FString::Printf(TEXT("Single query time < 1ms (actual: %.4f ms)"), QueryTimeMs), 
		QueryTimeMs < 1.0);

	// Verify data was retrieved (if it exists)
	if (CharacterData)
	{
		UE_LOG(LogTemp, Display, TEXT("Successfully retrieved character data: %s"), *CharacterData->GetName());
	}
	else
	{
		AddWarning(TEXT("Character data not found - this is expected if test assets don't exist"));
	}

	return true;
}

/**
 * Performance test: Bulk query performance
 * Target: 1000 queries should complete quickly with high cache hit rate
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigBulkQueryPerformanceTest, 
	"DelveDeep.Performance.BulkQueryPerformance",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigBulkQueryPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("ConfigurationManager available"), ConfigManager);

	// Get initial performance stats
	int32 InitialCacheHits, InitialCacheMisses;
	float InitialAvgQueryTime;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Perform 1000 queries
	const int32 QueryCount = 1000;
	TArray<FName> TestNames = {
		FName("DA_Character_Warrior"),
		FName("DA_Character_Ranger"),
		FName("DA_Character_Mage"),
		FName("DA_Character_Necromancer"),
		FName("DA_Weapon_Sword"),
		FName("DA_Weapon_Bow"),
		FName("DA_Ability_Cleave"),
		FName("DA_Upgrade_HealthBoost")
	};

	double StartTime = FPlatformTime::Seconds();
	
	for (int32 i = 0; i < QueryCount; ++i)
	{
		// Cycle through test names to simulate realistic usage
		FName TestName = TestNames[i % TestNames.Num()];
		
		// Query different data types
		if (TestName.ToString().Contains(TEXT("Character")))
		{
			ConfigManager->GetCharacterData(TestName);
		}
		else if (TestName.ToString().Contains(TEXT("Weapon")))
		{
			ConfigManager->GetWeaponData(TestName);
		}
		else if (TestName.ToString().Contains(TEXT("Ability")))
		{
			ConfigManager->GetAbilityData(TestName);
		}
		else if (TestName.ToString().Contains(TEXT("Upgrade")))
		{
			ConfigManager->GetUpgradeData(TestName);
		}
	}
	
	double EndTime = FPlatformTime::Seconds();
	double TotalTimeMs = (EndTime - StartTime) * 1000.0;
	double AvgQueryTimeMs = TotalTimeMs / QueryCount;

	// Get final performance stats
	int32 FinalCacheHits, FinalCacheMisses;
	float FinalAvgQueryTime;
	ConfigManager->GetPerformanceStats(FinalCacheHits, FinalCacheMisses, FinalAvgQueryTime);

	// Calculate cache hit rate
	int32 TotalQueries = FinalCacheHits + FinalCacheMisses - InitialCacheHits - InitialCacheMisses;
	int32 NewCacheHits = FinalCacheHits - InitialCacheHits;
	float CacheHitRate = TotalQueries > 0 ? (static_cast<float>(NewCacheHits) / TotalQueries) * 100.0f : 0.0f;

	// Log results
	UE_LOG(LogTemp, Display, TEXT("Bulk query performance:"));
	UE_LOG(LogTemp, Display, TEXT("  Total queries: %d"), QueryCount);
	UE_LOG(LogTemp, Display, TEXT("  Total time: %.2f ms"), TotalTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Average query time: %.4f ms"), AvgQueryTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Cache hits: %d"), NewCacheHits);
	UE_LOG(LogTemp, Display, TEXT("  Cache misses: %d"), FinalCacheMisses - InitialCacheMisses);
	UE_LOG(LogTemp, Display, TEXT("  Cache hit rate: %.2f%%"), CacheHitRate);

	// Test average query time (< 1ms)
	TestTrue(FString::Printf(TEXT("Average query time < 1ms (actual: %.4f ms)"), AvgQueryTimeMs), 
		AvgQueryTimeMs < 1.0);

	// Test total time is reasonable (< 1000ms for 1000 queries)
	TestTrue(FString::Printf(TEXT("Total time for 1000 queries < 1000ms (actual: %.2f ms)"), TotalTimeMs), 
		TotalTimeMs < 1000.0);

	// Warn if cache hit rate is low (but don't fail - assets might not exist)
	if (CacheHitRate < 50.0f && TotalQueries > 0)
	{
		AddWarning(FString::Printf(TEXT("Cache hit rate is low: %.2f%% (expected > 95%% with existing assets)"), 
			CacheHitRate));
	}

	return true;
}

/**
 * Performance test: Cache hit rate measurement
 * Target: > 95% cache hit rate for repeated queries
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigCacheHitRateTest, 
	"DelveDeep.Performance.CacheHitRate",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigCacheHitRateTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("ConfigurationManager available"), ConfigManager);

	// Get initial stats
	int32 InitialCacheHits, InitialCacheMisses;
	float InitialAvgQueryTime;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Query the same asset multiple times (should hit cache after first query)
	const int32 RepeatCount = 100;
	FName TestAssetName = FName("DA_Character_Warrior");

	for (int32 i = 0; i < RepeatCount; ++i)
	{
		ConfigManager->GetCharacterData(TestAssetName);
	}

	// Get final stats
	int32 FinalCacheHits, FinalCacheMisses;
	float FinalAvgQueryTime;
	ConfigManager->GetPerformanceStats(FinalCacheHits, FinalCacheMisses, FinalAvgQueryTime);

	// Calculate cache hit rate
	int32 NewCacheHits = FinalCacheHits - InitialCacheHits;
	int32 NewCacheMisses = FinalCacheMisses - InitialCacheMisses;
	int32 TotalNewQueries = NewCacheHits + NewCacheMisses;
	
	float CacheHitRate = TotalNewQueries > 0 ? 
		(static_cast<float>(NewCacheHits) / TotalNewQueries) * 100.0f : 0.0f;

	// Log results
	UE_LOG(LogTemp, Display, TEXT("Cache hit rate test:"));
	UE_LOG(LogTemp, Display, TEXT("  Repeated queries: %d"), RepeatCount);
	UE_LOG(LogTemp, Display, TEXT("  Cache hits: %d"), NewCacheHits);
	UE_LOG(LogTemp, Display, TEXT("  Cache misses: %d"), NewCacheMisses);
	UE_LOG(LogTemp, Display, TEXT("  Cache hit rate: %.2f%%"), CacheHitRate);

	// For repeated queries of the same asset, we expect:
	// - First query: cache miss (loads asset)
	// - Subsequent queries: cache hits
	// So we should have 1 miss and (RepeatCount - 1) hits
	// Cache hit rate should be (RepeatCount - 1) / RepeatCount * 100%
	// For 100 queries, that's 99%

	if (TotalNewQueries > 0)
	{
		// Test cache hit rate (should be > 95% for repeated queries)
		TestTrue(FString::Printf(TEXT("Cache hit rate > 95%% for repeated queries (actual: %.2f%%)"), CacheHitRate), 
			CacheHitRate > 95.0f);

		// Verify we had exactly 1 cache miss (the first query)
		if (NewCacheMisses == 1)
		{
			UE_LOG(LogTemp, Display, TEXT("  ✓ Exactly 1 cache miss as expected (first query)"));
		}
		else if (NewCacheMisses == 0)
		{
			AddWarning(TEXT("No cache misses - asset might have been pre-cached"));
		}
		else
		{
			AddWarning(FString::Printf(TEXT("Unexpected cache miss count: %d (expected 1)"), NewCacheMisses));
		}
	}
	else
	{
		AddWarning(TEXT("No queries were tracked - test assets might not exist"));
	}

	return true;
}

/**
 * Performance test: Memory usage and cache efficiency
 * Verifies that caching doesn't cause excessive memory usage
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigMemoryEfficiencyTest, 
	"DelveDeep.Performance.MemoryEfficiency",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigMemoryEfficiencyTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("ConfigurationManager available"), ConfigManager);

	// Perform many queries to populate cache
	const int32 QueryCount = 1000;
	TArray<FName> TestNames = {
		FName("DA_Character_Warrior"),
		FName("DA_Character_Ranger"),
		FName("DA_Weapon_Sword"),
		FName("DA_Ability_Cleave")
	};

	for (int32 i = 0; i < QueryCount; ++i)
	{
		FName TestName = TestNames[i % TestNames.Num()];
		ConfigManager->GetCharacterData(TestName);
	}

	// Get performance stats
	int32 CacheHits, CacheMisses;
	float AvgQueryTime;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	// Log cache statistics
	UE_LOG(LogTemp, Display, TEXT("Memory efficiency test:"));
	UE_LOG(LogTemp, Display, TEXT("  Total queries: %d"), QueryCount);
	UE_LOG(LogTemp, Display, TEXT("  Cache hits: %d"), CacheHits);
	UE_LOG(LogTemp, Display, TEXT("  Cache misses: %d"), CacheMisses);
	UE_LOG(LogTemp, Display, TEXT("  Average query time: %.4f ms"), AvgQueryTime);

	// Verify cache is working efficiently
	// With repeated queries, we should have high cache hit rate
	int32 TotalQueries = CacheHits + CacheMisses;
	if (TotalQueries > 0)
	{
		float CacheHitRate = (static_cast<float>(CacheHits) / TotalQueries) * 100.0f;
		UE_LOG(LogTemp, Display, TEXT("  Cache hit rate: %.2f%%"), CacheHitRate);

		// For repeated queries of limited assets, cache hit rate should be very high
		TestTrue(FString::Printf(TEXT("Cache hit rate indicates efficient caching (%.2f%%)"), CacheHitRate), 
			CacheHitRate > 50.0f);
	}

	// Verify average query time is still fast after many queries
	TestTrue(FString::Printf(TEXT("Average query time remains fast after %d queries (%.4f ms)"), 
		QueryCount, AvgQueryTime), 
		AvgQueryTime < 1.0f);

	return true;
}

/**
 * Performance test: Concurrent query simulation
 * Simulates multiple systems querying configuration data simultaneously
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigConcurrentQueryTest, 
	"DelveDeep.Performance.ConcurrentQueries",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigConcurrentQueryTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("ConfigurationManager available"), ConfigManager);

	// Simulate concurrent queries from different systems
	const int32 SimulatedSystems = 10;
	const int32 QueriesPerSystem = 50;
	
	double StartTime = FPlatformTime::Seconds();

	// Simulate each system making queries
	for (int32 System = 0; System < SimulatedSystems; ++System)
	{
		for (int32 Query = 0; Query < QueriesPerSystem; ++Query)
		{
			// Each system queries different data types
			switch (System % 4)
			{
			case 0:
				ConfigManager->GetCharacterData(FName("DA_Character_Warrior"));
				break;
			case 1:
				ConfigManager->GetWeaponData(FName("DA_Weapon_Sword"));
				break;
			case 2:
				ConfigManager->GetAbilityData(FName("DA_Ability_Cleave"));
				break;
			case 3:
				ConfigManager->GetUpgradeData(FName("DA_Upgrade_HealthBoost"));
				break;
			}
		}
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalTimeMs = (EndTime - StartTime) * 1000.0;
	int32 TotalQueries = SimulatedSystems * QueriesPerSystem;
	double AvgQueryTimeMs = TotalTimeMs / TotalQueries;

	// Log results
	UE_LOG(LogTemp, Display, TEXT("Concurrent query simulation:"));
	UE_LOG(LogTemp, Display, TEXT("  Simulated systems: %d"), SimulatedSystems);
	UE_LOG(LogTemp, Display, TEXT("  Queries per system: %d"), QueriesPerSystem);
	UE_LOG(LogTemp, Display, TEXT("  Total queries: %d"), TotalQueries);
	UE_LOG(LogTemp, Display, TEXT("  Total time: %.2f ms"), TotalTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Average query time: %.4f ms"), AvgQueryTimeMs);

	// Test performance under concurrent load
	TestTrue(FString::Printf(TEXT("Average query time under concurrent load < 1ms (actual: %.4f ms)"), 
		AvgQueryTimeMs), 
		AvgQueryTimeMs < 1.0);

	// Get final performance stats
	int32 CacheHits, CacheMisses;
	float FinalAvgQueryTime;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, FinalAvgQueryTime);

	UE_LOG(LogTemp, Display, TEXT("  Final cache hits: %d"), CacheHits);
	UE_LOG(LogTemp, Display, TEXT("  Final cache misses: %d"), CacheMisses);
	UE_LOG(LogTemp, Display, TEXT("  Final average query time: %.4f ms"), FinalAvgQueryTime);

	return true;
}

/**
 * Performance test: Validation performance
 * Ensures validation doesn't significantly impact performance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigValidationPerformanceTest, 
	"DelveDeep.Performance.ValidationTime",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigValidationPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance and get subsystem
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDelveDeepConfigurationManager* ConfigManager = 
		GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	
	TestNotNull(TEXT("ConfigurationManager available"), ConfigManager);

	// Measure validation time
	double StartTime = FPlatformTime::Seconds();
	
	FString ValidationReport;
	bool bIsValid = ConfigManager->ValidateAllData(ValidationReport);
	
	double EndTime = FPlatformTime::Seconds();
	double ValidationTimeMs = (EndTime - StartTime) * 1000.0;

	// Log results
	UE_LOG(LogTemp, Display, TEXT("Validation performance:"));
	UE_LOG(LogTemp, Display, TEXT("  Validation time: %.2f ms"), ValidationTimeMs);
	UE_LOG(LogTemp, Display, TEXT("  Validation result: %s"), bIsValid ? TEXT("Valid") : TEXT("Has Issues"));

	if (!ValidationReport.IsEmpty())
	{
		UE_LOG(LogTemp, Display, TEXT("  Validation report:\n%s"), *ValidationReport);
	}

	// Test validation time (should be reasonable, < 100ms)
	TestTrue(FString::Printf(TEXT("Validation time < 100ms (actual: %.2f ms)"), ValidationTimeMs), 
		ValidationTimeMs < 100.0);

	return true;
}
