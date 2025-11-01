// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacter.h"
#include "DelveDeepMage.generated.h"

/**
 * Mage character class - Spell casting specialist with Mana resource system
 * 
 * The Mage uses Mana to cast powerful spells. Mana regenerates slowly over time,
 * requiring careful resource management during combat.
 */
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepMage : public ADelveDeepCharacter
{
	GENERATED_BODY()

public:
	ADelveDeepMage();

	/**
	 * Regenerates Mana resource over time
	 * @param DeltaTime Time elapsed since last regeneration tick
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Mage")
	void RegenerateMana(float DeltaTime);

	/**
	 * Casts a fireball spell that deals AoE damage at target location
	 * Placeholder for future combat system integration
	 * @param TargetLocation World location where fireball should explode
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Mage")
	void CastFireball(FVector TargetLocation);

protected:
	/**
	 * Override to start Mana regeneration timer
	 */
	virtual void BeginPlay() override;

	/**
	 * Override to handle Mana-specific resource changes
	 */
	virtual void OnResourceChanged(float OldValue, float NewValue) override;

	/**
	 * Override to stop Mana regeneration timer
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Mana regeneration rate per second */
	static constexpr float ManaRegenRate = 5.0f;

	/** Maximum Mana the Mage can accumulate */
	static constexpr float MaxMana = 100.0f;

	/** Timer handle for Mana regeneration */
	FTimerHandle ManaRegenTimerHandle;

	/** Starts the Mana regeneration timer */
	void StartManaRegeneration();
};
