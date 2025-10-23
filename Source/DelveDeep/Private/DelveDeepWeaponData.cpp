// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepWeaponData.h"

void UDelveDeepWeaponData::PostLoad()
{
	Super::PostLoad();
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadWeaponData");
	
	if (!Validate(Context))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Weapon data validation failed for '%s': %s"), 
			*GetName(), *Context.GetReport());
	}
}

bool UDelveDeepWeaponData::Validate(FValidationContext& Context) const
{
	bool bIsValid = true;
	
	// Validate base damage
	if (BaseDamage <= 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseDamage must be positive: %.2f"), BaseDamage));
		bIsValid = false;
	}
	
	// Validate attack speed
	if (AttackSpeed < 0.1f)
	{
		Context.AddError(FString::Printf(
			TEXT("AttackSpeed must be at least 0.1: %.2f"), AttackSpeed));
		bIsValid = false;
	}
	
	// Validate range
	if (Range < 10.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Range must be at least 10.0: %.2f"), Range));
		bIsValid = false;
	}
	
	// Validate projectile speed
	if (ProjectileSpeed < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("ProjectileSpeed cannot be negative: %.2f"), ProjectileSpeed));
		bIsValid = false;
	}
	
	// Validate max pierce targets
	if (MaxPierceTargets < 1)
	{
		Context.AddError(FString::Printf(
			TEXT("MaxPierceTargets must be at least 1: %d"), MaxPierceTargets));
		bIsValid = false;
	}
	
	// Validate piercing logic
	if (bPiercing && ProjectileSpeed <= 0.0f)
	{
		Context.AddWarning(TEXT("Piercing enabled but ProjectileSpeed is 0 (melee weapon?)"));
	}
	
	// Validate special ability reference
	if (!SpecialAbility.IsNull())
	{
		// Special ability is optional, but if set, log it for tracking
		UE_LOG(LogDelveDeepConfig, Verbose, TEXT("Weapon '%s' has special ability assigned"), *GetName());
	}
	
	return bIsValid;
}
