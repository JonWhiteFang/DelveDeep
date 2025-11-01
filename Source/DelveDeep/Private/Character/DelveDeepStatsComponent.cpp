// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepStatsComponent.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepStats, Log, All);

UDelveDeepStatsComponent::UDelveDeepStatsComponent()
{
	// Disable tick by default for performance
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize base stats to default values
	BaseHealth = 100.0f;
	BaseResource = 100.0f;
	BaseDamage = 10.0f;
	BaseMoveSpeed = 300.0f;

	// Initialize current stats
	CurrentHealth = BaseHealth;
	MaxHealth = BaseHealth;
	CurrentResource = BaseResource;
	MaxResource = BaseResource;

	// Stats are clean on initialization
	bStatsDirty = false;
}

void UDelveDeepStatsComponent::InitializeFromCharacterData(const UDelveDeepCharacterData* Data)
{
	if (!Data)
	{
		UE_LOG(LogDelveDeepStats, Error, TEXT("Cannot initialize stats component with null character data"));
		return;
	}

	// Load base stats from character data
	BaseHealth = Data->BaseHealth;
	BaseResource = Data->BaseMana; // Note: This will be overridden by subclasses for different resource types
	BaseDamage = Data->BaseDamage;
	BaseMoveSpeed = Data->BaseMoveSpeed;

	// Set current stats to max values
	MaxHealth = BaseHealth;
	CurrentHealth = MaxHealth;
	MaxResource = BaseResource;
	CurrentResource = MaxResource;

	// Clear any existing modifiers
	ActiveModifiers.Empty();
	bStatsDirty = false;

	UE_LOG(LogDelveDeepStats, Display, 
		TEXT("Stats initialized: Health=%.2f, Resource=%.2f, Damage=%.2f, MoveSpeed=%.2f"),
		BaseHealth, BaseResource, BaseDamage, BaseMoveSpeed);
}

bool UDelveDeepStatsComponent::ValidateComponent(FValidationContext& Context) const
{
	bool bIsValid = true;

	// Validate base stats are in reasonable ranges
	if (BaseHealth <= 0.0f || BaseHealth > 10000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), BaseHealth));
		bIsValid = false;
	}

	if (BaseResource < 0.0f || BaseResource > 10000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseResource out of range: %.2f (expected 0-10000)"), BaseResource));
		bIsValid = false;
	}

	if (BaseDamage < 0.0f || BaseDamage > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseDamage out of range: %.2f (expected 0-1000)"), BaseDamage));
		bIsValid = false;
	}

	if (BaseMoveSpeed <= 0.0f || BaseMoveSpeed > 2000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseMoveSpeed out of range: %.2f (expected 1-2000)"), BaseMoveSpeed));
		bIsValid = false;
	}

	// Validate current stats are within valid ranges
	if (CurrentHealth < 0.0f || CurrentHealth > MaxHealth)
	{
		Context.AddWarning(FString::Printf(
			TEXT("CurrentHealth out of range: %.2f (expected 0-%.2f)"), 
			CurrentHealth, MaxHealth));
	}

	if (CurrentResource < 0.0f || CurrentResource > MaxResource)
	{
		Context.AddWarning(FString::Printf(
			TEXT("CurrentResource out of range: %.2f (expected 0-%.2f)"), 
			CurrentResource, MaxResource));
	}

	return bIsValid;
}
