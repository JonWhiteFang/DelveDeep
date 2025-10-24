// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestUtilities.h"
#include "HAL/PlatformTime.h"
#include "Misc/EngineVersionComparison.h"

namespace DelveDeepTestUtils
{
	// ========================================
	// Game Instance and Subsystem Helpers
	// ========================================

	UGameInstance* CreateTestGameInstance()
	{
		UGameInstance* GameInstance = NewObject<UGameInstance>();
		if (GameInstance)
		{
			GameInstance->Init();
		}
		return GameInstance;
	}

	// ========================================
	// Mock Object Factories
	// ========================================

	UWorld* FMockWorld::Create()
	{
		// Create a minimal world for testing
		UWorld* World = NewObject<UWorld>();
		if (World)
		{
			World->WorldType = EWorldType::Game;
			World->InitializeActorsForPlay(FURL());
		}
		return World;
	}

	void FMockWorld::Destroy(UWorld* World)
	{
		if (World)
		{
			World->CleanupWorld();
			World->ConditionalBeginDestroy();
		}
	}

	UGameInstance* FMockGameInstance::Create()
	{
		return CreateTestGameInstance();
	}

	void FMockGameInstance::Destroy(UGameInstance* GameInstance)
	{
		if (GameInstance)
		{
			GameInstance->Shutdown();
			GameInstance->ConditionalBeginDestroy();
		}
	}

	// ========================================
	// Test Data Generators
	// ========================================

	FDelveDeepCharacterData CreateTestCharacterData(
		const FString& Name,
		float Health,
		float Damage)
	{
		FDelveDeepCharacterData Data;
		// Note: This is a placeholder. Actual implementation will depend on
		// the FDelveDeepCharacterData struct definition.
		// For now, we'll just return an empty struct.
		return Data;
	}

	FDelveDeepMonsterConfig CreateTestMonsterConfig(
		const FString& Name,
		float Health,
		float Damage)
	{
		FDelveDeepMonsterConfig Config;
		// Note: This is a placeholder. Actual implementation will depend on
		// the FDelveDeepMonsterConfig struct definition.
		// For now, we'll just return an empty struct.
		return Config;
	}

	FDelveDeepCharacterData CreateInvalidCharacterData()
	{
		FDelveDeepCharacterData Data;
		// Create data with invalid values for validation testing
		// Note: This is a placeholder. Actual implementation will set
		// invalid values based on the struct definition.
		return Data;
	}

	// ========================================
	// Validation Helpers
	// ========================================

	bool ValidateTestObject(UObject* Object, FValidationContext& Context)
	{
		Context.SystemName = TEXT("TestUtilities");
		Context.OperationName = TEXT("ValidateTestObject");

		if (!IsValid(Object))
		{
			Context.AddError(TEXT("Object is invalid or null"));
			return false;
		}

		return true;
	}

	// ========================================
	// Performance Measurement
	// ========================================

	FScopedTestTimer::FScopedTestTimer(const FString& TestName)
		: Name(TestName)
		, StartTime(FPlatformTime::Seconds())
	{
	}

	FScopedTestTimer::~FScopedTestTimer()
	{
		double ElapsedMs = GetElapsedMs();
		UE_LOG(LogTemp, Display, TEXT("Test '%s' completed in %.3f ms"), *Name, ElapsedMs);
	}

	double FScopedTestTimer::GetElapsedMs() const
	{
		double CurrentTime = FPlatformTime::Seconds();
		return (CurrentTime - StartTime) * 1000.0;
	}

	// ========================================
	// Memory Tracking
	// ========================================

	FScopedMemoryTracker::FScopedMemoryTracker()
		: StartMemory(0)
		, StartAllocations(0)
	{
		// Note: Actual memory tracking implementation would use
		// FMemory::GetAllocatorStats() or similar APIs
		// This is a placeholder implementation
	}

	FScopedMemoryTracker::~FScopedMemoryTracker()
	{
		uint64 AllocatedBytes = GetAllocatedBytes();
		int32 AllocationCount = GetAllocationCount();
		
		if (AllocatedBytes > 0)
		{
			UE_LOG(LogTemp, Display, TEXT("Memory allocated: %llu bytes (%d allocations)"),
				AllocatedBytes, AllocationCount);
		}
	}

	uint64 FScopedMemoryTracker::GetAllocatedBytes() const
	{
		// Note: Placeholder implementation
		// Actual implementation would calculate: CurrentMemory - StartMemory
		return 0;
	}

	int32 FScopedMemoryTracker::GetAllocationCount() const
	{
		// Note: Placeholder implementation
		// Actual implementation would calculate: CurrentAllocations - StartAllocations
		return 0;
	}

	// ========================================
	// Resource Cleanup
	// ========================================

	FScopedTestCleanup::FScopedTestCleanup(TFunction<void()> CleanupFunc)
		: Cleanup(CleanupFunc)
	{
	}

	FScopedTestCleanup::~FScopedTestCleanup()
	{
		if (Cleanup)
		{
			Cleanup();
		}
	}

	// ========================================
	// Timeout Protection
	// ========================================

	FTestTimeoutGuard::FTestTimeoutGuard(float TimeoutSeconds)
		: StartTime(FPlatformTime::Seconds())
		, Timeout(TimeoutSeconds)
	{
	}

	FTestTimeoutGuard::~FTestTimeoutGuard()
	{
		if (IsTimedOut())
		{
			UE_LOG(LogTemp, Error, TEXT("Test timed out after %.2f seconds"), Timeout);
		}
	}

	bool FTestTimeoutGuard::IsTimedOut() const
	{
		double CurrentTime = FPlatformTime::Seconds();
		double ElapsedTime = CurrentTime - StartTime;
		return ElapsedTime >= Timeout;
	}
}
