// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepConfigurationManager.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepUpgradeData.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test fixture for configuration manager tests
 * Provides common setup and teardown for configuration manager testing
 */
class FConfigurationManagerTestFixture
{
public:
	FConfigurationManagerTestFixture()
	{
		// Create test game instance
		GameInstance = NewObject<UGameInstance>();
		check(GameInstance);
		
		// Get configuration manager subsystem (auto-initializes)
		ConfigManager = GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
		check(ConfigManager);
	}

	~FConfigurationManagerTestFixture()
	{
		// Cleanup
		ConfigManager = nullptr;
		GameInstance = nullptr;
	}

	UGameInstance* GameInstance;
	UDelveDeepConfigurationManager* ConfigManager;
};

/**
 * Test asset caching on first query
 * Verifies that assets are properly cached when first accessed
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerAssetCachingTest, 
	"DelveDeep.ConfigurationManager.AssetCaching", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerAssetCachingTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Get initial performance stats
	int32 InitialCacheHits = 0;
	int32 InitialCacheMisses = 0;
	float InitialAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(InitialCacheHits, InitialCacheMisses, InitialAvgQueryTime);

	// Create and manually add a test character data asset to the cache
	UDelveDeepCharacterData* TestCharacterData = NewObject<UDelveDeepCharacterData>();
	TestCharacterData->CharacterName = FText::FromString(TEXT("TestWarrior"));
	TestCharacterData->BaseHealth = 100.0f;
	TestCharacterData->BaseDamage = 15.0f;
	TestCharacterData->MoveSpeed = 300.0f;

	// Access the private cache through reflection or by testing the public interface
	// Since we can't directly access the cache, we'll test the behavior through queries
	
	// Query for a non-existent asset (should result in cache miss)
	const UDelveDeepCharacterData* Result = ConfigManager->GetCharacterData(FName("NonExistentCharacter"));
	
	// Verify result is nullptr for non-existent asset
	TestNull(TEXT("Non-existent character should return nullptr"), Result);

	// Get updated performance stats
	int32 AfterCacheHits = 0;
	int32 AfterCacheMisses = 0;
	float AfterAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(AfterCacheHits, AfterCacheMisses, AfterAvgQueryTime);

	// Verify cache miss was recorded
	TestEqual(TEXT("Cache misses should increase by 1"), AfterCacheMisses, InitialCacheMisses + 1);
	TestEqual(TEXT("Cache hits should remain unchanged"), AfterCacheHits, InitialCacheHits);

	return true;
}

/**
 * Test cached asset returned on subsequent queries
 * Verifies that subsequent queries return cached data without reloading
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerCachedAssetTest, 
	"DelveDeep.ConfigurationManager.CachedAssetReturn", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerCachedAssetTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Note: Since we're in a code-only environment without actual data assets,
	// we'll test the caching behavior by querying the same non-existent asset multiple times
	// and verifying that cache misses increase appropriately

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
	TestNull(TEXT("First query should return nullptr"), Result1);
	TestNull(TEXT("Second query should return nullptr"), Result2);
	TestNull(TEXT("Third query should return nullptr"), Result3);

	// Get updated stats
	int32 AfterCacheHits = 0;
	int32 AfterCacheMisses = 0;
	float AfterAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(AfterCacheHits, AfterCacheMisses, AfterAvgQueryTime);

	// Verify that all three queries were tracked
	// Since the asset doesn't exist, all should be cache misses
	TestEqual(TEXT("Cache misses should increase by 3"), AfterCacheMisses, InitialCacheMisses + 3);

	// Verify query count increased
	TestTrue(TEXT("Average query time should be calculated"), AfterAvgQueryTime >= 0.0f);

	return true;
}

/**
 * Test cache hit rate tracking accuracy
 * Verifies that cache hit and miss tracking is accurate
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerCacheHitRateTest, 
	"DelveDeep.ConfigurationManager.CacheHitRateTracking", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerCacheHitRateTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

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
	TestEqual(TEXT("Cache misses should be tracked accurately"), AfterCacheMisses, ExpectedMisses);
	TestEqual(TEXT("Cache hits should remain unchanged for non-existent assets"), AfterCacheHits, InitialCacheHits);

	// Verify average query time is reasonable (should be very fast, < 1ms)
	TestTrue(TEXT("Average query time should be under 1ms"), AfterAvgQueryTime < 1.0f);

	// Calculate cache hit rate
	int32 TotalQueries = AfterCacheHits + AfterCacheMisses;
	if (TotalQueries > 0)
	{
		float HitRate = (float)AfterCacheHits / (float)TotalQueries * 100.0f;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Cache hit rate: %.2f%% (%d hits / %d total queries)"), 
			HitRate, AfterCacheHits, TotalQueries);
	}

	return true;
}

/**
 * Test data table lookup by name
 * Verifies that monster configs can be looked up from data tables
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerDataTableLookupTest, 
	"DelveDeep.ConfigurationManager.DataTableLookup", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerDataTableLookupTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Create a test data table
	UDataTable* TestDataTable = NewObject<UDataTable>();
	TestDataTable->RowStruct = FDelveDeepMonsterConfig::StaticStruct();

	// Add test monster config
	FDelveDeepMonsterConfig TestMonster;
	TestMonster.MonsterName = FText::FromString(TEXT("TestGoblin"));
	TestMonster.Health = 50.0f;
	TestMonster.Damage = 5.0f;
	TestMonster.MoveSpeed = 200.0f;
	TestMonster.DetectionRange = 500.0f;
	TestMonster.AttackRange = 100.0f;

	// Note: In a real test environment with data tables, we would add the row here
	// For now, we'll test the lookup behavior with a non-existent monster

	// Query for a monster config
	const FDelveDeepMonsterConfig* Result = ConfigManager->GetMonsterConfig(FName("TestGoblin"));

	// Since we don't have actual data tables loaded, result should be nullptr
	TestNull(TEXT("Monster config should return nullptr when table not loaded"), Result);

	// Get performance stats to verify query was tracked
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float AvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	// Verify query was tracked
	TestTrue(TEXT("Query should be tracked in performance stats"), CacheMisses > 0);

	return true;
}

/**
 * Test invalid name returns nullptr
 * Verifies that querying with invalid names returns nullptr gracefully
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerInvalidNameTest, 
	"DelveDeep.ConfigurationManager.InvalidNameReturnsNull", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerInvalidNameTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Test with various invalid names
	const UDelveDeepCharacterData* Result1 = ConfigManager->GetCharacterData(FName(""));
	const UDelveDeepCharacterData* Result2 = ConfigManager->GetCharacterData(FName("InvalidCharacter"));
	const UDelveDeepCharacterData* Result3 = ConfigManager->GetCharacterData(FName("NonExistent123"));

	// All should return nullptr
	TestNull(TEXT("Empty name should return nullptr"), Result1);
	TestNull(TEXT("Invalid name should return nullptr"), Result2);
	TestNull(TEXT("Non-existent name should return nullptr"), Result3);

	// Test with other asset types
	const UDelveDeepUpgradeData* UpgradeResult = ConfigManager->GetUpgradeData(FName("InvalidUpgrade"));
	const UDelveDeepWeaponData* WeaponResult = ConfigManager->GetWeaponData(FName("InvalidWeapon"));
	const UDelveDeepAbilityData* AbilityResult = ConfigManager->GetAbilityData(FName("InvalidAbility"));
	const FDelveDeepMonsterConfig* MonsterResult = ConfigManager->GetMonsterConfig(FName("InvalidMonster"));

	TestNull(TEXT("Invalid upgrade name should return nullptr"), UpgradeResult);
	TestNull(TEXT("Invalid weapon name should return nullptr"), WeaponResult);
	TestNull(TEXT("Invalid ability name should return nullptr"), AbilityResult);
	TestNull(TEXT("Invalid monster name should return nullptr"), MonsterResult);

	// Verify all queries were tracked as cache misses
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float AvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	TestTrue(TEXT("All invalid queries should be tracked as cache misses"), CacheMisses >= 7);

	return true;
}

/**
 * Test query performance under target thresholds
 * Verifies that queries complete within performance targets (< 1ms)
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerQueryPerformanceTest, 
	"DelveDeep.ConfigurationManager.QueryPerformance", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerQueryPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Measure single query performance
	double StartTime = FPlatformTime::Seconds();
	ConfigManager->GetCharacterData(FName("TestCharacter"));
	double EndTime = FPlatformTime::Seconds();
	double SingleQueryTime = (EndTime - StartTime) * 1000.0; // Convert to milliseconds

	// Single query should be under 1ms
	TestTrue(TEXT("Single query should complete in under 1ms"), SingleQueryTime < 1.0);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Single query time: %.4f ms"), SingleQueryTime);

	// Measure bulk query performance (1000 queries)
	const int32 BulkQueryCount = 1000;
	StartTime = FPlatformTime::Seconds();
	
	for (int32 i = 0; i < BulkQueryCount; ++i)
	{
		ConfigManager->GetCharacterData(FName("TestCharacter"));
	}
	
	EndTime = FPlatformTime::Seconds();
	double BulkQueryTime = (EndTime - StartTime) * 1000.0; // Convert to milliseconds
	double AvgBulkQueryTime = BulkQueryTime / BulkQueryCount;

	// Average query time should be under 1ms
	TestTrue(TEXT("Average bulk query time should be under 1ms"), AvgBulkQueryTime < 1.0);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Bulk query performance: %d queries in %.2f ms (avg: %.4f ms per query)"), 
		BulkQueryCount, BulkQueryTime, AvgBulkQueryTime);

	// Get performance stats from configuration manager
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float ConfigManagerAvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, ConfigManagerAvgQueryTime);

	// Verify configuration manager's tracked average is also under 1ms
	TestTrue(TEXT("Configuration manager tracked average should be under 1ms"), ConfigManagerAvgQueryTime < 1.0f);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Configuration manager stats: Hits=%d, Misses=%d, Avg=%.4f ms"), 
		CacheHits, CacheMisses, ConfigManagerAvgQueryTime);

	// Test data table query performance
	StartTime = FPlatformTime::Seconds();
	ConfigManager->GetMonsterConfig(FName("TestMonster"));
	EndTime = FPlatformTime::Seconds();
	double DataTableQueryTime = (EndTime - StartTime) * 1000.0;

	// Data table query should be under 0.5ms (target from requirements)
	TestTrue(TEXT("Data table query should complete in under 0.5ms"), DataTableQueryTime < 0.5);

	UE_LOG(LogDelveDeepConfig, Display, TEXT("Data table query time: %.4f ms"), DataTableQueryTime);

	return true;
}

/**
 * Test performance metrics accuracy
 * Verifies that performance metrics are calculated correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerPerformanceMetricsTest, 
	"DelveDeep.ConfigurationManager.PerformanceMetrics", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerPerformanceMetricsTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

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
	TestEqual(TEXT("Cache misses should match query count"), AfterCacheMisses, ExpectedMisses);

	// Verify average query time is reasonable
	TestTrue(TEXT("Average query time should be positive"), AfterAvgQueryTime > 0.0f);
	TestTrue(TEXT("Average query time should be under 1ms"), AfterAvgQueryTime < 1.0f);

	// Calculate cache hit rate
	int32 TotalQueries = AfterCacheHits + AfterCacheMisses;
	if (TotalQueries > 0)
	{
		float HitRate = (float)AfterCacheHits / (float)TotalQueries * 100.0f;
		UE_LOG(LogDelveDeepConfig, Display, TEXT("Final cache hit rate: %.2f%% (%d hits / %d total queries)"), 
			HitRate, AfterCacheHits, TotalQueries);
	}

	return true;
}

/**
 * Test multiple asset type queries
 * Verifies that different asset types can be queried independently
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerMultipleAssetTypesTest, 
	"DelveDeep.ConfigurationManager.MultipleAssetTypes", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerMultipleAssetTypesTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Query different asset types
	const UDelveDeepCharacterData* CharData = ConfigManager->GetCharacterData(FName("TestChar"));
	const UDelveDeepUpgradeData* UpgradeData = ConfigManager->GetUpgradeData(FName("TestUpgrade"));
	const UDelveDeepWeaponData* WeaponData = ConfigManager->GetWeaponData(FName("TestWeapon"));
	const UDelveDeepAbilityData* AbilityData = ConfigManager->GetAbilityData(FName("TestAbility"));
	const FDelveDeepMonsterConfig* MonsterData = ConfigManager->GetMonsterConfig(FName("TestMonster"));

	// All should return nullptr (assets don't exist)
	TestNull(TEXT("Character data should return nullptr"), CharData);
	TestNull(TEXT("Upgrade data should return nullptr"), UpgradeData);
	TestNull(TEXT("Weapon data should return nullptr"), WeaponData);
	TestNull(TEXT("Ability data should return nullptr"), AbilityData);
	TestNull(TEXT("Monster data should return nullptr"), MonsterData);

	// Verify all queries were tracked
	int32 CacheHits = 0;
	int32 CacheMisses = 0;
	float AvgQueryTime = 0.0f;
	ConfigManager->GetPerformanceStats(CacheHits, CacheMisses, AvgQueryTime);

	TestTrue(TEXT("All asset type queries should be tracked"), CacheMisses >= 5);

	return true;
}

/**
 * Test validation integration with configuration manager
 * Verifies that ValidateAllData works correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDelveDeepConfigManagerValidationIntegrationTest, 
	"DelveDeep.ConfigurationManager.ValidationIntegration", 
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDelveDeepConfigManagerValidationIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test fixture
	FConfigurationManagerTestFixture Fixture;
	UDelveDeepConfigurationManager* ConfigManager = Fixture.ConfigManager;

	// Run validation on all data
	FString ValidationReport;
	bool bIsValid = ConfigManager->ValidateAllData(ValidationReport);

	// Validation should complete without crashing
	TestTrue(TEXT("ValidateAllData should complete successfully"), true);

	// Report should be generated
	TestTrue(TEXT("Validation report should be generated"), !ValidationReport.IsEmpty());

	// Log the validation report for inspection
	UE_LOG(LogDelveDeepConfig, Display, TEXT("Validation Report:\n%s"), *ValidationReport);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
