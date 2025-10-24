// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "DelveDeepValidation.h"

/**
 * DelveDeep Test Utilities
 * 
 * Helper functions for common testing operations including:
 * - UObject creation and management
 * - Subsystem testing utilities
 * - Mock object factories
 * - Test data generators
 * - Performance measurement
 * - Memory tracking
 */

namespace DelveDeepTestUtils
{
	// ========================================
	// UObject Creation Helpers
	// ========================================

	/**
	 * Creates a test UObject of the specified type.
	 * The object is created with the transient package as outer.
	 * 
	 * @return Newly created test object
	 */
	template<typename T>
	T* CreateTestObject()
	{
		return NewObject<T>();
	}

	/**
	 * Creates a test UObject with a specific outer.
	 * 
	 * @param Outer The outer object for the new object
	 * @return Newly created test object
	 */
	template<typename T>
	T* CreateTestObjectWithOuter(UObject* Outer)
	{
		if (!Outer)
		{
			return nullptr;
		}
		return NewObject<T>(Outer);
	}

	/**
	 * Creates a test data asset of the specified type.
	 * 
	 * @return Newly created test data asset
	 */
	template<typename T>
	T* CreateTestDataAsset()
	{
		return NewObject<T>();
	}

	// ========================================
	// Game Instance and Subsystem Helpers
	// ========================================

	/**
	 * Creates a test game instance for subsystem testing.
	 * 
	 * @return Newly created test game instance
	 */
	DELVEDEEP_API UGameInstance* CreateTestGameInstance();

