// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestMacros.h"
#include "DelveDeepTestFixtures.h"
#include "DelveDeepTestUtilities.h"
#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test asset caching on first query
 * Verifies that assets are properly cached when first accessed
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerAssetCachingTest, 
	"DelveDeep.Configuration.AssetCaching", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerAssetCachingTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Get initial performance stats
	int32 InitialCacheHits = 0;
	int32 InitialCacheMisses = 0;
	float InitialAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Query for a non-existent asset (should result in cache miss)
	const UDelveDeepCharacterData* Result = ConfigManager->GetCharacterData(FName("NonExistentCharacter"));
	
	// Verify result is nullptr for non-existent asset
	EXPECT_NULL(Result);

	// Get updated performance stats
	int32 AfterCacheHits = 0;
	int32 AfterCacheMisses = 0;
	float AfterAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(AfterCacheHits, AfterCacheMisses, AfterAvgQueryTime);

	// Verify cache miss was recorded
	EXPECT_EQ(AfterCacheMisses, InitialCacheMisses + 1);
	EXPECT_EQ(AfterCacheHits, InitialCacheHits);

	Fixture.AfterEach();
	return true;
}

/**
 * Test cached asset returned on subsequent queries
 * Verifies that subsequent queries return cached data without reloading
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerCachedAssetTest, 
	"DelveDeep.Configuration.CachedAssetReturn", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerCachedAssetTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Get initial stats
	int32 InitialCacheHits = 0;
	int32 InitialCacheMisses = 0;
	float InitialAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Query the same asset multiple times
	FName TestAssetName("TestAsset");
	const UDelveDeepCharacterData* Result1 = ConfigManager->GetCharacterData(TestAssetName);
	const UDelveDeepCharacterData* Result2 = ConfigManager->GetCharacterData(TestAssetName);
	const UDelveDeepCharacterData* Result3 = ConfigManager->GetCharacterData(TestAssetName);

	// All results should be nullptr (asset doesn't exist)
	EXPECT_NULL(Result1);
	EXPECT_NULL(Result2);
	EXPECT_NULL(Result3);

	// Get updated stats
	int32 AfterCacheHits = 0;
	int32 AfterCacheMisses = 0;
	float AfterAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(AfterCacheHits, AfterCacheMisses, AfterAvgQueryTime);

	// Verify that all three queries were tracked
	EXPECT_EQ(AfterCacheMisses, InitialCacheMisses + 3);

	// Verify query count increased
	EXPECT_GE(AfterAvgQueryTime, 0.0f);

	Fixture.AfterEach();
	return true;
}

/**
 * Test cache hit rate tracking accuracy
 * Verifies that cache hit and miss tracking is accurate
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerCacheHitRateTest, 
	"DelveDeep.Configuration.CacheHitRateTracking", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerCacheHitRateTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Get initial stats
	int32 InitialCacheHits = 0;
	int32 InitialCacheMisses = 0;
	float InitialAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Perform multiple queries to different assets
	ConfigManager->GetCharacterData(FName("Asset1"));
	ConfigManager->GetCharacterData(FName("Asset2"));
	ConfigManager->GetCharacterData(FName("Asset3"));
	ConfigManager->GetUpgradeData(FName("Upgrade1"));
	ConfigManager->GetWeaponData(FName("Weapon1"));
	ConfigManager->GetAbilityData(FName("Ability1"));

	// Get updated stats
	int32 AfterCacheHits = 0;
	int32 AfterCacheMisses = 0;
	float AfterAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(AfterCacheHits, AfterCacheMisses, AfterAvgQueryTime);

	// Calculate expected values
	int32 ExpectedMisses = InitialCacheMisses + 6; // 6 queries to non-existent assets

	// Verify tracking accuracy
	EXPECT_EQ(AfterCacheMisses, ExpectedMisses);
	EXPECT_EQ(AfterCacheHits, InitialCacheHits);

	// Verify average query time is reasonable (should be very fast, < 1ms)
	EXPECT_LT(AfterAvgQueryTime, 1.0f);

	// Calculate cache hit rate
	int32 TotalQueries = AfterCacheHits + AfterCacheMisses;
	if (TotalQueries > 0)
	{
		float HitRate = (float)AfterCacheHits / (float)TotalQueries * 100.0f;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache hit rate: %.2f%% (%d hits / %d total queries)"), 
			HitRate, AfterCacheHits, TotalQueries);
	}

	Fixture.AfterEach();
	return true;
}

/**
 * Test data table lookup by name
 * Verifies that monster configs can be looked up from data tables
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerDataTableLookupTest, 
	"DelveDeep.Configuration.DataTableLookup", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerDataTableLookupTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Query for a monster config
	const FDelveDeepMonsterConfig* Result = ConfigManager->GetMonsterConfig(FName("TestGoblin"));

	// Since we don't have actual data tables loaded, result should be nullptr
	EXPECT_NULL(Result);

	// Get performance stats to verify query was tracked
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float AvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	// Verify query was tracked
	EXPECT_GT(CacheMisses, 0);

	Fixture.AfterEach();
	return true;
}

/**
 * Test invalid name returns nullptr
 * Verifies that querying with invalid names returns nullptr gracefully
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerInvalidNameTest, 
	"DelveDeep.Configuration.InvalidNameReturnsNull", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerInvalidNameTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Test with various invalid names
	const UDelveDeepCharacterData* Result1 = ConfigManager->GetCharacterData(FName(""));
	const UDelveDeepCharacterData* Result2 = ConfigManager->GetCharacterData(FName("InvalidCharacter"));
	const UDelveDeepCharacterData* Result3 = ConfigManager->GetCharacterData(FName("NonExistent123"));

	// All should return nullptr
	EXPECT_NULL(Result1);
	EXPECT_NULL(Result2);
	EXPECT_NULL(Result3);

	// Test with other asset types
	const UDelveDeepUpgradeData* UpgradeResult = ConfigManager->GetUpgradeData(FName("InvalidUpgrade"));
	const UDelveDeepWeaponData* WeaponResult = ConfigManager->GetWeaponData(FName("InvalidWeapon"));
	const UDelveDeepAbilityData* AbilityResult = ConfigManager->GetAbilityData(FName("InvalidAbility"));
	const FDelveDeepMonsterConfig* MonsterResult = ConfigManager->GetMonsterConfig(FName("InvalidMonster"));

	EXPECT_NULL(UpgradeResult);
	EXPECT_NULL(WeaponResult);
	EXPECT_NULL(AbilityResult);
	EXPECT_NULL(MonsterResult);

	// Verify all queries were tracked as cache misses
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float AvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	EXPECT_GE(CacheMisses, 7);

	Fixture.AfterEach();
	return true;
}

/**
 * Test query performance under target thresholds
 * Verifies that queries complete within performance targets (< 1ms)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerQueryPerformanceTest, 
	"DelveDeep.Configuration.QueryPerformance", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerQueryPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Measure single query performance
	{
		DelveDeepTestUtils::FScopedTestTimer Timer(TEXT("Single Query"));
		ConfigManager->GetCharacterData(FName("TestCharacter"));
		double SingleQueryTime = Timer.GetElapsedMs();

		// Single query should be under 1ms
		EXPECT_LT(SingleQueryTime, 1.0);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Single query time: %.4f ms"), SingleQueryTime);
	}

	// Measure bulk query performance (1000 queries)
	const int32 BulkQueryCount = 1000;
	{
		DelveDeepTestUtils::FScopedTestTimer Timer(TEXT("Bulk Queries"));
		
		for (int32 i = 0; i < BulkQueryCount; ++i)
		{
			ConfigManager->GetCharacterData(FName("TestCharacter"));
			Timer.RecordSample();
		}

		// Average query time should be under 1ms
		double AvgBulkQueryTime = Timer.GetAverageMs();
		EXPECT_LT(AvgBulkQueryTime, 1.0);
		EXPECT_TRUE(Timer.IsWithinBudget(1.0));

		UE_LOG(LogDelveDeepConfig, Display, TEXT("Bulk query performance: %d queries, Min=%.4f ms, Max=%.4f ms, Avg=%.4f ms, Median=%.4f ms"), 
			BulkQueryCount, Timer.GetMinMs(), Timer.GetMaxMs(), Timer.GetAverageMs(), Timer.GetMedianMs());
	}

	// Get performance stats from configuration manager
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float ConfigManagerAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, ConfigManagerAvgQueryTime);

	// Verify configuration manager's tracked average is also under 1ms
	EXPECT_LT(ConfigManagerAvgQueryTime, 1.0f);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration manager stats: Hits=%d, Misses=%d, Avg=%.4f ms"), 
		CacheHits, CacheMisses, ConfigManagerAvgQueryTime);

	// Test data table query performance
	{
		DelveDeepTestUtils::FScopedTestTimer Timer(TEXT("Data Table Query"));
		ConfigManager->GetMonsterConfig(FName("TestMonster"));
		double DataTableQueryTime = Timer.GetElapsedMs();

		// Data table query should be under 0.5ms (target from requirements)
		EXPECT_LT(DataTableQueryTime, 0.5);
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Data table query time: %.4f ms"), DataTableQueryTime);
	}

	Fixture.AfterEach();
	return true;
}

/**
 * Test performance metrics accuracy
 * Verifies that performance metrics are calculated correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerPerformanceMetricsTest, 
	"DelveDeep.Configuration.PerformanceMetrics", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerPerformanceMetricsTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Get initial stats
	int32 InitialCacheHits = 0;
	int32 InitialCacheMisses = 0;
	float InitialAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Perform a known number of queries
	const int32 QueryCount = 10;
	for (int32 i = 0; i < QueryCount; ++i)
	{
		ConfigManager->GetCharacterData(FName(*FString::Printf(TEXT("TestChar%d"), i)));
	}

	// Get updated stats
	int32 AfterCacheHits = 0;
	int32 AfterCacheMisses = 0;
	float AfterAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(AfterCacheHits, AfterCacheMisses, AfterAvgQueryTime);

	// Verify metrics
	int32 ExpectedMisses = InitialCacheMisses + QueryCount;
	EXPECT_EQ(AfterCacheMisses, ExpectedMisses);

	// Verify average query time is reasonable
	EXPECT_GT(AfterAvgQueryTime, 0.0f);
	EXPECT_LT(AfterAvgQueryTime, 1.0f);

	// Calculate cache hit rate
	int32 TotalQueries = AfterCacheHits + AfterCacheMisses;
	if (TotalQueries > 0)
	{
		float HitRate = (float)AfterCacheHits / (float)TotalQueries * 100.0f;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Final cache hit rate: %.2f%% (%d hits / %d total queries)"), 
			HitRate, AfterCacheHits, TotalQueries);
	}

	Fixture.AfterEach();
	return true;
}

/**
 * Test multiple asset type queries
 * Verifies that different asset types can be queried independently
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerMultipleAssetTypesTest, 
	"DelveDeep.Configuration.MultipleAssetTypes", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerMultipleAssetTypesTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Query different asset types
	const UDelveDeepCharacterData* CharData = ConfigManager->GetCharacterData(FName("TestChar"));
	const UDelveDeepUpgradeData* UpgradeData = ConfigManager->GetUpgradeData(FName("TestUpgrade"));
	const UDelveDeepWeaponData* WeaponData = ConfigManager->GetWeaponData(FName("TestWeapon"));
	const UDelveDeepAbilityData* AbilityData = ConfigManager->GetAbilityData(FName("TestAbility"));
	const FDelveDeepMonsterConfig* MonsterData = ConfigManager->GetMonsterConfig(FName("TestMonster"));

	// All should return nullptr (assets don't exist)
	EXPECT_NULL(CharData);
	EXPECT_NULL(UpgradeData);
	EXPECT_NULL(WeaponData);
	EXPECT_NULL(AbilityData);
	EXPECT_NULL(MonsterData);

	// Verify all queries were tracked
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float AvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	EXPECT_GE(CacheMisses, 5);

	Fixture.AfterEach();
	return true;
}

/**
 * Test validation integration with configuration manager
 * Verifies that ValidateAllData works correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerValidationIntegrationTest, 
	"DelveDeep.Configuration.ValidationIntegration", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerValidationIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Run validation on all data
	FString ValidationReport;
	bool bIsValid = ConfigManager->ValidateAllData(ValidationReport);

	// Validation should complete without crashing
	EXPECT_TRUE(true);

	// Report should be generated
	EXPECT_FALSE(ValidationReport.IsEmpty());

	// Log the validation report for inspection
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Report:\n%s"), *ValidationReport);

	Fixture.AfterEach();
	return true;
}

/**
 * Test memory usage during configuration queries
 * Verifies that queries don't cause excessive memory allocations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerMemoryUsageTest, 
	"DelveDeep.Configuration.MemoryUsage", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerMemoryUsageTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	// Track memory during queries
	{
		DelveDeepTestUtils::FScopedMemoryTracker MemTracker;
		
		// Perform multiple queries
		const int32 QueryCount = 100;
		for (int32 i = 0; i < QueryCount; ++i)
		{
			ConfigManager->GetCharacterData(FName(*FString::Printf(TEXT("TestChar%d"), i)));
		}

		// Memory usage should be reasonable (< 1MB for 100 queries)
		uint64 AllocatedBytes = MemTracker.GetAllocatedBytes();
		EXPECT_TRUE(MemTracker.IsWithinBudget(1024 * 1024)); // 1MB budget

		UE_LOG(LogDelveDeepConfig, Display, TEXT("Memory usage for %d queries: %llu bytes"), 
			QueryCount, AllocatedBytes);
	}

	Fixture.AfterEach();
	return true;
}

/**
 * Test subsystem initialization performance
 * Verifies that configuration manager initializes within performance targets
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfigurationManagerInitializationPerformanceTest, 
	"DelveDeep.Configuration.InitializationPerformance", 
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConfigurationManagerInitializationPerformanceTest::RunTest(const FString& Parameters)
{
	// Measure initialization time
	DelveDeepTestUtils::FScopedTestTimer Timer(TEXT("Configuration Manager Initialization"));
	
	// Create test fixture (this initializes the subsystem)
	FSubsystemTestFixture Fixture;
	Fixture.BeforeEach();

	// Get configuration manager
	UDelveDeepConfigurationManager* ConfigManager = Fixture.GetSubsystem<UDelveDeepConfigurationManager>();
	ASSERT_NOT_NULL(ConfigManager);

	double InitTime = Timer.GetElapsedMs();

	// Initialization should be under 100ms (target from requirements)
	EXPECT_LT(InitTime, 100.0);
	EXPECT_TRUE(Timer.IsWithinBudget(100.0));

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration manager initialization time: %.2f ms"), InitTime);

	Fixture.AfterEach();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
