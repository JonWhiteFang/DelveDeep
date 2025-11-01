// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepAbilitiesComponent.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Configuration/DelveDeepAbilityData.h"
#include "Validation/ValidationContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepAbilities, Log, All);

UDelveDeepAbilitiesComponent::UDelveDeepAbilitiesComponent()
{
	// Disable tick by default for performance
	PrimaryComponentTick.bCanEverTick = false;
}

void UDelveDeepAbilitiesComponent::InitializeFromCharacterData(const UDelveDeepCharacterData* Data)
{
	if (!Data)
	{
		UE_LOG(LogDelveDeepAbilities, Error, TEXT("Cannot initialize abilities component with null character data"));
		return;
	}

	// Load starting abilities from character data
	for (const TSoftObjectPtr<UDelveDeepAbilityData>& AbilityPtr : Data->StartingAbilities)
	{
		if (!AbilityPtr.IsNull())
		{
			if (const UDelveDeepAbilityData* Ability = AbilityPtr.LoadSynchronous())
			{
				AddAbility(Ability);
			}
			else
			{
				UE_LOG(LogDelveDeepAbilities, Warning,
					TEXT("Failed to load starting ability from character data"));
			}
		}
	}

	UE_LOG(LogDelveDeepAbilities, Display,
		TEXT("Abilities component initialized with %d abilities"), Abilities.Num());
}

bool UDelveDeepAbilitiesComponent::ValidateComponent(FValidationContext& Context) const
{
	Context.SystemName = TEXT("AbilitiesComponent");
	Context.OperationName = TEXT("ValidateComponent");

	bool bIsValid = true;

	// Validate all ability references
	for (int32 i = 0; i < Abilities.Num(); ++i)
	{
		if (!Abilities[i])
		{
			Context.AddError(FString::Printf(
				TEXT("Ability at index %d is null"), i));
			bIsValid = false;
		}
	}

	return bIsValid;
}

void UDelveDeepAbilitiesComponent::AddAbility(const UDelveDeepAbilityData* Ability)
{
	if (!Ability)
	{
		UE_LOG(LogDelveDeepAbilities, Warning, TEXT("Attempted to add null ability"));
		return;
	}

	// Check if ability already exists
	if (Abilities.Contains(Ability))
	{
		UE_LOG(LogDelveDeepAbilities, Warning,
			TEXT("Ability already exists in abilities list"));
		return;
	}

	// Add ability to list
	Abilities.Add(Ability);

	UE_LOG(LogDelveDeepAbilities, Verbose,
		TEXT("Added ability (Total: %d)"), Abilities.Num());
}

void UDelveDeepAbilitiesComponent::RemoveAbility(const UDelveDeepAbilityData* Ability)
{
	if (!Ability)
	{
		UE_LOG(LogDelveDeepAbilities, Warning, TEXT("Attempted to remove null ability"));
		return;
	}

	// Remove ability from list
	int32 RemovedCount = Abilities.Remove(Ability);

	if (RemovedCount > 0)
	{
		// Remove cooldown entry if exists
		AbilityCooldowns.Remove(Ability);

		UE_LOG(LogDelveDeepAbilities, Verbose,
			TEXT("Removed ability (Total: %d)"), Abilities.Num());
	}
	else
	{
		UE_LOG(LogDelveDeepAbilities, Warning,
			TEXT("Ability not found in abilities list"));
	}
}

bool UDelveDeepAbilitiesComponent::UseAbility(int32 AbilityIndex)
{
	// Validate ability index
	if (!Abilities.IsValidIndex(AbilityIndex))
	{
		UE_LOG(LogDelveDeepAbilities, Warning,
			TEXT("Invalid ability index: %d (Total: %d)"),
			AbilityIndex, Abilities.Num());
		return false;
	}

	// Check if ability can be used
	if (!CanUseAbility(AbilityIndex))
	{
		UE_LOG(LogDelveDeepAbilities, Verbose,
			TEXT("Ability at index %d cannot be used (on cooldown or insufficient resources)"),
			AbilityIndex);
		return false;
	}

	// Placeholder: Actual ability execution will be implemented in future tasks
	UE_LOG(LogDelveDeepAbilities, Display,
		TEXT("Used ability at index %d (placeholder)"), AbilityIndex);

	return true;
}

bool UDelveDeepAbilitiesComponent::CanUseAbility(int32 AbilityIndex) const
{
	// Validate ability index
	if (!Abilities.IsValidIndex(AbilityIndex))
	{
		return false;
	}

	const UDelveDeepAbilityData* Ability = Abilities[AbilityIndex];
	if (!Ability)
	{
		return false;
	}

	// Check cooldown
	if (const float* Cooldown = AbilityCooldowns.Find(Ability))
	{
		if (*Cooldown > 0.0f)
		{
			return false; // Still on cooldown
		}
	}

	// Placeholder: Will check resource cost in future implementation
	// For now, assume ability can be used if not on cooldown
	return true;
}