	/**
	 * Gets a subsystem from a game instance.
	 * 
	 * @param GameInstance The game instance to get the subsystem from
	 * @return The subsystem instance, or nullptr if not found
	 */
	template<typename T>
	T* GetTestSubsystem(UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			return nullptr;
		}
		return GameInstance->GetSubsystem<T>();
	}

	// ========================================
	// Mock Object Factories
	// ========================================

	/**
	 * Factory for creating mock UWorld instances for testing.
	 */
	class DELVEDEEP_API FMockWorld
	{
	public:
		/**
		 * Creates a minimal mock world for testing.
		 * 
		 * @return Newly created mock world
		 */
		static UWorld* Create();

		/**
		 * Destroys a mock world and cleans up resources.
		 * 
		 * @param World The world to destroy
		 */
		static void Destroy(UWorld* World);
	};

	/**
	 * Factory for creating mock UGameInstance instances for testing.
	 */
	class DELVEDEEP_API FMockGameInstance
	{
	public:
		/**
		 * Creates a minimal mock game instance for testing.
		 * 
		 * @return Newly created mock game instance
		 */
		static UGameInstance* Create();

		/**
		 * Destroys a mock game instance and cleans up resources.
		 * 
		 * @param GameInstance The game instance to destroy
		 */
		static void Destroy(UGameInstance* GameInstance);
	};

	// ========================================
	// Test Data Generators
	// ========================================

	/**
	 * Creates test character data asset with default or specified values.
	 * 
	 * @param Name Character name
	 * @param Health Base health value
	 * @param Damage Base damage value
	 * @return Test character data asset
	 */
	DELVEDEEP_API UDelveDeepCharacterData* CreateTestCharacterData(
		const FString& Name = TEXT("TestCharacter"),
		float Health = 100.0f,
		float Damage = 10.0f);

	/**
	 * Creates test monster configuration with default or specified values.
	 * 
	 * @param Name Monster name
	 * @param Health Monster health value
	 * @param Damage Monster damage value
	 * @return Test monster configuration struct
	 */
	DELVEDEEP_API struct FDelveDeepMonsterConfig CreateTestMonsterConfig(
		const FString& Name,
		float Health = 50.0f,
		float Damage = 5.0f);

	/**
	 * Creates test weapon data asset with default or specified values.
	 * 
	 * @param Name Weapon name
	 * @param Damage Base damage value
	 * @param AttackSpeed Attack speed value
	 * @return Test weapon data asset
	 */
	DELVEDEEP_API UDelveDeepWeaponData* CreateTestWeaponData(
		const FString& Name = TEXT("TestWeapon"),
		float Damage = 10.0f,
		float AttackSpeed = 1.0f);

	/**
	 * Creates test ability data asset with default or specified values.
	 * 
	 * @param Name Ability name
	 * @param Cooldown Cooldown duration
	 * @param ResourceCost Resource cost
	 * @return Test ability data asset
	 */
	DELVEDEEP_API UDelveDeepAbilityData* CreateTestAbilityData(
		const FString& Name = TEXT("TestAbility"),
		float Cooldown = 5.0f,
		float ResourceCost = 10.0f);

	/**
	 * Creates invalid character data for validation testing.
	 * Sets values outside valid ranges to trigger validation errors.
	 * 
	 * @return Test character data with invalid values
	 */
	DELVEDEEP_API UDelveDeepCharacterData* CreateInvalidCharacterData();

	/**
	 * Creates invalid monster config for validation testing.
	 * Sets values outside valid ranges to trigger validation errors.
	 * 
	 * @return Test monster config with invalid values
	 */
	DELVEDEEP_API FDelveDeepMonsterConfig CreateInvalidMonsterConfig();

	// ========================================
	// Validation Helpers
	// ========================================

	/**
	 * Validates a test object using FValidationContext.
	 * 
	 * @param Object The object to validate
	 * @param Context Validation context for error tracking
	 * @return True if validation succeeded, false otherwise
	 */
	DELVEDEEP_API bool ValidateTestObject(UObject* Object, FValidationContext& Context);

	/**
	 * Tests PostLoad validation for a data asset.
	 * Creates the asset, calls PostLoad(), and checks for validation errors.
	 * 
	 * @param DataAsset The data asset to test
	 * @return True if PostLoad validation executed correctly
	 */
	DELVEDEEP_API bool TestPostLoadValidation(UDataAsset* DataAsset);

	/**
	 * Tests explicit Validate() call on an object.
	 * 
	 * @param Object The object to validate
	 * @param Context Validation context for error tracking
	 * @return True if validation executed correctly
	 */
	DELVEDEEP_API bool TestExplicitValidation(UObject* Object, FValidationContext& Context);

	/**
	 * Verifies that a validation context contains expected error messages.
	 * 
	 * @param Context The validation context to check
	 * @param ExpectedErrors Array of expected error message substrings
	 * @return True if all expected errors are present
	 */
	DELVEDEEP_API bool VerifyValidationErrors(
		const FValidationContext& Context,
		const TArray<FString>& ExpectedErrors);

	/**
	 * Verifies that a validation context contains expected warning messages.
	 * 
	 * @param Context The validation context to check
	 * @param ExpectedWarnings Array of expected warning message substrings
	 * @return True if all expected warnings are present
	 */
	DELVEDEEP_API bool VerifyValidationWarnings(
		const FValidationContext& Context,
		const TArray<FString>& ExpectedWarnings);

	// ========================================
	// Performance Measurement
	// ========================================

	/**
	 * RAII-style timer for measuring test execution time.
	 * Automatically starts timing on construction and stops on destruction.
	 * Supports multiple iterations for stable performance measurements.
	 */
	class DELVEDEEP_API FScopedTestTimer
	{
	public:
		/**
		 * Constructs a timer and starts timing.
		 * 
		 * @param TestName Name of the test being timed (for logging)
		 */
		explicit FScopedTestTimer(const FString& TestName);

		/**
		 * Destructor stops timing and logs results.
		 */
		~FScopedTestTimer();

		/**
		 * Gets the elapsed time in milliseconds.
		 * 
		 * @return Elapsed time in milliseconds
		 */
		double GetElapsedMs() const;

		/**
		 * Gets the elapsed time in microseconds for high precision.
		 * 
		 * @return Elapsed time in microseconds
		 */
		double GetElapsedUs() const;

		/**
		 * Records a timing sample for statistical analysis.
		 * Call this after each iteration in a performance test.
		 */
		void RecordSample();

		/**
		 * Gets the minimum recorded time in milliseconds.
		 * 
		 * @return Minimum time across all samples
		 */
		double GetMinMs() const;

		/**
		 * Gets the maximum recorded time in milliseconds.
		 * 
		 * @return Maximum time across all samples
		 */
		double GetMaxMs() const;

		/**
		 * Gets the average recorded time in milliseconds.
		 * 
		 * @return Average time across all samples
		 */
		double GetAverageMs() const;

		/**
		 * Gets the median recorded time in milliseconds.
		 * 
		 * @return Median time across all samples
		 */
		double GetMedianMs() const;

		/**
		 * Validates that the average time is within the specified budget.
		 * 
		 * @param BudgetMs Maximum allowed average time in milliseconds
		 * @return True if within budget, false otherwise
		 */
		bool IsWithinBudget(double BudgetMs) const;

		/**
		 * Gets the number of recorded samples.
		 * 
		 * @return Number of samples
		 */
		int32 GetSampleCount() const;

	private:
		FString Name;
		double StartTime;
		double LastSampleTime;
		TArray<double> Samples;  // Recorded times in milliseconds
	};

	// ========================================
	// Memory Tracking
	// ========================================

	/**
	 * RAII-style memory tracker for detecting memory leaks and measuring allocations.
	 * Tracks memory usage from construction to destruction.
	 * Supports both native and managed memory tracking.
	 */
	class DELVEDEEP_API FScopedMemoryTracker
	{
	public:
		/**
		 * Constructs a memory tracker and records initial memory state.
		 */
		FScopedMemoryTracker();

		/**
		 * Destructor records final memory state and logs results.
		 */
		~FScopedMemoryTracker();

		/**
		 * Gets the number of bytes allocated since construction.
		 * 
		 * @return Bytes allocated
		 */
		uint64 GetAllocatedBytes() const;

		/**
		 * Gets the number of allocations since construction.
		 * 
		 * @return Allocation count
		 */
		int32 GetAllocationCount() const;

		/**
		 * Checks if memory was leaked (allocated but not freed).
		 * 
		 * @return True if memory leak detected, false otherwise
		 */
		bool HasMemoryLeak() const;

		/**
		 * Validates that memory usage is within the specified budget.
		 * 
		 * @param BudgetBytes Maximum allowed memory allocation in bytes
		 * @return True if within budget, false otherwise
		 */
		bool IsWithinBudget(uint64 BudgetBytes) const;

		/**
		 * Gets the peak memory usage since construction.
		 * 
		 * @return Peak memory usage in bytes
		 */
		uint64 GetPeakBytes() const;

	private:
		uint64 StartMemory;
		int32 StartAllocations;
		uint64 PeakMemory;
		bool bTrackingEnabled;
	};

	// ========================================
	// Resource Cleanup
	// ========================================

	/**
	 * RAII-style cleanup helper for automatic resource cleanup.
	 * Executes cleanup function on scope exit.
	 */
	class DELVEDEEP_API FScopedTestCleanup
	{
	public:
		/**
		 * Constructs a cleanup helper with a cleanup function.
		 * 
		 * @param CleanupFunc Function to call on destruction
		 */
		explicit FScopedTestCleanup(TFunction<void()> CleanupFunc);

		/**
		 * Destructor executes the cleanup function.
		 */
		~FScopedTestCleanup();

	private:
		TFunction<void()> Cleanup;
	};

	// ========================================
	// Timeout Protection
	// ========================================

	/**
	 * Timeout guard for async test operations.
	 * Tracks elapsed time and detects timeouts.
	 */
	class DELVEDEEP_API FTestTimeoutGuard
	{
	public:
		/**
		 * Constructs a timeout guard with specified timeout.
		 * 
		 * @param TimeoutSeconds Maximum allowed execution time
		 */
		explicit FTestTimeoutGuard(float TimeoutSeconds);

		/**
		 * Destructor logs timeout if it occurred.
		 */
		~FTestTimeoutGuard();

		/**
		 * Checks if the timeout has been exceeded.
		 * 
		 * @return True if timed out, false otherwise
		 */
		bool IsTimedOut() const;

	private:
		double StartTime;
		float Timeout;
	};
}
