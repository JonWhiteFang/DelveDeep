// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DelveDeepCharacter.h"
#include "DelveDeepWarrior.h"
#include "DelveDeepRanger.h"
#include "DelveDeepMage.h"
#include "DelveDeepNecromancer.h"
#include "DelveDeepStatsComponent.h"
#include "DelveDeepAbilitiesComponent.h"
#include "DelveDeepEquipmentComponent.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "DelveDeepValidation.h"

/**
 * Character-specific test utilities for DelveDeep.
 * 
 * Provides helper functions for:
 * - Creating test characters with custom stats
 * - Simulating damage and healing
 * - Triggering abilities
 * - Mocking event listeners for verification
 * 
 * Requirements: 19.1, 19.2, 19.3, 19.4, 19.5
 */

namespace DelveDeepTestUtils
{
	// ========================================
	// Character Creation Helpers
	// ========================================

	/**
	 * Creates a test character of the specified class.
	 * The character is fully initialized with all components.
	 * 
	 * @param CharacterClass The character class to create (Warrior, Ranger, Mage, Necromancer)
	 * @param Health Initial health value (default: 100.0f)
	 * @param Resource Initial resource value (default: 100.0f)
	 * @return Newly created test character
	 * 
	 * Requirement: 19.1 - Test utilities for creating test character instances
	 */
	DELVEDEEP_API ADelveDeepCharacter* CreateTestCharacter(
		TSubclassOf<ADelveDeepCharacter> CharacterClass,
		float Health = 100.0f,
		float Resource = 100.0f);

	/**
	 * Creates a test Warrior character.
	 * 
	 * @param Health Initial health value (default: 100.0f)
	 * @param Rage Initial rage value (default: 0.0f)
	 * @return Newly created test Warrior
	 * 
	 * Requirement: 19.1 - Test utilities for creating test character instances
	 */
	DELVEDEEP_API ADelveDeepWarrior* CreateTestWarrior(
		float Health = 100.0f,
		float Rage = 0.0f);

	/**
	 * Creates a test Ranger character.
	 * 
	 * @param Health Initial health value (default: 100.0f)
	 * @param Energy Initial energy value (default: 100.0f)
	 * @return Newly created test Ranger
	 * 
	 * Requirement: 19.1 - Test utilities for creating test character instances
	 */
	DELVEDEEP_API ADelveDeepRanger* CreateTestRanger(
		float Health = 100.0f,
		float Energy = 100.0f);

	/**
	 * Creates a test Mage character.
	 * 
	 * @param Health Initial health value (default: 100.0f)
	 * @param Mana Initial mana value (default: 100.0f)
	 * @return Newly created test Mage
	 * 
	 * Requirement: 19.1 - Test utilities for creating test character instances
	 */
	DELVEDEEP_API ADelveDeepMage* CreateTestMage(
		float Health = 100.0f,
		float Mana = 100.0f);

	/**
	 * Creates a test Necromancer character.
	 * 
	 * @param Health Initial health value (default: 100.0f)
	 * @param Souls Initial souls value (default: 0.0f)
	 * @return Newly created test Necromancer
	 * 
	 * Requirement: 19.1 - Test utilities for creating test character instances
	 */
	DELVEDEEP_API ADelveDeepNecromancer* CreateTestNecromancer(
		float Health = 100.0f,
		float Souls = 0.0f);

	// ========================================
	// Character Stat Manipulation
	// ========================================

	/**
	 * Sets character stats for testing purposes.
	 * Bypasses normal initialization and directly sets stat values.
	 * 
	 * @param Character The character to modify
	 * @param Health Health value to set
	 * @param Resource Resource value to set (Mana/Energy/Rage/Souls)
	 * @param Damage Base damage value to set
	 * @param MoveSpeed Move speed value to set
	 * @return True if stats were set successfully, false otherwise
	 * 
	 * Requirement: 19.2 - Utilities for setting and verifying stat values
	 */
	DELVEDEEP_API bool SetCharacterStats(
		ADelveDeepCharacter* Character,
		float Health,
		float Resource,
		float Damage = 10.0f,
		float MoveSpeed = 300.0f);

