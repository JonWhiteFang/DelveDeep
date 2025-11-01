// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepAbilityData.h"

void UDelveDeepAbilityData::PostLoad()
{
	Super::PostLoad();
	
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadAbilityData");
	
	if (!Validate(Context))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Ability data validation failed for '%s': %s"), 
			*GetName(), *Context.GetReport());
	}
}

bool UDelveDeepAbilityData::Validate(FDelveDeepValidationContext& Context) const
{
	bool bIsValid = true;
	
	// Validate cooldown
	if (Cooldown < 0.1f)
	{
		Context.AddError(FString::Printf(
			TEXT("Cooldown must be at least 0.1 seconds: %.2f"), Cooldown));
		bIsValid = false;
	}
	
	// Validate cast time
	if (CastTime < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("CastTime cannot be negative: %.2f"), CastTime));
		bIsValid = false;
	}
	
	// Validate duration
	if (Duration < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Duration cannot be negative: %.2f"), Duration));
		bIsValid = false;
	}
	
	// Validate resource cost
	if (ResourceCost < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("ResourceCost cannot be negative: %.2f"), ResourceCost));
		bIsValid = false;
	}
	
	// Validate damage multiplier
	if (DamageMultiplier < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("DamageMultiplier cannot be negative: %.2f"), DamageMultiplier));
		bIsValid = false;
	}
	
	// Validate AoE radius
	if (AoERadius < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("AoERadius cannot be negative: %.2f"), AoERadius));
		bIsValid = false;
	}
	
	// Validate AoE logic
	if (AoERadius > 0.0f && bAffectsAllies)
	{
		Context.AddWarning(TEXT("AoE ability affects allies - ensure this is intentional"));
	}
	
	// Validate instant cast with duration
	if (CastTime == 0.0f && Duration > 0.0f)
	{
		UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Ability '%s' is instant cast with duration (buff/debuff?)"), *GetName());
	}
	
	return bIsValid;
}
