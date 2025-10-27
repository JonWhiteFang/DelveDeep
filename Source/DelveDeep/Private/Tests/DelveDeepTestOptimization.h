// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * DelveDeep Test Execution Optimization
 * 
 * Utilities for optimizing test execution including:
 * - Parallel test execution configuration
 * - Test execution time tracking
 * - Test object creation optimization
 * - Test result caching
 */

/**
 * Test execution statistics for a single test.
 */
struct DELVEDEEP_API FTestExecutionStats
{
	/** Test name */
	FString TestName;

	/** Execution time in milliseconds */
	double ExecutionTimeMs;

	/** Number of times this test has been executed */
	int32 ExecutionCount;

	/** Average execution time across all runs */
	double AverageExecutionTimeMs;

	/** Whether this test passed on last execution */
	bool bLastPassed;

	/** Timestamp of last execution */
	FDateTime LastExecutionTime;

	FTestExecutionStats()
		: ExecutionTimeMs(0.0)
		, ExecutionCount(0)
		, AverageExecutionTimeMs(0.0)
		, bLastPassed(false)
	{
	}
};

/**
 * Test result cache entry.
 * Stores test results to avoid re-running unchanged tests.
 */
struct DELVEDEEP_API FTestResultCacheEntry
{
	/** Test name */
	FString TestName;

	/** Hash of test code (for detecting changes) */
	uint32 CodeHash;

	/** Whether test passed */
	bool bPassed;

	/** Execution time in milliseconds */
	double ExecutionTimeMs;

	/** Timestamp when result was cached */
	FDateTime CacheTime;

	/** Test output/errors */
	TArray<FString> Output;

	FTestResultCacheEntry()
		: CodeHash(0)
		, bPassed(false)
		, ExecutionTimeMs(0.0)
	{
	}
};

/**
 * Test execution optimizer.
 * Provides utilities for optimizing test execution performance.
 */
class DELVEDEEP_API FTestExecutionOptimizer
{
public:
	/**
	 * Gets the singleton instance.
	 * 
	 * @return Optimizer instance
	 */
	static FTestExecutionOptimizer& Get();

	/**
	 * Records test execution statistics.
	 * 
	 * @param TestName Name of the test
	 * @param ExecutionTimeMs Execution time in milliseconds
	 * @param bPassed Whether the test passed
	 */
	void RecordTestExecution(const FString& TestName, double ExecutionTimeMs, bool bPassed);

	/**
	 * Gets execution statistics for a test.
	 * 
	 * @param TestName Name of the test
	 * @return Execution statistics, or nullptr if not found
	 */
	const FTestExecutionStats* GetTestStats(const FString& TestName) const;

	/**
	 * Gets all execution statistics.
	 * 
	 * @return Map of test name to statistics
	 */
	const TMap<FString, FTestExecutionStats>& GetAllStats() const { return ExecutionStats; }

	/**
	 * Gets the slowest tests.
	 * 
	 * @param Count Number of tests to return
	 * @return Array of test names sorted by execution time (slowest first)
	 */
	TArray<FString> GetSlowestTests(int32 Count = 10) const;

	/**
	 * Gets the fastest tests.
	 * 
	 * @param Count Number of tests to return
	 * @return Array of test names sorted by execution time (fastest first)
	 */
	TArray<FString> GetFastestTests(int32 Count = 10) const;

	/**
	 * Gets total execution time for all tests.
	 * 
	 * @return Total time in milliseconds
	 */
	double GetTotalExecutionTime() const;

	/**
	 * Exports execution statistics to CSV.
	 * 
	 * @param OutputPath Path to output file (relative to project directory)
	 * @return True if export succeeded
	 */
	bool ExportStatsToCSV(const FString& OutputPath) const;

	/**
	 * Caches a test result.
	 * 
	 * @param TestName Name of the test
	 * @param CodeHash Hash of test code
	 * @param bPassed Whether test passed
	 * @param ExecutionTimeMs Execution time
	 * @param Output Test output
	 */
	void CacheTestResult(
		const FString& TestName,
		uint32 CodeHash,
		bool bPassed,
		double ExecutionTimeMs,
		const TArray<FString>& Output);

	/**
	 * Gets a cached test result.
	 * 
	 * @param TestName Name of the test
	 * @param CodeHash Hash of test code
	 * @return Cached result, or nullptr if not found or outdated
	 */
	const FTestResultCacheEntry* GetCachedResult(const FString& TestName, uint32 CodeHash) const;

	/**
	 * Checks if a test result is cached and valid.
	 * 
	 * @param TestName Name of the test
	 * @param CodeHash Hash of test code
	 * @return True if cached result is valid
	 */
	bool HasValidCachedResult(const FString& TestName, uint32 CodeHash) const;

	/**
	 * Clears all cached results.
	 */
	void ClearCache();

	/**
	 * Clears cached results older than specified time.
	 * 
	 * @param MaxAge Maximum age in seconds
	 */
	void ClearOldCache(double MaxAgeSeconds = 3600.0);

	/**
	 * Saves cache to disk.
	 * 
	 * @param FilePath Path to cache file (relative to project directory)
	 * @return True if save succeeded
	 */
	bool SaveCache(const FString& FilePath) const;

