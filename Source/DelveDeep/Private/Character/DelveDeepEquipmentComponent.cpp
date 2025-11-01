// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepEquipmentComponent.h"
#include "Character/DelveDeepCharacter.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Configuration/DelveDeepWeaponData.h"
#include "Validation/ValidationContext.h"
#include "DelveDeepLogChannels.h"

UDelveDeepEquipmentComponent::UDelveDeepEquipmentComponent()
{
	// Disable tick by default
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	CurrentWeapon = nullptr;
}

void UDelveDeepEquipmentComponent::InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData)
{
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeep, Warning, TEXT("EquipmentComponent: Cannot initialize from null character data"));
		return;
	}

	// Load starting weapon from data asset
	if (!CharacterData->StartingWeapon.IsNull())
	{
		const UDelveDeepWeaponData* WeaponData = CharacterData->StartingWeapon.LoadSynchronous();
		if (WeaponData)
		{
			EquipWeapon(WeaponData);
			UE_LOG(LogDelveDeep, Display, TEXT("EquipmentComponent: Equipped starting weapon '%s'"), 
				*WeaponData->GetName());
		}
		else
		{
			UE_LOG(LogDelveDeep, Warning, TEXT("EquipmentComponent: Failed to load starting weapon"));
		}
	}
	else
	{
		UE_LOG(LogDelveDeep, Display, TEXT("EquipmentComponent: No starting weapon assigned"));
	}
}

void UDelveDeepEquipmentComponent::EquipWeapon(const UDelveDeepWeaponData* Weapon)
{
	if (!Weapon)
	{
		UE_LOG(LogDelveDeep, Warning, TEXT("EquipmentComponent: Cannot equip null weapon"));
		return;
	}

	// Remove modifiers from current weapon
	if (CurrentWeapon)
	{
		RemoveWeaponModifiers(CurrentWeapon);
	}

	// Set new weapon
	CurrentWeapon = Weapon;

	// Apply modifiers from new weapon
	ApplyWeaponModifiers(CurrentWeapon);

	UE_LOG(LogDelveDeep, Display, TEXT("EquipmentComponent: Equipped weapon '%s'"), 
		*Weapon->GetName());

	// TODO: Broadcast WeaponEquipped event through UDelveDeepEventSubsystem
	// This will be implemented when the event system integration is complete
}

float UDelveDeepEquipmentComponent::GetEquipmentStatModifier(FName StatName) const
{
	// Placeholder implementation
	// TODO: Calculate stat modifiers from equipped weapon and other equipment
	// This will be implemented when weapon data includes stat modifiers
	return 0.0f;
}

void UDelveDeepEquipmentComponent::ApplyWeaponModifiers(const UDelveDeepWeaponData* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// Placeholder implementation
	// TODO: Apply weapon stat modifiers to character stats
	// This will be implemented when weapon data includes stat modifiers
	// Example: StatsComponent->AddStatModifier("Damage", Weapon->DamageBonus, -1.0f);
}

void UDelveDeepEquipmentComponent::RemoveWeaponModifiers(const UDelveDeepWeaponData* Weapon)
{
	if (!Weapon)
	{
		return;
	}

	// Placeholder implementation
	// TODO: Remove weapon stat modifiers from character stats
	// This will be implemented when weapon data includes stat modifiers
	// Example: StatsComponent->RemoveStatModifier("Damage");
}

bool UDelveDeepEquipmentComponent::ValidateComponent(FValidationContext& Context) const
{
	Context.SystemName = TEXT("Equipment");
	Context.OperationName = TEXT("ValidateComponent");

	bool bIsValid = Super::ValidateComponent(Context);

	// Validate weapon reference if equipped
	if (CurrentWeapon && !IsValid(CurrentWeapon))
	{
		Context.AddError(TEXT("Current weapon reference is invalid"));
		bIsValid = false;
	}

	return bIsValid;
}
