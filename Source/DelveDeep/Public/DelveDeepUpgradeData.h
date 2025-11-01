// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DelveDeepValidation.h"
#include "DelveDeepUpgradeData.generated.h"

/**
 * Data asset for upgrade configuration.
 * Stores cost parameters, stat modifications, and upgrade dependencies.
 */
UCLASS(BlueprintType, Category = "DelveDeep|Configuration")
class DELVEDEEP_API UDelveDeepUpgradeData : public UDataAsset
{
	GENERATED_BODY()

public:
	// Display information
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText UpgradeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	// Cost parameters
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "1"))
	int32 BaseCost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float CostScalingFactor = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost", meta = (ClampMin = "1"))
	int32 MaxLevel = 10;

	// Stat modifications
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	float HealthModifier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	float DamageModifier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	float MoveSpeedModifier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	float ArmorModifier = 0.0f;

	// Dependencies
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dependencies")
	TArray<TSoftObjectPtr<UDelveDeepUpgradeData>> RequiredUpgrades;

	/**
	 * Calculates the cost for a specific upgrade level using exponential scaling.
	 * Formula: FinalCost = BaseCost * (CostScalingFactor ^ Level)
	 * 
	 * @param Level The upgrade level to calculate cost for
	 * @return The calculated cost for the specified level
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Configuration")
	int32 CalculateCostForLevel(int32 Level) const;

	// Validation
	virtual void PostLoad() override;
	bool Validate(FDelveDeepValidationContext& Context) const;
};
