// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepMonsterConfig.h"

void FDelveDeepMonsterConfig::OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, 
	TArray<FString>& OutCollectedImportProblems)
{
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("ImportMonsterData");
	
	if (!Validate(Context))
	{
		// Add all validation errors to the import problems list
		for (const FString& Error : Context.ValidationErrors)
		{
			OutCollectedImportProblems.Add(FString::Printf(
				TEXT("Row '%s': %s"), *InRowName.ToString(), *Error));
		}
		
		// Log the validation failure
		UE_LOG(LogDelveDeepConfig, Error, 
			TEXT("Monster config validation failed for row '%s': %s"), 
			*InRowName.ToString(), *Context.GetReport());
	}
	else if (Context.ValidationWarnings.Num() > 0)
	{
		// Log warnings even if validation passed
		UE_LOG(LogDelveDeepConfig, Warning, 
			TEXT("Monster config warnings for row '%s': %d warning(s)"), 
			*InRowName.ToString(), Context.ValidationWarnings.Num());
	}
}

bool FDelveDeepMonsterConfig::Validate(FDelveDeepValidationContext& Context) const
{
	bool bIsValid = true;
	
	// Validate health (must be positive)
	if (Health <= 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Health must be positive (current value: %.2f)"), Health));
		bIsValid = false;
	}
	
	// Validate damage (must be non-negative)
	if (Damage < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Damage cannot be negative (current value: %.2f)"), Damage));
		bIsValid = false;
	}
	
	// Validate move speed (must be non-negative)
	if (MoveSpeed < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("MoveSpeed cannot be negative (current value: %.2f)"), MoveSpeed));
		bIsValid = false;
	}
	
	// Validate armor (must be non-negative)
	if (Armor < 0.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("Armor cannot be negative (current value: %.2f)"), Armor));
		bIsValid = false;
	}
	
	// Validate detection range (must be at least 100.0)
	if (DetectionRange < 100.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("DetectionRange must be at least 100.0 (current value: %.2f)"), DetectionRange));
		bIsValid = false;
	}
	
	// Validate attack range (must be at least 10.0)
	if (AttackRange < 10.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("AttackRange must be at least 10.0 (current value: %.2f)"), AttackRange));
		bIsValid = false;
	}
	
	// Validate attack range is not greater than detection range
	if (AttackRange > DetectionRange)
	{
		Context.AddWarning(FString::Printf(
			TEXT("AttackRange (%.2f) is greater than DetectionRange (%.2f)"), 
			AttackRange, DetectionRange));
	}
	
	// Validate AI behavior type is not empty
	if (AIBehaviorType.IsNone())
	{
		Context.AddWarning(TEXT("AIBehaviorType is not set"));
	}
	
	// Validate coin drop range
	if (CoinDropMin < 0)
	{
		Context.AddError(FString::Printf(
			TEXT("CoinDropMin cannot be negative (current value: %d)"), CoinDropMin));
		bIsValid = false;
	}
	
	if (CoinDropMax < 0)
	{
		Context.AddError(FString::Printf(
			TEXT("CoinDropMax cannot be negative (current value: %d)"), CoinDropMax));
		bIsValid = false;
	}
	
	if (CoinDropMax < CoinDropMin)
	{
		Context.AddError(FString::Printf(
			TEXT("CoinDropMax (%d) must be greater than or equal to CoinDropMin (%d)"), 
			CoinDropMax, CoinDropMin));
		bIsValid = false;
	}
	
	// Validate experience reward
	if (ExperienceReward < 0)
	{
		Context.AddError(FString::Printf(
			TEXT("ExperienceReward cannot be negative (current value: %d)"), ExperienceReward));
		bIsValid = false;
	}
	
	// Validate display information
	if (MonsterName.IsEmpty())
	{
		Context.AddWarning(TEXT("MonsterName is empty"));
	}
	
	return bIsValid;
}
