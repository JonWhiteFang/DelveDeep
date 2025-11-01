// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepUpgradeData.h"

void UDelveDeepUpgradeData::PostLoad()
{
	Super::PostLoad();
	
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadUpgradeData");
	
	if (!Validate(Context))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Upgrade data validation failed for '%s': %s"), 
			*GetName(), *Context.GetReport());
	}
}

int32 UDelveDeepUpgradeData::CalculateCostForLevel(int32 Level) const
{
	// Clamp level to valid range
	Level = FMath::Clamp(Level, 1, MaxLevel);
	
	// Calculate cost using exponential scaling: BaseCost * (ScalingFactor ^ Level)
	float Cost = static_cast<float>(BaseCost) * FMath::Pow(CostScalingFactor, static_cast<float>(Level));
	
	return FMath::RoundToInt(Cost);
}

bool UDelveDeepUpgradeData::Validate(FDelveDeepValidationContext& Context) const
{
	bool bIsValid = true;
	
	// Validate base cost
	if (BaseCost < 1)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseCost must be at least 1: %d"), BaseCost));
		bIsValid = false;
	}
	
	// Validate cost scaling factor
	if (CostScalingFactor < 1.0f || CostScalingFactor > 10.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("CostScalingFactor out of range: %.2f (expected 1.0-10.0)"), CostScalingFactor));
		bIsValid = false;
	}
	
	// Validate max level
	if (MaxLevel < 1)
	{
		Context.AddError(FString::Printf(
			TEXT("MaxLevel must be at least 1: %d"), MaxLevel));
		bIsValid = false;
	}
	
	// Validate that at least one stat modifier is non-zero
	if (HealthModifier == 0.0f && DamageModifier == 0.0f && 
		MoveSpeedModifier == 0.0f && ArmorModifier == 0.0f)
	{
		Context.AddWarning(TEXT("All stat modifiers are zero - upgrade has no effect"));
	}
	
	// Validate required upgrades
	for (int32 i = 0; i < RequiredUpgrades.Num(); ++i)
	{
		if (RequiredUpgrades[i].IsNull())
		{
			Context.AddError(FString::Printf(
				TEXT("Null reference in required upgrades at index %d"), i));
			bIsValid = false;
		}
	}
	
	// Check for circular dependencies (self-reference)
	for (const TSoftObjectPtr<UDelveDeepUpgradeData>& RequiredUpgrade : RequiredUpgrades)
	{
		if (!RequiredUpgrade.IsNull())
		{
			// Get the asset path for comparison
			FString ThisPath = GetPathName();
			FString RequiredPath = RequiredUpgrade.ToSoftObjectPath().ToString();
			
			if (ThisPath == RequiredPath)
			{
				Context.AddError(TEXT("Upgrade cannot require itself (circular dependency)"));
				bIsValid = false;
			}
		}
	}
	
	return bIsValid;
}
