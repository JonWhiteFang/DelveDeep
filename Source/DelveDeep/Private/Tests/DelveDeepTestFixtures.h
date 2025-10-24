// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepTestUtilities.h"
#include "DelveDeepConfigurationManager.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepTelemetrySubsystem.h"

/**
 * DelveDeep Test Fixtures
 * 
 * Base classes for organizing tests with setup and teardown.
 * Fixtures provide common initialization and cleanup logic for related tests.
 * 
 * Available Fixtures:
 * - FDelveDeepTestFixture: Base fixture with object tracking
 * - FSubsystemTestFixture: Fixture for testing subsystems
 * - FIntegrationTestFixture: Fixture for multi-subsystem integration tests
 */

// ========================================
// Base Test Fixture
// ========================================

/**
 * Base test fixture providing setup/teardown and object tracking.
 * All test fixtures should inherit from this class.
 * 
 * Usage:
 *   class FMyTestFixture : public FDelveDeepTestFixture
 *   {
 *   public:
 *       virtual void BeforeEach() override
 *       {
 *           FDelveDeepTestFixture::BeforeEach();
 *           // Custom setup
 *       }
 *   };
 */
class DELVEDEEP_API FDelveDeepTestFixture
{
public:
	virtual ~FDelveDeepTestFixture() = default;

	/**
	 * Called before each test.
	 * Override to add custom setup logic.
	 */
	virtual void BeforeEach()
	{
		// Base implementation does nothing
		// Override in derived classes for custom setup
	}

	/**
	 * Called after each test.
	 * Override to add custom teardown logic.
	 * Always call parent implementation to ensure cleanup.
	 */
	virtual void AfterEach()
	{
		CleanupTestObjects();
	}

protected:
	/**
	 * Cleans up all tracked test objects.
	 * Called automatically in AfterEach().
	 */
	void CleanupTestObjects()
	{
		for (UObject* Object : TestObjects)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
		TestObjects.Empty();
	}

	/**
	 * Creates and tracks a test object.
	 * The object will be automatically cleaned up in AfterEach().
	 * 
	 * @return Newly created and tracked object
	 */
	template<typename T>
	T* CreateAndTrackObject()
	{
		T* Object = NewObject<T>();
		if (Object)
		{
			TestObjects.Add(Object);
		}
		return Object;
	}

	/**
	 * Creates and tracks a test object with a specific outer.
	 * 
	 * @param Outer The outer object for the new object
	 * @return Newly created and tracked object
	 */
	template<typename T>
	T* CreateAndTrackObjectWithOuter(UObject* Outer)
	{
		T* Object = NewObject<T>(Outer);
		if (Object)
		{
			TestObjects.Add(Object);
		}
		return Object;
	}

	/**
	 * Array of test objects to clean up.
	 * Objects are automatically destroyed in AfterEach().
	 */
	TArray<UObject*> TestObjects;
};

// ========================================
// Subsystem Test Fixture
// ========================================

/**
 * Test fixture for testing subsystems.
 * Creates a test game instance and provides subsystem access.
 * 
 * Usage:
 *   class FMySubsystemTest : public FSubsystemTestFixture
 *   {
 *   public:
 *       virtual void BeforeEach() override
 *       {
 *           FSubsystemTestFixture::BeforeEach();
 *           MySubsystem = GetSubsystem<UMySubsystem>();
 *       }
 *   };
 */
class DELVEDEEP_API FSubsystemTestFixture : public FDelveDeepTestFixture
{
public:
	/**
	 * Creates test game instance and initializes subsystems.
	 */
	virtual void BeforeEach() override
	{
		FDelveDeepTestFixture::BeforeEach();
		
		// Create test game instance
		GameInstance = DelveDeepTestUtils::CreateTestGameInstance();
		
		if (!GameInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create test game instance"));
		}
	}

	/**
	 * Shuts down game instance and cleans up.
	 */
	virtual void AfterEach() override
	{
		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance->ConditionalBeginDestroy();
			GameInstance = nullptr;
		}
		
		FDelveDeepTestFixture::AfterEach();
	}

