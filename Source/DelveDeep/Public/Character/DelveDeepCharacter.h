// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "DelveDeepCharacter.generated.h"

class UDelveDeepStatsComponent;
class UDelveDeepAbilitiesComponent;
class UDelveDeepEquipmentComponent;
class UDelveDeepCharacterData;
class UDelveDeepAbilityData;
class UDelveDeepWeaponData;
struct FValidationContext;

/**
 * Base character class for DelveDeep.
 * Inherits from APaperCharacter for 2D sprite support.
 * Uses component-based architecture for stats, abilities, and equipment.
 */
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ADelveDeepCharacter();

	// Character data loading
	virtual void BeginPlay() override;

	/**
	 * Initialize character from configuration data.
	 * Queries UDelveDeepConfigurationManager using CharacterClassName.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void InitializeFromData();

	/**
	 * Validate character data using FValidationContext.
	 * @param Context Validation context for error/warning tracking
	 * @return True if character data is valid
	 */
	bool ValidateCharacterData(FValidationContext& Context) const;

	// Damage and healing
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void TakeDamage(float DamageAmount, AActor* DamageSource);

	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void Heal(float HealAmount);

	// Death and respawn
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void Die();

	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void Respawn();

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	bool IsDead() const { return bIsDead; }

	// Component accessors
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	UDelveDeepStatsComponent* GetStatsComponent() const { return StatsComponent; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	UDelveDeepAbilitiesComponent* GetAbilitiesComponent() const { return AbilitiesComponent; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	UDelveDeepEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	// Stats accessors
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
	float GetCurrentResource() const;

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Stats")
	float GetMaxResource() const;

protected:
	/**
	 * Initialize all character components after data is loaded.
	 */
	void InitializeComponents();

	// Blueprint events
	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnDamaged(float DamageAmount, AActor* DamageSource);

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnHealed(float HealAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnStatChanged(FName StatName, float OldValue, float NewValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnResourceChanged(float OldValue, float NewValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnAbilityUsed(const UDelveDeepAbilityData* Ability);

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Character")
	void OnWeaponEquipped(const UDelveDeepWeaponData* Weapon);

	/**
	 * Broadcast damage event through event subsystem.
	 */
	void BroadcastDamageEvent(float DamageAmount, AActor* DamageSource);

	/**
	 * Broadcast heal event through event subsystem.
	 */
	void BroadcastHealEvent(float HealAmount);

	/**
	 * Broadcast death event through event subsystem.
	 */
	void BroadcastDeathEvent(AActor* Killer);

	/**
	 * Name used to lookup character data from configuration manager.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	FName CharacterClassName;

	/**
	 * Stats component managing health, resource, damage, and move speed.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	UDelveDeepStatsComponent* StatsComponent;

	/**
	 * Abilities component managing character abilities and cooldowns.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	UDelveDeepAbilitiesComponent* AbilitiesComponent;

	/**
	 * Equipment component managing weapons and equipment.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	UDelveDeepEquipmentComponent* EquipmentComponent;

	/**
	 * Cached reference to loaded character data.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DelveDeep|Character")
	const UDelveDeepCharacterData* CharacterData;

	/**
	 * Flag indicating if character is dead.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character")
	bool bIsDead;

	/**
	 * Timer handle for destroying actor after death.
	 */
	FTimerHandle DeathTimerHandle;
};
