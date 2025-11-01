// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacter.h"
#include "Events/DelveDeepEventPayload.h"
#include "DelveDeepNecromancer.generated.h"

/**
 * Necromancer character class - Minion summoning specialist with Soul resource system
 * 
 * The Necromancer collects Souls from slain enemies and uses them to summon undead minions.
 * Souls are a limited resource with a maximum of 10, requiring strategic use.
 */
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepNecromancer : public ADelveDeepCharacter
{
	GENERATED_BODY()

public:
	ADelveDeepNecromancer();

	/**
	 * Collects a Soul from a slain enemy
	 * Called automatically when enemy death events are received
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Necromancer")
	void CollectSoul();

	/**
	 * Summons an undead minion using collected Souls
	 * Placeholder for future minion system integration
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Necromancer")
	void SummonMinion();

	/**
	 * Gets the number of active minions currently summoned
	 * @return Number of active minions
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Necromancer")
	int32 GetActiveMinionCount() const { return ActiveMinions.Num(); }

protected:
	/**
	 * Override to register for enemy death events
	 */
	virtual void BeginPlay() override;

	/**
	 * Override to handle Soul-specific resource changes
	 */
	virtual void OnResourceChanged(float OldValue, float NewValue) override;

	/**
	 * Override to unregister from enemy death events
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Maximum Souls the Necromancer can collect */
	static constexpr float MaxSouls = 10.0f;

	/** Maximum number of minions that can be active simultaneously */
	static constexpr int32 MaxMinions = 3;

	/** Array of currently active minions */
	UPROPERTY()
	TArray<AActor*> ActiveMinions;

	/** Registers for enemy death events from the event subsystem */
	void RegisterForEnemyDeathEvents();

	/** Unregisters from enemy death events */
	void UnregisterFromEnemyDeathEvents();

	/**
	 * Callback for enemy death events
	 * @param Payload Event payload containing death information
	 */
	void OnEnemyDeath(const FDelveDeepEventPayload& Payload);

	/** Handle for enemy death event listener */
	FDelegateHandle EnemyDeathEventHandle;
};
