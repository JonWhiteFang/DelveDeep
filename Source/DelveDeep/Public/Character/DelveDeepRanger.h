// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacter.h"
#include "DelveDeepRanger.generated.h"

/**
 * Ranger character class - Ranged combat specialist with Energy resource system
 * 
 * The Ranger uses Energy to fuel ranged attacks and abilities. Energy regenerates
 * automatically over time at a steady rate.
 */
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepRanger : public ADelveDeepCharacter
{
	GENERATED_BODY()

public:
	ADelveDeepRanger();

	/**
	 * Regenerates Energy resource over time
	 * @param DeltaTime Time elapsed since last regeneration tick
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Ranger")
	void RegenerateEnergy(float DeltaTime);

	/**
	 * Performs a piercing shot that hits multiple enemies in a line
	 * Placeholder for future combat system integration
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Ranger")
	void PerformPiercingShot();

protected:
	/**
	 * Override to start Energy regeneration timer
	 */
	virtual void BeginPlay() override;

	/**
	 * Override to handle Energy-specific resource changes
	 */
	virtual void OnResourceChanged(float OldValue, float NewValue) override;

	/**
	 * Override to stop Energy regeneration timer
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Energy regeneration rate per second */
	static constexpr float EnergyRegenRate = 10.0f;

	/** Maximum Energy the Ranger can accumulate */
	static constexpr float MaxEnergy = 100.0f;

	/** Timer handle for Energy regeneration */
	FTimerHandle EnergyRegenTimerHandle;

	/** Starts the Energy regeneration timer */
	void StartEnergyRegeneration();
};
