// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacter.h"
#include "DelveDeepWarrior.generated.h"

/**
 * Warrior character class - Melee combat specialist with Rage resource system
 * 
 * The Warrior generates Rage from dealing and taking damage, using it to fuel powerful melee abilities.
 * Rage is capped at 100 and decays over time when not in combat.
 */
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepWarrior : public ADelveDeepCharacter
{
	GENERATED_BODY()

public:
	ADelveDeepWarrior();

	/**
	 * Generates Rage resource for the Warrior
	 * @param Amount The amount of Rage to generate (clamped to MaxRage)
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Warrior")
	void GenerateRage(float Amount);

	/**
	 * Performs a cleave attack that hits multiple enemies in front of the Warrior
	 * Placeholder for future combat system integration
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Warrior")
	void PerformCleaveAttack();

protected:
	/**
	 * Override to generate Rage when taking damage
	 */
	virtual void TakeDamage(float DamageAmount, AActor* DamageSource) override;

	/**
	 * Override to handle Rage-specific resource changes
	 */
	virtual void OnResourceChanged(float OldValue, float NewValue) override;

private:
	/** Amount of Rage generated per point of damage dealt */
	static constexpr float RagePerDamageDealt = 5.0f;

	/** Amount of Rage generated per point of damage taken */
	static constexpr float RagePerDamageTaken = 10.0f;

	/** Maximum Rage the Warrior can accumulate */
	static constexpr float MaxRage = 100.0f;
};
