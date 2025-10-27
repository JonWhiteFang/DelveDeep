// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "DelveDeepValidation.h"
#include "DelveDeepTestReport.h"

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

	// ========================================
	// Console Command Testing
	// ========================================

	/**
	 * Console output capture helper for testing console commands.
	 * Captures log output during command execution for verification.
	 */
	class DELVEDEEP_API FConsoleOutputCapture
	{
	public:
		/**
		 * Constructs a console output capture and starts capturing.
		 */
		FConsoleOutputCapture();

		/**
		 * Destructor stops capturing.
		 */
		~FConsoleOutputCapture();

		/**
		 * Gets all captured output lines.
		 * 
		 * @return Array of captured log messages
		 */
		const TArray<FString>& GetCapturedOutput() const;

		/**
		 * Checks if the captured output contains a specific string.
		 * 
		 * @param SearchString String to search for
		 * @return True if found, false otherwise
		 */
		bool ContainsOutput(const FString& SearchString) const;

		/**
		 * Checks if the captured output contains all specified strings.
		 * 
		 * @param SearchStrings Array of strings to search for
		 * @return True if all found, false otherwise
		 */
		bool ContainsAllOutput(const TArray<FString>& SearchStrings) const;

		/**
		 * Gets the number of captured output lines.
		 * 
		 * @return Number of lines captured
		 */
		int32 GetOutputLineCount() const;

		/**
		 * Clears all captured output.
		 */
		void ClearOutput();

	private:
		TArray<FString> CapturedOutput;
		void* OutputDeviceHandle;  // Opaque handle to output device
	};

	/**
	 * Executes a console command programmatically.
	 * 
	 * @param Command The console command to execute (e.g., "DelveDeep.ValidateAllData")
	 * @return True if command was found and executed, false otherwise
	 */
	DELVEDEEP_API bool ExecuteConsoleCommand(const FString& Command);

	/**
	 * Executes a console command with arguments programmatically.
	 * 
	 * @param Command The console command to execute
	 * @param Args Array of arguments to pass to the command
	 * @return True if command was found and executed, false otherwise
	 */
	DELVEDEEP_API bool ExecuteConsoleCommandWithArgs(const FString& Command, const TArray<FString>& Args);

	/**
	 * Executes a console command and captures its output.
	 * 
	 * @param Command The console command to execute
	 * @param OutCapturedOutput Array to receive captured output lines
	 * @return True if command was found and executed, false otherwise
	 */
	DELVEDEEP_API bool ExecuteConsoleCommandWithCapture(
		const FString& Command,
		TArray<FString>& OutCapturedOutput);

	/**
	 * Verifies that a console command is registered.
	 * 
	 * @param CommandName The name of the command to check (e.g., "DelveDeep.ValidateAllData")
	 * @return True if command is registered, false otherwise
	 */
	DELVEDEEP_API bool IsConsoleCommandRegistered(const FString& CommandName);

	/**
	 * Gets all registered console commands matching a prefix.
	 * 
	 * @param Prefix Command prefix to search for (e.g., "DelveDeep.")
	 * @return Array of matching command names
	 */
	DELVEDEEP_API TArray<FString> GetRegisteredConsoleCommands(const FString& Prefix);

	/**
	 * Tests that a console command handles invalid parameters gracefully.
	 * 
	 * @param Command The console command to test
	 * @param InvalidArgs Array of invalid arguments to test
	 * @param OutCapturedOutput Array to receive captured output (should contain error messages)
	 * @return True if command handled invalid parameters without crashing
	 */
	DELVEDEEP_API bool TestConsoleCommandInvalidParameters(
		const FString& Command,
		const TArray<FString>& InvalidArgs,
		TArray<FString>& OutCapturedOutput);

	/**
	 * Verifies that a console command produces expected side effects.
	 * This is a helper that executes a command and allows custom verification.
	 * 
	 * @param Command The console command to execute
	 * @param VerificationFunc Function to call after command execution to verify side effects
	 * @return True if command executed and verification passed
	 */
	DELVEDEEP_API bool VerifyConsoleCommandSideEffects(
		const FString& Command,
		TFunction<bool()> VerificationFunc);

	/**
	 * Tests that all expected console commands are registered for a subsystem.
	 * 
	 * @param SubsystemName Name of the subsystem (e.g., "Validation", "Events")
	 * @param ExpectedCommands Array of expected command names
	 * @param OutMissingCommands Array to receive names of missing commands
	 * @return True if all expected commands are registered
	 */
	DELVEDEEP_API bool VerifySubsystemCommandsRegistered(
		const FString& SubsystemName,
		const TArray<FString>& ExpectedCommands,
		TArray<FString>& OutMissingCommands);
}
