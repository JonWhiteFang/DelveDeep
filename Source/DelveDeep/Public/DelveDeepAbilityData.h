// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DelveDeepValidation.h"
#include "DelveDeepAbilityData.generated.h"

/**
 * Data asset for ability configuration.
 * Stores timing parameters, resource costs, damage parameters, and AoE settings.
 */
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepAbilityData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Display information
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// Timing parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.1"))
	float Cooldown = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float CastTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	// Resource cost
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "0.0"))
	float ResourceCost = 10.0f;

	// Damage parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	FName DamageType = "Physical";

	// Area of effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE", meta = (ClampMin = "0.0"))
	float AoERadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AoE")
	bool bAffectsAllies = false;

	// Validation
	virtual void PostLoad() override;
	bool Validate(FDelveDeepValidationContext& Context) const;
};
