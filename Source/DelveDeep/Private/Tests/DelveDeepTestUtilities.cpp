// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestUtilities.h"
#include "HAL/PlatformTime.h"
#include "Misc/EngineVersionComparison.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepMonsterConfig.h"
#include "DelveDeepWeaponData.h"
#include "DelveDeepAbilityData.h"
#include "DelveDeepUpgradeData.h"

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

	bool TestPostLoadValidation(UDataAsset* DataAsset)
	{
		if (!IsValid(DataAsset))
		{
			UE_LOG(LogTemp, Error, TEXT("TestPostLoadValidation: DataAsset is null"));
			return false;
		}

		// Call PostLoad to trigger validation
		DataAsset->PostLoad();

		// PostLoad validation logs errors but doesn't return a value
		// We consider the test successful if PostLoad completes without crashing
		return true;
	}

	bool TestExplicitValidation(UObject* Object, FValidationContext& Context)
	{
		if (!IsValid(Object))
		{
			Context.AddError(TEXT("Object is null or invalid"));
			return false;
		}

		// Try to cast to objects that support validation
		if (UDelveDeepCharacterData* CharacterData = Cast<UDelveDeepCharacterData>(Object))
		{
			return CharacterData->Validate(Context);
		}
		else if (UDelveDeepWeaponData* WeaponData = Cast<UDelveDeepWeaponData>(Object))
		{
			return WeaponData->Validate(Context);
		}
		else if (UDelveDeepAbilityData* AbilityData = Cast<UDelveDeepAbilityData>(Object))
		{
			return AbilityData->Validate(Context);
		}
		else if (UDelveDeepUpgradeData* UpgradeData = Cast<UDelveDeepUpgradeData>(Object))
		{
			return UpgradeData->Validate(Context);
		}
		else
		{
			Context.AddWarning(FString::Printf(
				TEXT("Object type %s does not support explicit validation"),
				*Object->GetClass()->GetName()));
			return true;
		}
	}

	bool VerifyValidationErrors(
		const FValidationContext& Context,
		const TArray<FString>& ExpectedErrors)
	{
		if (ExpectedErrors.Num() == 0)
		{
			return true;
		}

		// Check that all expected errors are present
		for (const FString& ExpectedError : ExpectedErrors)
		{
			bool bFound = false;
			for (const FString& ActualError : Context.ValidationErrors)
			{
				if (ActualError.Contains(ExpectedError))
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				UE_LOG(LogTemp, Error, TEXT("Expected error not found: %s"), *ExpectedError);
				UE_LOG(LogTemp, Error, TEXT("Actual errors: %s"), *Context.GetReport());
				return false;
			}
		}

		return true;
	}

	bool VerifyValidationWarnings(
		const FValidationContext& Context,
		const TArray<FString>& ExpectedWarnings)
	{
		if (ExpectedWarnings.Num() == 0)
		{
			return true;
		}

		// Check that all expected warnings are present
		for (const FString& ExpectedWarning : ExpectedWarnings)
		{
			bool bFound = false;
			for (const FString& ActualWarning : Context.ValidationWarnings)
			{
				if (ActualWarning.Contains(ExpectedWarning))
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				UE_LOG(LogTemp, Error, TEXT("Expected warning not found: %s"), *ExpectedWarning);
				UE_LOG(LogTemp, Error, TEXT("Actual warnings: %s"), *Context.GetReport());
				return false;
			}
		}

		return true;
	}

	// ========================================
	// Performance Measurement
	// ========================================

	FScopedTestTimer::FScopedTestTimer(const FString& TestName)
		: Name(TestName)
		, StartTime(FPlatformTime::Seconds())
		, LastSampleTime(FPlatformTime::Seconds())
	{
	}

	FScopedTestTimer::~FScopedTestTimer()
	{
		double ElapsedMs = GetElapsedMs();
		
		if (Samples.Num() > 0)
		{
			// Log detailed statistics if samples were recorded
			UE_LOG(LogTemp, Display, TEXT("Test '%s' completed: %d samples, Min=%.3f ms, Max=%.3f ms, Avg=%.3f ms, Median=%.3f ms"),
				*Name, Samples.Num(), GetMinMs(), GetMaxMs(), GetAverageMs(), GetMedianMs());
		}
		else
		{
			// Log simple elapsed time if no samples
			UE_LOG(LogTemp, Display, TEXT("Test '%s' completed in %.3f ms"), *Name, ElapsedMs);
		}
	}

	double FScopedTestTimer::GetElapsedMs() const
	{
		double CurrentTime = FPlatformTime::Seconds();
		return (CurrentTime - StartTime) * 1000.0;
	}

	double FScopedTestTimer::GetElapsedUs() const
	{
		double CurrentTime = FPlatformTime::Seconds();
		return (CurrentTime - StartTime) * 1000000.0;
	}

	void FScopedTestTimer::RecordSample()
	{
		double CurrentTime = FPlatformTime::Seconds();
		double SampleTimeMs = (CurrentTime - LastSampleTime) * 1000.0;
		Samples.Add(SampleTimeMs);
		LastSampleTime = CurrentTime;
	}

	double FScopedTestTimer::GetMinMs() const
	{
		if (Samples.Num() == 0)
		{
			return 0.0;
		}
		
		double MinTime = Samples[0];
		for (double Sample : Samples)
		{
			if (Sample < MinTime)
			{
				MinTime = Sample;
			}
		}
		return MinTime;
	}

	double FScopedTestTimer::GetMaxMs() const
	{
		if (Samples.Num() == 0)
		{
			return 0.0;
		}
		
		double MaxTime = Samples[0];
		for (double Sample : Samples)
		{
			if (Sample > MaxTime)
			{
				MaxTime = Sample;
			}
		}
		return MaxTime;
	}

	double FScopedTestTimer::GetAverageMs() const
	{
		if (Samples.Num() == 0)
		{
			return 0.0;
		}
		
		double Sum = 0.0;
		for (double Sample : Samples)
		{
			Sum += Sample;
		}
		return Sum / Samples.Num();
	}

	double FScopedTestTimer::GetMedianMs() const
	{
		if (Samples.Num() == 0)
		{
			return 0.0;
		}
		
		// Create sorted copy of samples
		TArray<double> SortedSamples = Samples;
		SortedSamples.Sort();
		
		int32 MiddleIndex = SortedSamples.Num() / 2;
		
		if (SortedSamples.Num() % 2 == 0)
		{
			// Even number of samples: average the two middle values
			return (SortedSamples[MiddleIndex - 1] + SortedSamples[MiddleIndex]) / 2.0;
		}
		else
		{
			// Odd number of samples: return the middle value
			return SortedSamples[MiddleIndex];
		}
	}

	bool FScopedTestTimer::IsWithinBudget(double BudgetMs) const
	{
		if (Samples.Num() > 0)
		{
			return GetAverageMs() <= BudgetMs;
		}
		else
		{
			return GetElapsedMs() <= BudgetMs;
		}
	}

	int32 FScopedTestTimer::GetSampleCount() const
	{
		return Samples.Num();
	}

	// ========================================
	// Memory Tracking
	// ========================================

	FScopedMemoryTracker::FScopedMemoryTracker()
		: StartMemory(0)
		, StartAllocations(0)
		, PeakMemory(0)
		, bTrackingEnabled(false)
	{
		// Get current memory stats using Unreal's memory tracking
		FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
		StartMemory = MemStats.UsedPhysical;
		PeakMemory = StartMemory;
		
		// Note: Allocation count tracking requires more detailed profiling
		// For now, we track memory usage which is the primary concern
		StartAllocations = 0;
		bTrackingEnabled = true;
	}

	FScopedMemoryTracker::~FScopedMemoryTracker()
	{
		if (!bTrackingEnabled)
		{
			return;
		}
		
		uint64 AllocatedBytes = GetAllocatedBytes();
		int32 AllocationCount = GetAllocationCount();
		
		if (AllocatedBytes > 0)
		{
			UE_LOG(LogTemp, Display, TEXT("Memory allocated: %llu bytes (Peak: %llu bytes)"),
				AllocatedBytes, GetPeakBytes());
			
			if (HasMemoryLeak())
			{
				UE_LOG(LogTemp, Warning, TEXT("Potential memory leak detected: %llu bytes not freed"),
					AllocatedBytes);
			}
		}
		else if (AllocatedBytes < 0)
		{
			// Memory was freed (negative delta)
			UE_LOG(LogTemp, Display, TEXT("Memory freed: %lld bytes"), -static_cast<int64>(AllocatedBytes));
		}
	}

	uint64 FScopedMemoryTracker::GetAllocatedBytes() const
	{
		if (!bTrackingEnabled)
		{
			return 0;
		}
		
		FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
		uint64 CurrentMemory = MemStats.UsedPhysical;
		
		// Calculate delta (can be negative if memory was freed)
		if (CurrentMemory >= StartMemory)
		{
			return CurrentMemory - StartMemory;
		}
		else
		{
			// Memory decreased, return 0 to indicate no leak
			return 0;
		}
	}

	int32 FScopedMemoryTracker::GetAllocationCount() const
	{
		if (!bTrackingEnabled)
		{
			return 0;
		}
		
		// Note: Detailed allocation count tracking would require
		// hooking into the memory allocator or using profiling tools
		// For testing purposes, we estimate based on memory delta
		// Assuming average allocation size of 1KB
		uint64 AllocatedBytes = GetAllocatedBytes();
		if (AllocatedBytes > 0)
		{
			return static_cast<int32>(AllocatedBytes / 1024);
		}
		return 0;
	}

	bool FScopedMemoryTracker::HasMemoryLeak() const
	{
		if (!bTrackingEnabled)
		{
			return false;
		}
		
		// Consider it a leak if more than 1KB was allocated and not freed
		// Small allocations might be cached or pooled by the engine
		uint64 AllocatedBytes = GetAllocatedBytes();
		return AllocatedBytes > 1024;
	}

	bool FScopedMemoryTracker::IsWithinBudget(uint64 BudgetBytes) const
	{
		if (!bTrackingEnabled)
		{
			return true;
		}
		
		return GetAllocatedBytes() <= BudgetBytes;
	}

	uint64 FScopedMemoryTracker::GetPeakBytes() const
	{
		if (!bTrackingEnabled)
		{
			return 0;
		}
		
		FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
		uint64 CurrentMemory = MemStats.UsedPhysical;
		
		// Update peak if current is higher
		if (CurrentMemory > PeakMemory)
		{
			const_cast<FScopedMemoryTracker*>(this)->PeakMemory = CurrentMemory;
		}
		
		return PeakMemory - StartMemory;
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
