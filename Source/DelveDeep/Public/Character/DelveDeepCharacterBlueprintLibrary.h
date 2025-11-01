// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DelveDeepCharacterBlueprintLibrary.generated.h"

class ADelveDeepCharacter;

/**
 * Blueprint function library for character system utilities.
 * Provides helper functions for spawning, querying, and modifying characters.
 */
UCLASS()
class DELVEDEEP_API UDelveDeepCharacterBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Character spawning
	/**
	 * Spawn a character at the specified location and rotation.
	 * @param WorldContextObject World context for spawning
	 * @param CharacterClass Class of character to spawn
	 * @param Location Spawn location
	 * @param Rotation Spawn rotation
	 * @return Spawned character, or nullptr if spawn failed
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character", 
		meta = (WorldContext = "WorldContextObject"))
	static ADelveDeepCharacter* SpawnCharacter(
		const UObject* WorldContextObject,
		TSubclassOf<ADelveDeepCharacter> CharacterClass,
		FVector Location,
		FRotator Rotation);

	// Character queries
	/**
	 * Check if a character is alive (not dead).
	 * @param Character Character to check
	 * @return True if character is alive
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	static bool IsCharacterAlive(ADelveDeepCharacter* Character);

	/**
	 * Get character health as a percentage (0.0 to 1.0).
	 * @param Character Character to query
	 * @return Health percentage
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	static float GetHealthPercentage(ADelveDeepCharacter* Character);

	/**
	 * Get character resource as a percentage (0.0 to 1.0).
	 * @param Character Character to query
	 * @return Resource percentage
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	static float GetResourcePercentage(ADelveDeepCharacter* Character);

	// Character actions
	/**
	 * Apply damage to a character.
	 * @param Character Character to damage
	 * @param DamageAmount Amount of damage to apply
	 * @param DamageSource Source of the damage
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	static void ApplyDamageToCharacter(
		ADelveDeepCharacter* Character,
		float DamageAmount,
		AActor* DamageSource);

	/**
	 * Heal a character.
	 * @param Character Character to heal
	 * @param HealAmount Amount of healing to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	static void HealCharacter(ADelveDeepCharacter* Character, float HealAmount);

	// Stat modifiers
	/**
	 * Add a temporary stat boost to a character.
	 * @param Character Character to modify
	 * @param StatName Name of the stat to boost (e.g., "MaxHealth", "MoveSpeed")
	 * @param Modifier Modifier value (additive)
	 * @param Duration Duration in seconds (0 = permanent)
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	static void AddTemporaryStatBoost(
		ADelveDeepCharacter* Character,
		FName StatName,
		float Modifier,
		float Duration);
};