	/**
	 * Loads cache from disk.
	 * 
	 * @param FilePath Path to cache file (relative to project directory)
	 * @return True if load succeeded
	 */
	bool LoadCache(const FString& FilePath);

	/**
	 * Resets all statistics.
	 */
	void Reset();

private:
	/** Singleton instance */
	static FTestExecutionOptimizer* Instance;

	/** Execution statistics by test name */
	TMap<FString, FTestExecutionStats> ExecutionStats;

	/** Cached test results */
	TMap<FString, FTestResultCacheEntry> ResultCache;

	/** Private constructor for singleton */
	FTestExecutionOptimizer() = default;
};

/**
 * Helper macros for test optimization.
 */

/**
 * Declares a test with proper flags for parallel execution.
 * Use this for unit tests that can run in parallel.
 */
#define IMPLEMENT_DELVEDEEP_UNIT_TEST(TestName, TestPath) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, TestPath, \
		EAutomationTestFlags::ApplicationContextMask | \
		EAutomationTestFlags::ProductFilter)

/**
 * Declares a test with proper flags for integration testing.
 * Integration tests run sequentially to avoid conflicts.
 */
#define IMPLEMENT_DELVEDEEP_INTEGRATION_TEST(TestName, TestPath) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, TestPath, \
		EAutomationTestFlags::ApplicationContextMask | \
		EAutomationTestFlags::ProductFilter | \
		EAutomationTestFlags::NonNullRHI)

/**
 * Declares a test with proper flags for performance testing.
 * Performance tests run sequentially with high priority.
 */
#define IMPLEMENT_DELVEDEEP_PERFORMANCE_TEST(TestName, TestPath) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestName, TestPath, \
		EAutomationTestFlags::ApplicationContextMask | \
		EAutomationTestFlags::PerfFilter | \
		EAutomationTestFlags::HighPriority)

/**
 * Declares a complex test with fixture support and parallel execution.
 */
#define IMPLEMENT_DELVEDEEP_COMPLEX_UNIT_TEST(TestName, TestPath) \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestName, TestPath, \
		EAutomationTestFlags::ApplicationContextMask | \
		EAutomationTestFlags::ProductFilter)

/**
 * Declares a complex integration test with fixture support.
 */
#define IMPLEMENT_DELVEDEEP_COMPLEX_INTEGRATION_TEST(TestName, TestPath) \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestName, TestPath, \
		EAutomationTestFlags::ApplicationContextMask | \
		EAutomationTestFlags::ProductFilter | \
		EAutomationTestFlags::NonNullRHI)

/**
 * RAII helper for tracking test execution time.
 * Automatically records execution statistics on destruction.
 */
class DELVEDEEP_API FScopedTestExecutionTracker
{
public:
	/**
	 * Constructs a tracker and starts timing.
	 * 
	 * @param TestName Name of the test being tracked
	 */
	explicit FScopedTestExecutionTracker(const FString& TestName);

	/**
	 * Destructor records execution time.
	 */
	~FScopedTestExecutionTracker();

	/**
	 * Marks the test as passed or failed.
	 * 
	 * @param bPassed Whether the test passed
	 */
	void SetPassed(bool bPassed);

private:
	FString TestName;
	double StartTime;
	bool bPassed;
};

/**
 * Object pool for test objects.
 * Reuses objects across tests to reduce allocation overhead.
 */
template<typename T>
class DELVEDEEP_API TTestObjectPool
{
public:
	/**
	 * Gets an object from the pool or creates a new one.
	 * 
	 * @return Pooled object
	 */
	T* Acquire()
	{
		if (AvailableObjects.Num() > 0)
		{
			T* Object = AvailableObjects.Pop();
			ActiveObjects.Add(Object);
			return Object;
		}

		// Create new object
		T* Object = NewObject<T>();
		ActiveObjects.Add(Object);
		TotalCreated++;
		return Object;
	}

	/**
	 * Returns an object to the pool.
	 * 
	 * @param Object Object to return
	 */
	void Release(T* Object)
	{
		if (!Object)
		{
			return;
		}

		ActiveObjects.Remove(Object);
		AvailableObjects.Add(Object);
	}

	/**
	 * Releases all active objects back to the pool.
	 */
	void ReleaseAll()
	{
		for (T* Object : ActiveObjects)
		{
			AvailableObjects.Add(Object);
		}
		ActiveObjects.Empty();
	}

	/**
	 * Clears the pool and destroys all objects.
	 */
	void Clear()
	{
		for (T* Object : AvailableObjects)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
		AvailableObjects.Empty();

		for (T* Object : ActiveObjects)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
		ActiveObjects.Empty();

		TotalCreated = 0;
	}

	/**
	 * Gets the number of objects in the pool.
	 * 
	 * @return Number of available objects
	 */
	int32 GetAvailableCount() const { return AvailableObjects.Num(); }

	/**
	 * Gets the number of active objects.
	 * 
	 * @return Number of active objects
	 */
	int32 GetActiveCount() const { return ActiveObjects.Num(); }

	/**
	 * Gets the total number of objects created.
	 * 
	 * @return Total created
	 */
	int32 GetTotalCreated() const { return TotalCreated; }

private:
	TArray<T*> AvailableObjects;
	TArray<T*> ActiveObjects;
	int32 TotalCreated = 0;
};
