// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacterComponent.h"
#include "DelveDeepEquipmentComponent.generated.h"

class UDelveDeepWeaponData;
class UDelveDeepCharacterData;

/**
 * Equipment component for managing character weapons and equipment.
 * Handles weapon equipping, stat modifiers from equipment, and equipment data loading.
 * 
 * This is a placeholder implementation for the Character System Foundation.
 * Full equipment functionality will be implemented in future specs.
 */
UCLASS(BlueprintType, Category = "DelveDeep|Character|Equipment", meta = (BlueprintSpawnableComponent))
class DELVEDEEP_API UDelveDeepEquipmentComponent : public UDelveDeepCharacterComponent
{
	GENERATED_BODY()

public:
	UDelveDeepEquipmentComponent();

	// Weapon management
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Equipment")
	void EquipWeapon(const UDelveDeepWeaponData* Weapon);

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Equipment")
	const UDelveDeepWeaponData* GetCurrentWeapon() const { return CurrentWeapon; }

	// Stat modifiers from equipment (placeholder)
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Equipment")
	float GetEquipmentStatModifier(FName StatName) const;

protected:
	// Current weapon
	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Equipment")
	const UDelveDeepWeaponData* CurrentWeapon;

	// Initialization
	virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData) override;

	// Apply/remove equipment modifiers (placeholder)
	void ApplyWeaponModifiers(const UDelveDeepWeaponData* Weapon);
	void RemoveWeaponModifiers(const UDelveDeepWeaponData* Weapon);

	// Validation
	virtual bool ValidateComponent(FValidationContext& Context) const override;
};
