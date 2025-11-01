// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacterComponent.h"
#include "DelveDeepStatsComponent.generated.h"

class UDelveDeepCharacterData;
struct FValidationContext;

/**
 * Stat modifier structure for temporary stat boosts/debuffs.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepStatModifier
{
	GENERATED_BODY()

	/** Modifier value (additive or multiplicative depending on stat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Modifier = 0.0f;

	/** Total duration of the modifier in seconds (0 = permanent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Duration = 0.0f;

	/** Remaining time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float RemainingTime = 0.0f;

	FDelveDeepStatModifier()
		: Modifier(0.0f)
		, Duration(0.0f)
		, RemainingTime(0.0f)
	{}

	FDelveDeepStatModifier(float InModifier, float InDuration)
		: Modifier(InModifier)
		, Duration(InDuration)
		, RemainingTime(InDuration)
	{}
};

/**
 * Stats component managing character health, resource, damage, and move speed.
 * Supports temporary stat modifiers with duration tracking.
 */
UCLASS(BlueprintType, ClassGroup = (DelveDeep), meta = (BlueprintSpawnableComponent))
class DELVEDEEP_API UDelveDeepStatsComponent : public UDelveDeepCharacterComponent
{
	GENERATED_BODY()

public:
	UDelveDeepStatsComponent();

	// Component lifecycle
	virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* Data) override;
	virtual bool ValidateComponent(FValidationContext& Context) const override;

	// Base stats (loaded from character data)
	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float BaseHealth;

	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float BaseResource;

	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float BaseDamage;

	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float BaseMoveSpeed;

	// Current stats
	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float CurrentResource;

	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Stats")
	float MaxResource;

	// Stat getters
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetCurrentResource() const { return CurrentResource; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetMaxResource() const { return MaxResource; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetHealthPercentage() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetResourcePercentage() const { return MaxResource > 0.0f ? CurrentResource / MaxResource : 0.0f; }

	// Health modification
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Stats")
	void ModifyHealth(float Delta);

	// Resource modification
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Stats")
	void ModifyResource(float Delta);

	// Reset stats to maximum values
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Stats")
	void ResetToMaxValues();

	// Stat modifier system
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Stats")
	void AddStatModifier(FName StatName, float Modifier, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Stats")
	void RemoveStatModifier(FName StatName);

	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Stats")
	void ClearAllModifiers();

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Stats")
	float GetModifiedStat(FName StatName) const;

	// Recalculate all stats with modifiers applied
	void RecalculateStats();

	// Blueprint events
	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Stats")
	void OnStatChanged(FName StatName, float OldValue, float NewValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "DelveDeep|Stats")
	void OnResourceChanged(float OldValue, float NewValue);

protected:
	/**
	 * Active stat modifiers mapped by modifier name.
	 */
	UPROPERTY(Transient)
	TMap<FName, FDelveDeepStatModifier> ActiveModifiers;

	/**
	 * Flag indicating stats need recalculation.
	 */
	bool bStatsDirty;

	/**
	 * Cached modified stats (recalculated when dirty).
	 */
	float CachedMaxHealth;
	float CachedMaxResource;
	float CachedMoveSpeed;

	/**
	 * Timer handle for cleaning up expired modifiers.
	 */
	FTimerHandle CleanupTimerHandle;

	/**
	 * Apply modifiers to a base stat value.
	 */
	float ApplyModifiers(FName StatName, float BaseValue) const;

	/**
	 * Clean up expired stat modifiers.
	 */
	void CleanupExpiredModifiers();
};