	/**
	 * Sets only health for a character.
	 * 
	 * @param Character The character to modify
	 * @param Health Health value to set
	 * @return True if health was set successfully, false otherwise
	 * 
	 * Requirement: 19.2 - Utilities for setting and verifying stat values
	 */
	DELVEDEEP_API bool SetCharacterHealth(
		ADelveDeepCharacter* Character,
		float Health);

	/**
	 * Sets only resource for a character.
	 * 
	 * @param Character The character to modify
	 * @param Resource Resource value to set
	 * @return True if resource was set successfully, false otherwise
	 * 
	 * Requirement: 19.2 - Utilities for setting and verifying stat values
	 */
	DELVEDEEP_API bool SetCharacterResource(
		ADelveDeepCharacter* Character,
		float Resource);

	/**
	 * Verifies that character stats match expected values.
	 * 
	 * @param Character The character to verify
	 * @param ExpectedHealth Expected health value
	 * @param ExpectedResource Expected resource value
	 * @param Tolerance Tolerance for floating point comparison (default: 0.01f)
	 * @return True if all stats match within tolerance, false otherwise
	 * 
	 * Requirement: 19.2 - Utilities for setting and verifying stat values
	 */
	DELVEDEEP_API bool VerifyCharacterStats(
		const ADelveDeepCharacter* Character,
		float ExpectedHealth,
		float ExpectedResource,
		float Tolerance = 0.01f);

	// ========================================
	// Damage and Healing Simulation
	// ========================================

	/**
	 * Simulates damage to a character for testing.
	 * Applies damage and verifies the character's response.
	 * 
	 * @param Character The character to damage
	 * @param DamageAmount Amount of damage to apply
	 * @param DamageSource Optional damage source actor
	 * @return True if damage was applied successfully, false otherwise
	 * 
	 * Requirement: 19.3 - Utilities for simulating damage and healing
	 */
	DELVEDEEP_API bool SimulateDamage(
		ADelveDeepCharacter* Character,
		float DamageAmount,
		AActor* DamageSource = nullptr);

	/**
	 * Simulates lethal damage to a character.
	 * Applies enough damage to kill the character.
	 * 
	 * @param Character The character to kill
	 * @param DamageSource Optional damage source actor
	 * @return True if character died, false otherwise
	 * 
	 * Requirement: 19.3 - Utilities for simulating damage and healing
	 */
	DELVEDEEP_API bool SimulateLethalDamage(
		ADelveDeepCharacter* Character,
		AActor* DamageSource = nullptr);

	/**
	 * Simulates healing for a character.
	 * Applies healing and verifies the character's response.
	 * 
	 * @param Character The character to heal
	 * @param HealAmount Amount of healing to apply
	 * @return True if healing was applied successfully, false otherwise
	 * 
	 * Requirement: 19.3 - Utilities for simulating damage and healing
	 */
	DELVEDEEP_API bool SimulateHealing(
		ADelveDeepCharacter* Character,
		float HealAmount);

	/**
	 * Simulates healing to full health.
	 * Heals the character to maximum health.
	 * 
	 * @param Character The character to heal
	 * @return True if character was healed to full, false otherwise
	 * 
	 * Requirement: 19.3 - Utilities for simulating damage and healing
	 */
	DELVEDEEP_API bool SimulateFullHealing(
		ADelveDeepCharacter* Character);

	/**
	 * Verifies that damage was applied correctly.
	 * Checks that health decreased by the expected amount.
	 * 
	 * @param Character The character that took damage
	 * @param InitialHealth Health before damage
	 * @param DamageAmount Expected damage amount
	 * @param Tolerance Tolerance for floating point comparison (default: 0.01f)
	 * @return True if damage was applied correctly, false otherwise
	 * 
	 * Requirement: 19.3 - Utilities for simulating damage and healing
	 */
	DELVEDEEP_API bool VerifyDamageApplied(
		const ADelveDeepCharacter* Character,
		float InitialHealth,
		float DamageAmount,
		float Tolerance = 0.01f);