protected:
	/**
	 * Gets a subsystem from the test game instance.
	 * 
	 * @return The subsystem instance, or nullptr if not found
	 */
	template<typename T>
	T* GetSubsystem()
	{
		return DelveDeepTestUtils::GetTestSubsystem<T>(GameInstance);
	}

	/**
	 * Verifies that a subsystem is initialized.
	 * 
	 * @param Subsystem The subsystem to verify
	 * @return True if subsystem is valid, false otherwise
	 */
	template<typename T>
	bool VerifySubsystemInitialized(T* Subsystem)
	{
		if (!Subsystem)
		{
			UE_LOG(LogTemp, Error, TEXT("Subsystem %s is not initialized"), *T::StaticClass()->GetName());
			return false;
		}
		return true;
	}

	/**
	 * Test game instance for subsystem testing.
	 */
	UGameInstance* GameInstance = nullptr;
};

// ========================================
// Integration Test Fixture
// ========================================

/**
 * Test fixture for integration tests involving multiple subsystems.
 * Initializes all major subsystems and provides convenient access.
 * 
 * Usage:
 *   class FMyIntegrationTest : public FIntegrationTestFixture
 *   {
 *   public:
 *       void RunTest()
 *       {
 *           // ConfigManager, EventSubsystem, TelemetrySubsystem are available
 *           ConfigManager->GetCharacterData("Warrior");
 *           EventSubsystem->BroadcastEvent(Payload);
 *       }
 *   };
 */
class DELVEDEEP_API FIntegrationTestFixture : public FSubsystemTestFixture
{
public:
	/**
	 * Initializes all major subsystems for integration testing.
	 */
	virtual void BeforeEach() override
	{
		FSubsystemTestFixture::BeforeEach();
		
		// Initialize all subsystems
		ConfigManager = GetSubsystem<UDelveDeepConfigurationManager>();
		EventSubsystem = GetSubsystem<UDelveDeepEventSubsystem>();
		TelemetrySubsystem = GetSubsystem<UDelveDeepTelemetrySubsystem>();
		
		// Verify subsystems initialized correctly
		if (!VerifySubsystemInitialized(ConfigManager))
		{
			UE_LOG(LogTemp, Error, TEXT("Configuration Manager failed to initialize"));
		}
		
		if (!VerifySubsystemInitialized(EventSubsystem))
		{
			UE_LOG(LogTemp, Error, TEXT("Event Subsystem failed to initialize"));
		}
		
		if (!VerifySubsystemInitialized(TelemetrySubsystem))
		{
			UE_LOG(LogTemp, Error, TEXT("Telemetry Subsystem failed to initialize"));
		}
	}

	/**
	 * Verifies that all subsystems are in a valid state.
	 * 
	 * @return True if all subsystems are valid, false otherwise
	 */
	bool VerifyAllSubsystems()
	{
		return ConfigManager != nullptr &&
		       EventSubsystem != nullptr &&
		       TelemetrySubsystem != nullptr;
	}

protected:
	/**
	 * Configuration manager subsystem.
	 */
	UDelveDeepConfigurationManager* ConfigManager = nullptr;

	/**
	 * Event subsystem.
	 */
	UDelveDeepEventSubsystem* EventSubsystem = nullptr;

	/**
	 * Telemetry subsystem.
	 */
	UDelveDeepTelemetrySubsystem* TelemetrySubsystem = nullptr;
};

// ========================================
// Helper Macros for Fixture-Based Tests
// ========================================

/**
 * Macro for defining a test with a fixture.
 * Automatically handles BeforeEach/AfterEach calls.
 * 
 * Usage:
 *   DEFINE_FIXTURE_TEST(FMyTestFixture, "DelveDeep.MySystem.MyTest")
 *   {
 *       // Test code here
 *       // Fixture members are available
 *   }
 */
#define DEFINE_FIXTURE_TEST(FixtureClass, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		F##FixtureClass##Test, \
		TestName, \
		EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter) \
	bool F##FixtureClass##Test::RunTest(const FString& Parameters) \
	{ \
		FixtureClass Fixture; \
		Fixture.BeforeEach(); \
		bool bTestResult = true;

/**
 * Macro for ending a fixture test definition.
 * Automatically calls AfterEach and returns result.
 */
#define END_FIXTURE_TEST() \
		Fixture.AfterEach(); \
		return bTestResult; \
	}
