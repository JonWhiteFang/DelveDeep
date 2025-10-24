// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestUtilities.h"
#include "HAL/PlatformTime.h"
#include "Misc/EngineVersionComparison.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"

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
			
			// Initialize world with minimal setup
			World->InitializeActorsForPlay(FURL());
			
			// Create timer manager for async testing
			if (!World->GetTimerManager().IsValid())
			{
				// Timer manager is typically created during world initialization
				// but we ensure it exists for testing
			}
		}
		return World;
	}

	void FMockWorld::Destroy(UWorld* World)
	{
		if (World && IsValid(World))
		{
			// Clean up world resources
			World->CleanupWorld();
			
			// Mark for garbage collection
			World->ConditionalBeginDestroy();
		}
	}

	UGameInstance* FMockGameInstance::Create()
	{
		// Create game instance with full initialization
		UGameInstance* GameInstance = CreateTestGameInstance();
		
		if (GameInstance)
		{
			// Subsystems are automatically initialized via Init()
			// No additional setup needed for basic testing
		}
		
		return GameInstance;
	}

	void FMockGameInstance::Destroy(UGameInstance* GameInstance)
	{
		if (GameInstance && IsValid(GameInstance))
		{
			// Shutdown subsystems and cleanup
			GameInstance->Shutdown();
			
			// Mark for garbage collection
			GameInstance->ConditionalBeginDestroy();
		}
	}

	// ========================================
	// Test Data Generators
	// ========================================

	UDelveDeepCharacterData* CreateTestCharacterData(
		const FString& Name,
		float Health,
		float Damage)
	{
		UDelveDeepCharacterData* Data = NewObject<UDelveDeepCharacterData>();
		if (Data)
		{
			Data->CharacterName = FText::FromString(Name);
			Data->Description = FText::FromString(FString::Printf(TEXT("Test character: %s"), *Name));
			Data->BaseHealth = Health;
			Data->BaseDamage = Damage;
			Data->MoveSpeed = 300.0f;
			Data->BaseArmor = 0.0f;
			Data->MaxResource = 100.0f;
			Data->ResourceRegenRate = 5.0f;
			Data->BaseAttackSpeed = 1.0f;
			Data->AttackRange = 100.0f;
		}
		return Data;
	}

	FDelveDeepMonsterConfig CreateTestMonsterConfig(
		const FString& Name,
		float Health,
		float Damage)
	{
		FDelveDeepMonsterConfig Config;
		Config.MonsterName = FText::FromString(Name);
		Config.Description = FText::FromString(FString::Printf(TEXT("Test monster: %s"), *Name));
		Config.Health = Health;
		Config.Damage = Damage;
		Config.MoveSpeed = 200.0f;
		Config.Armor = 0.0f;
		Config.DetectionRange = 500.0f;
		Config.AttackRange = 100.0f;
		Config.AIBehaviorType = "Melee";
		Config.CoinDropMin = 1;
		Config.CoinDropMax = 5;
		Config.ExperienceReward = 10;
		return Config;
	}

	UDelveDeepWeaponData* CreateTestWeaponData(
		const FString& Name,
		float Damage,
		float AttackSpeed)
	{
		UDelveDeepWeaponData* Data = NewObject<UDelveDeepWeaponData>();
		if (Data)
		{
			Data->WeaponName = FText::FromString(Name);
			Data->Description = FText::FromString(FString::Printf(TEXT("Test weapon: %s"), *Name));
			Data->BaseDamage = Damage;
			Data->AttackSpeed = AttackSpeed;
			Data->Range = 100.0f;
			Data->DamageType = "Physical";
			Data->ProjectileSpeed = 0.0f;
			Data->bPiercing = false;
			Data->MaxPierceTargets = 1;
		}
		return Data;
	}

	UDelveDeepAbilityData* CreateTestAbilityData(
		const FString& Name,
		float Cooldown,
		float ResourceCost)
	{
		UDelveDeepAbilityData* Data = NewObject<UDelveDeepAbilityData>();
		if (Data)
		{
			Data->AbilityName = FText::FromString(Name);
			Data->Description = FText::FromString(FString::Printf(TEXT("Test ability: %s"), *Name));
			Data->Cooldown = Cooldown;
			Data->CastTime = 0.0f;
			Data->Duration = 0.0f;
			Data->ResourceCost = ResourceCost;
			Data->DamageMultiplier = 1.0f;
			Data->DamageType = "Physical";
			Data->AoERadius = 0.0f;
			Data->bAffectsAllies = false;
		}
		return Data;
	}

	UDelveDeepCharacterData* CreateInvalidCharacterData()
	{
		UDelveDeepCharacterData* Data = NewObject<UDelveDeepCharacterData>();
		if (Data)
		{
			// Set invalid values to trigger validation errors
			Data->CharacterName = FText::FromString(TEXT("InvalidCharacter"));
			Data->Description = FText::FromString(TEXT("Character with invalid data for testing"));
			Data->BaseHealth = -10.0f;  // Invalid: negative health
			Data->BaseDamage = -5.0f;   // Invalid: negative damage
			Data->MoveSpeed = 0.0f;     // Invalid: zero move speed
			Data->BaseArmor = -1.0f;    // Invalid: negative armor
			Data->MaxResource = -50.0f; // Invalid: negative resource
			Data->ResourceRegenRate = -1.0f; // Invalid: negative regen
			Data->BaseAttackSpeed = 0.0f;    // Invalid: zero attack speed
			Data->AttackRange = 0.0f;        // Invalid: zero range
		}
		return Data;
	}

	FDelveDeepMonsterConfig CreateInvalidMonsterConfig()
	{
		FDelveDeepMonsterConfig Config;
		// Set invalid values to trigger validation errors
		Config.MonsterName = FText::FromString(TEXT("InvalidMonster"));
		Config.Description = FText::FromString(TEXT("Monster with invalid data for testing"));
		Config.Health = -20.0f;      // Invalid: negative health
		Config.Damage = -10.0f;      // Invalid: negative damage
		Config.MoveSpeed = -50.0f;   // Invalid: negative speed
		Config.Armor = -5.0f;        // Invalid: negative armor
		Config.DetectionRange = 0.0f; // Invalid: zero detection range
		Config.AttackRange = 0.0f;    // Invalid: zero attack range
		Config.CoinDropMin = -1;      // Invalid: negative coins
		Config.CoinDropMax = -5;      // Invalid: negative coins
		Config.ExperienceReward = -10; // Invalid: negative experience
		return Config;
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
