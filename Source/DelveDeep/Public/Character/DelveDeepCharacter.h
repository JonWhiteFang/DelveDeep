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
struct FDelveDeepValidationContext;

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Initialize character from configuration data.
	 * Queries UDelveDeepConfigurationManager using CharacterClassName.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void InitializeFromData();

	/**
	 * Validate character data using FDelveDeepValidationContext.
	 * @param Context Validation context for error/warning tracking
	 * @return True if character data is valid
	 */
	bool ValidateCharacterData(FDelveDeepValidationContext& Context) const;

	// Damage and healing
	/**
	 * Override of APawn::TakeDamage to handle damage events.
	 * @param Damage Amount of damage to apply
	 * @param DamageEvent Damage event information
	 * @param EventInstigator Controller that instigated the damage
	 * @param DamageCauser Actor that caused the damage
	 * @return Actual damage applied
	 */
	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 * Simplified damage function for Blueprint use.
	 * @param DamageAmount Amount of damage to apply
	 * @param DamageSource Actor that caused the damage
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void ApplySimpleDamage(float DamageAmount, AActor* DamageSource);

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

	// Movement
	/**
	 * Update sprite facing direction based on movement velocity.
	 * Called automatically when velocity changes.
	 */
	void UpdateSpriteFacingDirection();

	// Sprite animations
	/**
	 * Play idle animation from character data.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Animation")
	void PlayIdleAnimation();

	/**
	 * Play walk animation from character data.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Animation")
	void PlayWalkAnimation();

	/**
	 * Play attack animation from character data.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Animation")
	void PlayAttackAnimation();

	/**
	 * Play death animation from character data.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Animation")
	void PlayDeathAnimation();

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
