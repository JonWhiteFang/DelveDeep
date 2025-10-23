// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepCharacterData.h"

void UDelveDeepCharacterData::PostLoad()
{
	Super::PostLoad();
	
	FValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadCharacterData");
	
	if (!Validate(Context))
	{
		UE_LOG(LogDelveDeepConfig, Error, TEXT("Character data validation failed for '%s': %s"), 
			*GetName(), *Context.GetReport());
	}
}

bool UDelveDeepCharacterData::Validate(FValidationContext& Context) const
{
	bool bIsValid = true;
	
	// Validate base health
	if (BaseHealth <= 0.0f || BaseHealth > 10000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseHealth out of range: %.2f (expected 1.0-10000.0)"), BaseHealth));
		bIsValid = false;
	}
	
	// Validate base damage
	if (BaseDamage <= 0.0f || BaseDamage > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseDamage out of range: %.2f (expected 1.0-1000.0)"), BaseDamage));
		bIsValid = false;
	}
	
	// Validate move speed
	if (MoveSpeed < 50.0f || MoveSpeed > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("MoveSpeed out of range: %.2f (expected 50.0-1000.0)"), MoveSpeed));
		bIsValid = false;
	}
	
	// Validate base armor
	if (BaseArmor < 0.0f || BaseArmor > 100.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseArmor out of range: %.2f (expected 0.0-100.0)"), BaseArmor));
		bIsValid = false;
	}
	
	// Validate max resource
	if (MaxResource < 0.0f || MaxResource > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("MaxResource out of range: %.2f (expected 0.0-1000.0)"), MaxResource));
		bIsValid = false;
	}
	
	// Validate resource regen rate
	if (ResourceRegenRate < 0.0f || ResourceRegenRate > 100.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("ResourceRegenRate out of range: %.2f (expected 0.0-100.0)"), ResourceRegenRate));
		bIsValid = false;
	}
	
	// Validate base attack speed
	if (BaseAttackSpeed < 0.1f || BaseAttackSpeed > 10.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseAttackSpeed out of range: %.2f (expected 0.1-10.0)"), BaseAttackSpeed));
		bIsValid = false;
	}
	
	// Validate attack range
	if (AttackRange < 10.0f || AttackRange > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("AttackRange out of range: %.2f (expected 10.0-1000.0)"), AttackRange));
		bIsValid = false;
	}
	
	// Validate starting weapon reference
	if (StartingWeapon.IsNull())
	{
		Context.AddWarning(TEXT("No starting weapon assigned"));
	}
	
	// Validate starting abilities
	if (StartingAbilities.Num() == 0)
	{
		Context.AddWarning(TEXT("No starting abilities assigned"));
	}
	
	for (int32 i = 0; i < StartingAbilities.Num(); ++i)
	{
		if (StartingAbilities[i].IsNull())
		{
			Context.AddError(FString::Printf(
				TEXT("Null reference in starting abilities at index %d"), i));
			bIsValid = false;
		}
	}
	
	return bIsValid;
}
