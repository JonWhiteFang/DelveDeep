// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTestUtilitiesCharacter.h"
#include "HAL/PlatformTime.h"
#include "GameplayTagsManager.h"

namespace DelveDeepTestUtils
{
	// ========================================
	// Character Creation Helpers
	// ========================================

	ADelveDeepCharacter* CreateTestCharacter(
		TSubclassOf<ADelveDeepCharacter> CharacterClass,
		float Health,
		float Resource)
	{
		if (!CharacterClass)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateTestCharacter: CharacterClass is null"));
			return nullptr;
		}

		// Create character instance
		ADelveDeepCharacter* Character = NewObject<ADelveDeepCharacter>(
			GetTransientPackage(),
			CharacterClass);

		if (!Character)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateTestCharacter: Failed to create character"));
			return nullptr;
		}

		// Set initial stats
		if (!SetCharacterStats(Character, Health, Resource))
		{
			UE_LOG(LogTemp, Error, TEXT("CreateTestCharacter: Failed to set character stats"));
			return nullptr;
		}

		return Character;
	}

	ADelveDeepWarrior* CreateTestWarrior(float Health, float Rage)
	{
		ADelveDeepWarrior* Warrior = NewObject<ADelveDeepWarrior>();
		
		if (Warrior)
		{
			SetCharacterStats(Warrior, Health, Rage);
		}

		return Warrior;
	}

	ADelveDeepRanger* CreateTestRanger(float Health, float Energy)
	{
		ADelveDeepRanger* Ranger = NewObject<ADelveDeepRanger>();
		
		if (Ranger)
		{
			SetCharacterStats(Ranger, Health, Energy);
		}

		return Ranger;
	}

	ADelveDeepMage* CreateTestMage(float Health, float Mana)
	{
		ADelveDeepMage* Mage = NewObject<ADelveDeepMage>();
		
		if (Mage)
		{
			SetCharacterStats(Mage, Health, Mana);
		}

		return Mage;
	}

	ADelveDeepNecromancer* CreateTestNecromancer(float Health, float Souls)
	{
		ADelveDeepNecromancer* Necromancer = NewObject<ADelveDeepNecromancer>();
		
		if (Necromancer)
		{
			SetCharacterStats(Necromancer, Health, Souls);
		}

		return Necromancer;
	}

	// ========================================
	// Character Stat Manipulation
	// ========================================

	bool SetCharacterStats(
		ADelveDeepCharacter* Character,
		float Health,
		float Resource,
		float Damage,
		float MoveSpeed)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SetCharacterStats: Character is null or invalid"));
			return false;
		}

		UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
		if (!IsValid(StatsComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("SetCharacterStats: StatsComponent is null or invalid"));
			return false;
		}

		// Set health
		StatsComponent->SetHealth(Health);

		// Set resource
		StatsComponent->SetResource(Resource);

		// Note: BaseDamage and MoveSpeed would need setter methods in StatsComponent
		// For now, we only set health and resource which have public setters

		return true;
	}

	bool SetCharacterHealth(ADelveDeepCharacter* Character, float Health)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SetCharacterHealth: Character is null or invalid"));
			return false;
		}

		UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
		if (!IsValid(StatsComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("SetCharacterHealth: StatsComponent is null or invalid"));
			return false;
		}

		StatsComponent->SetHealth(Health);
		return true;
	}

	bool SetCharacterResource(ADelveDeepCharacter* Character, float Resource)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SetCharacterResource: Character is null or invalid"));
			return false;
		}

		UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
		if (!IsValid(StatsComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("SetCharacterResource: StatsComponent is null or invalid"));
			return false;
		}

		StatsComponent->SetResource(Resource);
		return true;
	}

	bool VerifyCharacterStats(
		const ADelveDeepCharacter* Character,
		float ExpectedHealth,
		float ExpectedResource,
		float Tolerance)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterStats: Character is null or invalid"));
			return false;
		}

		float ActualHealth = Character->GetCurrentHealth();
		float ActualResource = Character->GetCurrentResource();

		bool bHealthMatches = FMath::IsNearlyEqual(ActualHealth, ExpectedHealth, Tolerance);
		bool bResourceMatches = FMath::IsNearlyEqual(ActualResource, ExpectedResource, Tolerance);

		if (!bHealthMatches)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterStats: Health mismatch. Expected: %.2f, Actual: %.2f"),
				ExpectedHealth, ActualHealth);
		}

		if (!bResourceMatches)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterStats: Resource mismatch. Expected: %.2f, Actual: %.2f"),
				ExpectedResource, ActualResource);
		}

		return bHealthMatches && bResourceMatches;
	}

	// ========================================
	// Damage and Healing Simulation
	// ========================================

	bool SimulateDamage(
		ADelveDeepCharacter* Character,
		float DamageAmount,
		AActor* DamageSource)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateDamage: Character is null or invalid"));
			return false;
		}

		if (DamageAmount < 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateDamage: DamageAmount is negative: %.2f"), DamageAmount);
			return false;
		}

		// Apply damage
		Character->TakeDamage(DamageAmount, DamageSource);

		return true;
	}

	bool SimulateLethalDamage(
		ADelveDeepCharacter* Character,
		AActor* DamageSource)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateLethalDamage: Character is null or invalid"));
			return false;
		}

		// Get current health and apply enough damage to kill
		float CurrentHealth = Character->GetCurrentHealth();
		float LethalDamage = CurrentHealth + 1.0f;

		Character->TakeDamage(LethalDamage, DamageSource);

		// Verify character is dead
		return Character->IsDead();
	}

	bool SimulateHealing(
		ADelveDeepCharacter* Character,
		float HealAmount)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateHealing: Character is null or invalid"));
			return false;
		}

		if (HealAmount < 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateHealing: HealAmount is negative: %.2f"), HealAmount);
			return false;
		}

		// Apply healing
		Character->Heal(HealAmount);

		return true;
	}

	bool SimulateFullHealing(ADelveDeepCharacter* Character)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateFullHealing: Character is null or invalid"));
			return false;
		}

		float CurrentHealth = Character->GetCurrentHealth();
		float MaxHealth = Character->GetMaxHealth();
		float HealAmount = MaxHealth - CurrentHealth;

		if (HealAmount > 0.0f)
		{
			Character->Heal(HealAmount);
		}

		// Verify character is at full health
		return VerifyCharacterAtFullHealth(Character);
	}

	bool VerifyDamageApplied(
		const ADelveDeepCharacter* Character,
		float InitialHealth,
		float DamageAmount,
		float Tolerance)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyDamageApplied: Character is null or invalid"));
			return false;
		}

		float CurrentHealth = Character->GetCurrentHealth();
		float ExpectedHealth = FMath::Max(0.0f, InitialHealth - DamageAmount);

		bool bMatches = FMath::IsNearlyEqual(CurrentHealth, ExpectedHealth, Tolerance);

		if (!bMatches)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyDamageApplied: Health mismatch. Expected: %.2f, Actual: %.2f"),
				ExpectedHealth, CurrentHealth);
		}

		return bMatches;
	}

	bool VerifyHealingApplied(
		const ADelveDeepCharacter* Character,
		float InitialHealth,
		float HealAmount,
		float Tolerance)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyHealingApplied: Character is null or invalid"));
			return false;
		}

		float CurrentHealth = Character->GetCurrentHealth();
		float MaxHealth = Character->GetMaxHealth();
		float ExpectedHealth = FMath::Min(MaxHealth, InitialHealth + HealAmount);

		bool bMatches = FMath::IsNearlyEqual(CurrentHealth, ExpectedHealth, Tolerance);

		if (!bMatches)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyHealingApplied: Health mismatch. Expected: %.2f, Actual: %.2f"),
				ExpectedHealth, CurrentHealth);
		}

		return bMatches;
	}

	// ========================================
	// Ability Testing
	// ========================================

	bool TriggerAbility(
		ADelveDeepCharacter* Character,
		int32 AbilityIndex)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerAbility: Character is null or invalid"));
			return false;
		}

		UDelveDeepAbilitiesComponent* AbilitiesComponent = Character->GetAbilitiesComponent();
		if (!IsValid(AbilitiesComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("TriggerAbility: AbilitiesComponent is null or invalid"));
			return false;
		}

		// Use the ability
		bool bSuccess = AbilitiesComponent->UseAbility(AbilityIndex);

		if (!bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("TriggerAbility: Failed to use ability at index %d"), AbilityIndex);
		}

		return bSuccess;
	}

	bool VerifyAbilityUsed(
		const ADelveDeepCharacter* Character,
		int32 AbilityIndex,
		float InitialResource,
		float ExpectedResourceCost,
		float Tolerance)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyAbilityUsed: Character is null or invalid"));
			return false;
		}

		float CurrentResource = Character->GetCurrentResource();
		float ExpectedResource = InitialResource - ExpectedResourceCost;

		bool bMatches = FMath::IsNearlyEqual(CurrentResource, ExpectedResource, Tolerance);

		if (!bMatches)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyAbilityUsed: Resource mismatch. Expected: %.2f, Actual: %.2f"),
				ExpectedResource, CurrentResource);
		}

		return bMatches;
	}

	bool SimulateAbilityCooldownComplete(
		ADelveDeepCharacter* Character,
		int32 AbilityIndex)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateAbilityCooldownComplete: Character is null or invalid"));
			return false;
		}

		UDelveDeepAbilitiesComponent* AbilitiesComponent = Character->GetAbilitiesComponent();
		if (!IsValid(AbilitiesComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("SimulateAbilityCooldownComplete: AbilitiesComponent is null or invalid"));
			return false;
		}

		// Note: This would require access to cooldown management in AbilitiesComponent
		// For now, we just verify the component exists
		// In a real implementation, we would manipulate the cooldown timer

		UE_LOG(LogTemp, Warning, TEXT("SimulateAbilityCooldownComplete: Cooldown simulation not fully implemented"));
		return true;
	}

	// ========================================
	// Mock Event Listener Implementation
	// ========================================

	FMockEventListener::FMockEventListener(
		UDelveDeepEventSubsystem* InEventSubsystem,
		FGameplayTag InEventTag)
		: EventSubsystem(InEventSubsystem)
		, ListenedEventTag(InEventTag)
		, LastEventTime(-1.0)
	{
		if (!IsValid(EventSubsystem))
		{
			UE_LOG(LogTemp, Error, TEXT("FMockEventListener: EventSubsystem is null or invalid"));
			return;
		}

		// Register listener
		ListenerHandle = EventSubsystem->RegisterListener(
			ListenedEventTag,
			[this](const FDelveDeepEventPayload& Payload)
			{
				OnEventReceived(Payload);
			},
			nullptr,  // No owner for test listener
			EDelveDeepEventPriority::Normal
		);

		UE_LOG(LogTemp, Display, TEXT("FMockEventListener: Registered for event tag: %s"),
			*ListenedEventTag.ToString());
	}

	FMockEventListener::~FMockEventListener()
	{
		if (IsValid(EventSubsystem) && ListenerHandle.IsValid())
		{
			EventSubsystem->UnregisterListener(ListenerHandle);
			UE_LOG(LogTemp, Display, TEXT("FMockEventListener: Unregistered listener"));
		}
	}

	const FDelveDeepEventPayload* FMockEventListener::GetLastEvent() const
	{
		if (CapturedEvents.Num() == 0)
		{
			return nullptr;
		}

		return CapturedEvents.Last().Get();
	}

	void FMockEventListener::ClearEvents()
	{
		CapturedEvents.Empty();
		LastEventTime = -1.0;
	}

	bool FMockEventListener::VerifyEventReceived(
		TFunction<bool(const FDelveDeepEventPayload&)> VerificationFunc) const
	{
		if (!VerificationFunc)
		{
			UE_LOG(LogTemp, Error, TEXT("FMockEventListener::VerifyEventReceived: VerificationFunc is null"));
			return false;
		}

		for (const TSharedPtr<FDelveDeepEventPayload>& Event : CapturedEvents)
		{
			if (Event.IsValid() && VerificationFunc(*Event))
			{
				return true;
			}
		}

		return false;
	}

	bool FMockEventListener::VerifyEventCount(int32 ExpectedCount) const
	{
		int32 ActualCount = CapturedEvents.Num();
		
		if (ActualCount != ExpectedCount)
		{
			UE_LOG(LogTemp, Error, TEXT("FMockEventListener::VerifyEventCount: Count mismatch. Expected: %d, Actual: %d"),
				ExpectedCount, ActualCount);
			return false;
		}

		return true;
	}

	bool FMockEventListener::VerifyEventOrder(
		const TArray<TFunction<bool(const FDelveDeepEventPayload&)>>& VerificationFuncs) const
	{
		if (VerificationFuncs.Num() != CapturedEvents.Num())
		{
			UE_LOG(LogTemp, Error, TEXT("FMockEventListener::VerifyEventOrder: Count mismatch. Expected: %d, Actual: %d"),
				VerificationFuncs.Num(), CapturedEvents.Num());
			return false;
		}

		for (int32 i = 0; i < VerificationFuncs.Num(); ++i)
		{
			if (!VerificationFuncs[i])
			{
				UE_LOG(LogTemp, Error, TEXT("FMockEventListener::VerifyEventOrder: VerificationFunc at index %d is null"), i);
				return false;
			}

			if (!CapturedEvents[i].IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("FMockEventListener::VerifyEventOrder: Event at index %d is invalid"), i);
				return false;
			}

			if (!VerificationFuncs[i](*CapturedEvents[i]))
			{
				UE_LOG(LogTemp, Error, TEXT("FMockEventListener::VerifyEventOrder: Event at index %d failed verification"), i);
				return false;
			}
		}

		return true;
	}

	double FMockEventListener::GetTimeSinceLastEvent() const
	{
		if (LastEventTime < 0.0)
		{
			return -1.0;
		}

		double CurrentTime = FPlatformTime::Seconds();
		return CurrentTime - LastEventTime;
	}

	void FMockEventListener::OnEventReceived(const FDelveDeepEventPayload& Payload)
	{
		// Create a copy of the payload
		TSharedPtr<FDelveDeepEventPayload> PayloadCopy = MakeShared<FDelveDeepEventPayload>(Payload);
		CapturedEvents.Add(PayloadCopy);

		// Update last event time
		LastEventTime = FPlatformTime::Seconds();

		UE_LOG(LogTemp, Display, TEXT("FMockEventListener: Event received. Total events: %d"),
			CapturedEvents.Num());
	}

	// ========================================
	// Character State Verification
	// ========================================

	bool VerifyCharacterAlive(const ADelveDeepCharacter* Character)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterAlive: Character is null or invalid"));
			return false;
		}

		bool bIsAlive = !Character->IsDead();

		if (!bIsAlive)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterAlive: Character is dead"));
		}

		return bIsAlive;
	}

	bool VerifyCharacterDead(const ADelveDeepCharacter* Character)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterDead: Character is null or invalid"));
			return false;
		}

		bool bIsDead = Character->IsDead();

		if (!bIsDead)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterDead: Character is alive"));
		}

		return bIsDead;
	}

	bool VerifyCharacterComponents(const ADelveDeepCharacter* Character)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterComponents: Character is null or invalid"));
			return false;
		}

		bool bHasStatsComponent = IsValid(Character->GetStatsComponent());
		bool bHasAbilitiesComponent = IsValid(Character->GetAbilitiesComponent());
		bool bHasEquipmentComponent = IsValid(Character->GetEquipmentComponent());

		if (!bHasStatsComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterComponents: StatsComponent is missing"));
		}

		if (!bHasAbilitiesComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterComponents: AbilitiesComponent is missing"));
		}

		if (!bHasEquipmentComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterComponents: EquipmentComponent is missing"));
		}

		return bHasStatsComponent && bHasAbilitiesComponent && bHasEquipmentComponent;
	}

	bool VerifyCharacterStatsValid(
		const ADelveDeepCharacter* Character,
		FValidationContext& Context)
	{
		Context.SystemName = TEXT("CharacterTestUtilities");
		Context.OperationName = TEXT("VerifyCharacterStatsValid");

		if (!IsValid(Character))
		{
			Context.AddError(TEXT("Character is null or invalid"));
			return false;
		}

		UDelveDeepStatsComponent* StatsComponent = Character->GetStatsComponent();
		if (!IsValid(StatsComponent))
		{
			Context.AddError(TEXT("StatsComponent is null or invalid"));
			return false;
		}

		// Verify health is within valid range
		float CurrentHealth = Character->GetCurrentHealth();
		float MaxHealth = Character->GetMaxHealth();

		if (CurrentHealth < 0.0f)
		{
			Context.AddError(FString::Printf(TEXT("CurrentHealth is negative: %.2f"), CurrentHealth));
		}

		if (CurrentHealth > MaxHealth)
		{
			Context.AddError(FString::Printf(TEXT("CurrentHealth (%.2f) exceeds MaxHealth (%.2f)"),
				CurrentHealth, MaxHealth));
		}

		if (MaxHealth <= 0.0f)
		{
			Context.AddError(FString::Printf(TEXT("MaxHealth is invalid: %.2f"), MaxHealth));
		}

		// Verify resource is within valid range
		float CurrentResource = Character->GetCurrentResource();
		float MaxResource = Character->GetMaxResource();

		if (CurrentResource < 0.0f)
		{
			Context.AddError(FString::Printf(TEXT("CurrentResource is negative: %.2f"), CurrentResource));
		}

		if (CurrentResource > MaxResource)
		{
			Context.AddError(FString::Printf(TEXT("CurrentResource (%.2f) exceeds MaxResource (%.2f)"),
				CurrentResource, MaxResource));
		}

		if (MaxResource < 0.0f)
		{
			Context.AddError(FString::Printf(TEXT("MaxResource is negative: %.2f"), MaxResource));
		}

		return Context.IsValid();
	}

	bool VerifyCharacterAtFullHealth(
		const ADelveDeepCharacter* Character,
		float Tolerance)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterAtFullHealth: Character is null or invalid"));
			return false;
		}

		float CurrentHealth = Character->GetCurrentHealth();
		float MaxHealth = Character->GetMaxHealth();

		bool bAtFullHealth = FMath::IsNearlyEqual(CurrentHealth, MaxHealth, Tolerance);

		if (!bAtFullHealth)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterAtFullHealth: Health not at maximum. Current: %.2f, Max: %.2f"),
				CurrentHealth, MaxHealth);
		}

		return bAtFullHealth;
	}

	bool VerifyCharacterAtFullResource(
		const ADelveDeepCharacter* Character,
		float Tolerance)
	{
		if (!IsValid(Character))
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterAtFullResource: Character is null or invalid"));
			return false;
		}

		float CurrentResource = Character->GetCurrentResource();
		float MaxResource = Character->GetMaxResource();

		bool bAtFullResource = FMath::IsNearlyEqual(CurrentResource, MaxResource, Tolerance);

		if (!bAtFullResource)
		{
			UE_LOG(LogTemp, Error, TEXT("VerifyCharacterAtFullResource: Resource not at maximum. Current: %.2f, Max: %.2f"),
				CurrentResource, MaxResource);
		}

		return bAtFullResource;
	}
}