	/**
	 * Verifies that healing was applied correctly.
	 * Checks that health increased by the expected amount.
	 * 
	 * @param Character The character that was healed
	 * @param InitialHealth Health before healing
	 * @param HealAmount Expected heal amount
	 * @param Tolerance Tolerance for floating point comparison (default: 0.01f)
	 * @return True if healing was applied correctly, false otherwise
	 * 
	 * Requirement: 19.3 - Utilities for simulating damage and healing
	 */
	DELVEDEEP_API bool VerifyHealingApplied(
		const ADelveDeepCharacter* Character,
		float InitialHealth,
		float HealAmount,
		float Tolerance = 0.01f);

	// ========================================
	// Ability Testing
	// ========================================

	/**
	 * Triggers an ability on a character for testing.
	 * Simulates ability usage and verifies the response.
	 * 
	 * @param Character The character using the ability
	 * @param AbilityIndex Index of the ability to trigger
	 * @return True if ability was triggered successfully, false otherwise
	 * 
	 * Requirement: 19.4 - Utilities for triggering abilities and verifying effects
	 */
	DELVEDEEP_API bool TriggerAbility(
		ADelveDeepCharacter* Character,
		int32 AbilityIndex);

	/**
	 * Verifies that an ability was used correctly.
	 * Checks resource cost, cooldown, and effects.
	 * 
	 * @param Character The character that used the ability
	 * @param AbilityIndex Index of the ability that was used
	 * @param InitialResource Resource before ability use
	 * @param ExpectedResourceCost Expected resource cost
	 * @param Tolerance Tolerance for floating point comparison (default: 0.01f)
	 * @return True if ability was used correctly, false otherwise
	 * 
	 * Requirement: 19.4 - Utilities for triggering abilities and verifying effects
	 */
	DELVEDEEP_API bool VerifyAbilityUsed(
		const ADelveDeepCharacter* Character,
		int32 AbilityIndex,
		float InitialResource,
		float ExpectedResourceCost,
		float Tolerance = 0.01f);

	/**
	 * Simulates ability cooldown completion.
	 * Advances time to complete ability cooldown.
	 * 
	 * @param Character The character with the ability on cooldown
	 * @param AbilityIndex Index of the ability
	 * @return True if cooldown was completed, false otherwise
	 * 
	 * Requirement: 19.4 - Utilities for triggering abilities and verifying effects
	 */
	DELVEDEEP_API bool SimulateAbilityCooldownComplete(
		ADelveDeepCharacter* Character,
		int32 AbilityIndex);

	// ========================================
	// Mock Event Listener
	// ========================================

	/**
	 * Mock event listener for testing event broadcasts.
	 * Captures events and provides verification methods.
	 * 
	 * Requirement: 19.5 - Mock event listeners for verifying event broadcasts
	 */
	class DELVEDEEP_API FMockEventListener
	{
	public:
		/**
		 * Constructs a mock event listener.
		 * 
		 * @param EventSubsystem The event subsystem to register with
		 * @param EventTag The event tag to listen for
		 */
		FMockEventListener(
			UDelveDeepEventSubsystem* EventSubsystem,
			FGameplayTag EventTag);

		/**
		 * Destructor unregisters the listener.
		 */
		~FMockEventListener();

		/**
		 * Gets the number of events received.
		 * 
		 * @return Number of events captured
		 */
		int32 GetEventCount() const { return CapturedEvents.Num(); }

		/**
		 * Checks if any events were received.
		 * 
		 * @return True if at least one event was received, false otherwise
		 */
		bool WasEventReceived() const { return CapturedEvents.Num() > 0; }

		/**
		 * Gets all captured events.
		 * 
		 * @return Array of captured event payloads
		 */
		const TArray<TSharedPtr<FDelveDeepEventPayload>>& GetCapturedEvents() const { return CapturedEvents; }

