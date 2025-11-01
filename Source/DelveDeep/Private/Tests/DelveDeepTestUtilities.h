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

	// TODO: Enable when character system is implemented
	#if 0

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

	#endif // 0

	// ========================================
	// Validation Helpers
	// ========================================

	/**
	 * Validates a test object using FDelveDeepValidationContext.
	 * 
	 * @param Object The object to validate
	 * @param Context Validation context for error tracking
	 * @return True if validation succeeded, false otherwise
	 */
	DELVEDEEP_API bool ValidateTestObject(UObject* Object, FDelveDeepValidationContext& Context);

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
	DELVEDEEP_API bool TestExplicitValidation(UObject* Object, FDelveDeepValidationContext& Context);

	/**
	 * Verifies that a validation context contains expected error messages.
	 * 
	 * @param Context The validation context to check
	 * @param ExpectedErrors Array of expected error message substrings
	 * @return True if all expected errors are present
	 */
	DELVEDEEP_API bool VerifyValidationErrors(
		const FDelveDeepValidationContext& Context,
		const TArray<FString>& ExpectedErrors);

	/**
	 * Verifies that a validation context contains expected warning messages.
	 * 
	 * @param Context The validation context to check
	 * @param ExpectedWarnings Array of expected warning message substrings
	 * @return True if all expected warnings are present
	 */
	DELVEDEEP_API bool VerifyValidationWarnings(
		const FDelveDeepValidationContext& Context,
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
	// Blueprint Testing Utilities
	// ========================================

	/**
	 * Calls a Blueprint-callable function from C++ tests.
	 * 
	 * @param Object The object containing the function
	 * @param FunctionName Name of the Blueprint-callable function
	 * @param Params Optional parameters to pass to the function
	 * @return True if function was found and called successfully
	 */
	DELVEDEEP_API bool CallBlueprintFunction(
		UObject* Object,
		const FString& FunctionName,
		const TArray<FString>& Params = TArray<FString>());

	/**
	 * Reads a Blueprint-exposed property value.
	 * 
	 * @param Object The object containing the property
	 * @param PropertyName Name of the Blueprint-exposed property
	 * @param OutValue String representation of the property value
	 * @return True if property was found and read successfully
	 */
	DELVEDEEP_API bool ReadBlueprintProperty(
		UObject* Object,
		const FString& PropertyName,
		FString& OutValue);

	/**
	 * Writes a Blueprint-exposed property value.
	 * 
	 * @param Object The object containing the property
	 * @param PropertyName Name of the Blueprint-exposed property
	 * @param Value String representation of the value to set
	 * @return True if property was found and written successfully
	 */
	DELVEDEEP_API bool WriteBlueprintProperty(
		UObject* Object,
		const FString& PropertyName,
		const FString& Value);

	/**
	 * Triggers a Blueprint event and verifies it was broadcast.
	 * 
	 * @param Object The object that broadcasts the event
	 * @param EventName Name of the Blueprint event
	 * @param OutEventTriggered Set to true if event was triggered
	 * @return True if event mechanism was found
	 */
	DELVEDEEP_API bool TriggerBlueprintEvent(
		UObject* Object,
		const FString& EventName,
		bool& OutEventTriggered);

	/**
	 * Verifies that a Blueprint-callable function handles invalid inputs gracefully.
	 * 
	 * @param Object The object containing the function
	 * @param FunctionName Name of the Blueprint-callable function
	 * @param InvalidParams Array of invalid parameters to test
	 * @return True if function handled invalid inputs without crashing
	 */
	DELVEDEEP_API bool TestBlueprintFunctionInvalidInputs(
		UObject* Object,
		const FString& FunctionName,
		const TArray<FString>& InvalidParams);

	/**
	 * Tests a static Blueprint function library method.
	 * 
	 * @param LibraryClass The Blueprint function library class
	 * @param FunctionName Name of the static function
	 * @param Params Optional parameters to pass to the function
	 * @return True if function was found and called successfully
	 */
	DELVEDEEP_API bool TestBlueprintLibraryFunction(
		UClass* LibraryClass,
		const FString& FunctionName,
		const TArray<FString>& Params = TArray<FString>());

	// ========================================
	// Error Handling Testing Utilities
	// ========================================

	/**
	 * Simulates an error scenario for testing error handling.
	 * 
	 * @param ErrorType Type of error to simulate ("NullPointer", "InvalidData", "OutOfRange", etc.)
	 * @param Context Validation context to receive error messages
	 * @return True if error was simulated successfully
	 */
	DELVEDEEP_API bool SimulateErrorScenario(
		const FString& ErrorType,
		FDelveDeepValidationContext& Context);

	/**
	 * Verifies that an error was logged with the appropriate severity.
	 * 
	 * @param ExpectedMessage Expected error message substring
	 * @param ExpectedSeverity Expected log severity ("Error", "Warning", "Display")
	 * @param CapturedOutput Captured log output to search
	 * @return True if error was logged with correct severity
	 */
	DELVEDEEP_API bool VerifyErrorLogged(
		const FString& ExpectedMessage,
		const FString& ExpectedSeverity,
		const TArray<FString>& CapturedOutput);

	/**
	 * Tests error recovery by simulating an error and verifying recovery.
	 * 
	 * @param ErrorFunc Function that triggers an error
	 * @param RecoveryFunc Function that should recover from the error
	 * @param VerificationFunc Function that verifies successful recovery
	 * @return True if error recovery succeeded
	 */
	DELVEDEEP_API bool TestErrorRecovery(
		TFunction<void()> ErrorFunc,
		TFunction<void()> RecoveryFunc,
		TFunction<bool()> VerificationFunc);

	/**
	 * Verifies that validation error messages are clear and actionable.
	 * 
	 * @param Context Validation context containing error messages
	 * @param RequiredElements Array of required elements in error messages (e.g., "value", "expected")
	 * @return True if all error messages contain required elements
	 */
	DELVEDEEP_API bool VerifyValidationErrorQuality(
		const FDelveDeepValidationContext& Context,
		const TArray<FString>& RequiredElements);

	/**
	 * Tests error propagation through FDelveDeepValidationContext.
	 * 
	 * @param SourceContext Source validation context with errors
	 * @param TargetContext Target validation context that should receive errors
	 * @param PropagationFunc Function that propagates errors between contexts
	 * @return True if errors were propagated correctly
	 */
	DELVEDEEP_API bool TestErrorPropagation(
		const FDelveDeepValidationContext& SourceContext,
		FDelveDeepValidationContext& TargetContext,
		TFunction<void(const FDelveDeepValidationContext&, FDelveDeepValidationContext&)> PropagationFunc);

	// ========================================
	// Test Data Loading Utilities
	// ========================================

	/**
	 * Loads test data from a JSON file.
	 * 
	 * @param FilePath Path to the JSON file (relative to project directory)
	 * @param OutJsonObject Parsed JSON object
	 * @return True if file was loaded and parsed successfully
	 */
	DELVEDEEP_API bool LoadTestDataFromJSON(
		const FString& FilePath,
		TSharedPtr<FJsonObject>& OutJsonObject);

	/**
	 * Loads test data from a CSV file.
	 * 
	 * @param FilePath Path to the CSV file (relative to project directory)
	 * @param OutRows Array of rows, each row is an array of column values
	 * @param bHasHeader True if first row contains column headers
	 * @return True if file was loaded and parsed successfully
	 */
	DELVEDEEP_API bool LoadTestDataFromCSV(
		const FString& FilePath,
		TArray<TArray<FString>>& OutRows,
		bool bHasHeader = true);

	/**
	 * Creates a parameterized test dataset from JSON.
	 * 
	 * @param JsonObject JSON object containing test data
	 * @param DatasetName Name of the dataset array in JSON
	 * @param OutTestCases Array of test case data
	 * @return True if dataset was extracted successfully
	 */
	DELVEDEEP_API bool CreateParameterizedTestDataset(
		const TSharedPtr<FJsonObject>& JsonObject,
		const FString& DatasetName,
		TArray<TSharedPtr<FJsonObject>>& OutTestCases);

	/**
	 * Generates realistic test data matching production schemas.
	 * 
	 * @param SchemaType Type of schema to generate ("Character", "Monster", "Weapon", etc.)
	 * @param Count Number of test data instances to generate
	 * @param OutData Array of generated test data objects
	 * @return True if data was generated successfully
	 */
	DELVEDEEP_API bool GenerateRealisticTestData(
		const FString& SchemaType,
		int32 Count,
		TArray<UObject*>& OutData);

	/**
	 * Creates test data with varying sizes for scalability testing.
	 * 
	 * @param BaseData Base test data to scale
	 * @param Sizes Array of sizes to generate (e.g., [10, 100, 1000])
	 * @param OutDatasets Map of size to generated datasets
	 * @return True if datasets were generated successfully
	 */
	DELVEDEEP_API bool CreateScalabilityTestDatasets(
		UObject* BaseData,
		const TArray<int32>& Sizes,
		TMap<int32, TArray<UObject*>>& OutDatasets);

	/**
	 * Validates test data against a schema.
	 * 
	 * @param Data Test data to validate
	 * @param SchemaType Expected schema type
	 * @param Context Validation context for error tracking
	 * @return True if data matches schema
	 */
	DELVEDEEP_API bool ValidateTestDataSchema(
		UObject* Data,
		const FString& SchemaType,
		FDelveDeepValidationContext& Context);

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
