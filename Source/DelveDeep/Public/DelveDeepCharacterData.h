// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DelveDeepValidation.h"
#include "DelveDeepCharacterData.generated.h"

// Forward declarations
class UDelveDeepWeaponData;
class UDelveDeepAbilityData;

/**
 * Data asset for character configuration.
 * Stores all base stats, resource parameters, and starting equipment for a character class.
 */
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepCharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Display information
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// Base stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0", ClampMax = "10000.0"))
	float BaseHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BaseArmor = 0.0f;

	// Resource system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MaxResource = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ResourceRegenRate = 5.0f;

	// Combat parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float BaseAttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float AttackRange = 100.0f;

	// Starting equipment
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TSoftObjectPtr<UDelveDeepWeaponData> StartingWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TArray<TSoftObjectPtr<UDelveDeepAbilityData>> StartingAbilities;

	// Validation
	virtual void PostLoad() override;
	bool Validate(FValidationContext& Context) const;
};