		/**
		 * Gets the most recent event.
		 * 
		 * @return Pointer to the most recent event, or nullptr if no events
		 */
		const FDelveDeepEventPayload* GetLastEvent() const;

		/**
		 * Clears all captured events.
		 */
		void ClearEvents();

		/**
		 * Verifies that an event was received with specific criteria.
		 * 
		 * @param VerificationFunc Function to verify event payload
		 * @return True if at least one event matches the criteria, false otherwise
		 */
		bool VerifyEventReceived(TFunction<bool(const FDelveDeepEventPayload&)> VerificationFunc) const;

		/**
		 * Verifies that exactly N events were received.
		 * 
		 * @param ExpectedCount Expected number of events
		 * @return True if event count matches, false otherwise
		 */
		bool VerifyEventCount(int32 ExpectedCount) const;

		/**
		 * Verifies that events were received in a specific order.
		 * 
		 * @param VerificationFuncs Array of verification functions in expected order
		 * @return True if events match the expected order, false otherwise
		 */
		bool VerifyEventOrder(const TArray<TFunction<bool(const FDelveDeepEventPayload&)>>& VerificationFuncs) const;

		/**
		 * Gets the time elapsed since the last event.
		 * 
		 * @return Time in seconds since last event, or -1.0 if no events
		 */
		double GetTimeSinceLastEvent() const;

	private:
		/** Event subsystem reference */
		UDelveDeepEventSubsystem* EventSubsystem;

		/** Event tag being listened for */
		FGameplayTag ListenedEventTag;

		/** Delegate handle for unregistering */
		FDelegateHandle ListenerHandle;

		/** Captured events */
		TArray<TSharedPtr<FDelveDeepEventPayload>> CapturedEvents;

		/** Time of last event */
		double LastEventTime;

		/**
		 * Callback for event reception.
		 * 
		 * @param Payload The event payload
		 */
		void OnEventReceived(const FDelveDeepEventPayload& Payload);
	};

	// ========================================
	// Character State Verification
	// ========================================

	/**
	 * Verifies that a character is alive.
	 * 
	 * @param Character The character to check
	 * @return True if character is alive, false otherwise
	 */
	DELVEDEEP_API bool VerifyCharacterAlive(const ADelveDeepCharacter* Character);

	/**
	 * Verifies that a character is dead.
	 * 
	 * @param Character The character to check
	 * @return True if character is dead, false otherwise
	 */
	DELVEDEEP_API bool VerifyCharacterDead(const ADelveDeepCharacter* Character);

	/**
	 * Verifies that a character has all required components.
	 * 
	 * @param Character The character to check
	 * @return True if all components are present, false otherwise
	 */
	DELVEDEEP_API bool VerifyCharacterComponents(const ADelveDeepCharacter* Character);

	/**
	 * Verifies that a character's stats are within valid ranges.
	 * 
	 * @param Character The character to check
	 * @param Context Validation context for error tracking
	 * @return True if all stats are valid, false otherwise
	 */
	DELVEDEEP_API bool VerifyCharacterStatsValid(
		const ADelveDeepCharacter* Character,
		FValidationContext& Context);

	/**
	 * Verifies that a character's health is at maximum.
	 * 
	 * @param Character The character to check
	 * @param Tolerance Tolerance for floating point comparison (default: 0.01f)
	 * @return True if health is at maximum, false otherwise
	 */
	DELVEDEEP_API bool VerifyCharacterAtFullHealth(
		const ADelveDeepCharacter* Character,
		float Tolerance = 0.01f);

	/**
	 * Verifies that a character's resource is at maximum.
	 * 
	 * @param Character The character to check
	 * @param Tolerance Tolerance for floating point comparison (default: 0.01f)
	 * @return True if resource is at maximum, false otherwise
	 */
	DELVEDEEP_API bool VerifyCharacterAtFullResource(
		const ADelveDeepCharacter* Character,
		float Tolerance = 0.01f);
}
