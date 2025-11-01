// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepCharacterData.h"
#include "DelveDeepValidationTemplates.h"
#include "DelveDeepValidationSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UDelveDeepCharacterData::PostLoad()
{
	Super::PostLoad();
	
	// Try to get validation subsystem for cached validation
	UDelveDeepValidationSubsystem* ValidationSubsystem = nullptr;
	
	// Get world and game instance if available
	UWorld* World = GetWorld();
	if (World && World->GetGameInstance())
	{
		ValidationSubsystem = World->GetGameInstance()->GetSubsystem<UDelveDeepValidationSubsystem>();
	}
	
	FDelveDeepValidationContext Context;
	Context.SystemName = TEXT("Configuration");
	Context.OperationName = TEXT("LoadCharacterData");
	
	if (ValidationSubsystem)
	{
		// Use subsystem validation with caching
		if (!ValidationSubsystem->ValidateObjectWithCache(this, Context))
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("Character data validation failed for '%s': %s"), 
				*GetName(), *Context.GetReport());
		}
	}
	else
	{
		// Fall back to basic validation
		if (!Validate(Context))
		{
			UE_LOG(LogDelveDeepConfig, Error, TEXT("Character data validation failed for '%s': %s"), 
				*GetName(), *Context.GetReport());
		}
	}
}

bool UDelveDeepCharacterData::Validate(FDelveDeepValidationContext& Context) const
{
	bool bIsValid = true;
	
	// Add info message for validation start
	Context.AddInfo(FString::Printf(TEXT("Validating character data: %s"), *GetName()));
	
	// Validate base health using template
	bIsValid &= DelveDeepValidation::ValidateRange(BaseHealth, 1.0f, 10000.0f, TEXT("BaseHealth"), Context);
	
	// Validate base damage using template
	bIsValid &= DelveDeepValidation::ValidateRange(BaseDamage, 1.0f, 1000.0f, TEXT("BaseDamage"), Context);
	
	// Validate move speed using template
	bIsValid &= DelveDeepValidation::ValidateRange(MoveSpeed, 50.0f, 1000.0f, TEXT("MoveSpeed"), Context);
	
	// Validate base armor using template
	bIsValid &= DelveDeepValidation::ValidateRange(BaseArmor, 0.0f, 100.0f, TEXT("BaseArmor"), Context);
	
	// Validate max resource using template
	bIsValid &= DelveDeepValidation::ValidateRange(MaxResource, 0.0f, 1000.0f, TEXT("MaxResource"), Context);
	
	// Validate resource regen rate using template
	bIsValid &= DelveDeepValidation::ValidateRange(ResourceRegenRate, 0.0f, 100.0f, TEXT("ResourceRegenRate"), Context);
	
	// Validate base attack speed using template
	bIsValid &= DelveDeepValidation::ValidateRange(BaseAttackSpeed, 0.1f, 10.0f, TEXT("BaseAttackSpeed"), Context);
	
	// Validate attack range using template
	bIsValid &= DelveDeepValidation::ValidateRange(AttackRange, 10.0f, 1000.0f, TEXT("AttackRange"), Context);
	
	// Validate starting weapon reference (warning severity for optional field)
	DelveDeepValidation::ValidateSoftReference(StartingWeapon, TEXT("StartingWeapon"), Context, true, EValidationSeverity::Warning);
	
	// Validate starting abilities
	if (StartingAbilities.Num() == 0)
	{
		Context.AddIssue(EValidationSeverity::Warning, TEXT("No starting abilities assigned"));
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
	
	// Add success info message if validation passed
	if (bIsValid)
	{
		Context.AddInfo(TEXT("Character data validation passed"));
	}
	
	return bIsValid;
}
