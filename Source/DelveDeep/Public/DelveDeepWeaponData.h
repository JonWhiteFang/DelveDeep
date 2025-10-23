// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DelveDeepValidation.h"
#include "DelveDeepWeaponData.generated.h"

// Forward declarations
class UDelveDeepAbilityData;

/**
 * Data asset for weapon configuration.
 * Stores combat stats, projectile parameters, and special ability references.
 */
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Display information
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// Combat stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "1.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.1"))
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "10.0"))
	float Range = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FName DamageType = "Physical";

	// Projectile parameters (for ranged weapons)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	bool bPiercing = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "1"))
	int32 MaxPierceTargets = 1;

	// Special abilities
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TSoftObjectPtr<UDelveDeepAbilityData> SpecialAbility;

	// Validation
	virtual void PostLoad() override;
	bool Validate(FValidationContext& Context) const;
};
