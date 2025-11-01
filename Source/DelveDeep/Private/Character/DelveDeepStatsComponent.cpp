// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepStatsComponent.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "GameplayTagsManager.h"
#include "Engine/World.h"

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
	BaseResource = Data->MaxResource; // Note: This will be overridden by subclasses for different resource types
	BaseDamage = Data->BaseDamage;
	BaseMoveSpeed = Data->MoveSpeed;

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

void UDelveDeepStatsComponent::ModifyHealth(float Delta)
{
	// Store old value for event broadcasting
	float OldHealth = CurrentHealth;

	// Apply delta and clamp to valid range
	CurrentHealth = FMath::Clamp(CurrentHealth + Delta, 0.0f, MaxHealth);

	// Only broadcast events if health actually changed
	if (!FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		// Broadcast stat changed event
		OnStatChanged(FName("Health"), OldHealth, CurrentHealth);

		// Broadcast health change event through event subsystem
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GameInstance = World->GetGameInstance())
			{
				if (UDelveDeepEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>())
				{
					FDelveDeepHealthChangeEventPayload Payload;
					Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Character.Health.Changed"));
					Payload.Character = GetCharacterOwner();
					Payload.PreviousHealth = OldHealth;
					Payload.NewHealth = CurrentHealth;
					Payload.MaxHealth = MaxHealth;
					Payload.Instigator = GetCharacterOwner();

					EventSubsystem->BroadcastEvent(Payload);
				}
			}
		}

		UE_LOG(LogDelveDeepStats, Verbose, 
			TEXT("Health modified: %.2f -> %.2f (Delta: %.2f)"), 
			OldHealth, CurrentHealth, Delta);
	}
}

void UDelveDeepStatsComponent::ModifyResource(float Delta)
{
	// Store old value for event broadcasting
	float OldResource = CurrentResource;

	// Apply delta and clamp to valid range
	CurrentResource = FMath::Clamp(CurrentResource + Delta, 0.0f, MaxResource);

	// Only broadcast events if resource actually changed
	if (!FMath::IsNearlyEqual(OldResource, CurrentResource))
	{
		// Broadcast resource changed event
		OnResourceChanged(OldResource, CurrentResource);

		// Broadcast stat changed event
		OnStatChanged(FName("Resource"), OldResource, CurrentResource);

		UE_LOG(LogDelveDeepStats, Verbose, 
			TEXT("Resource modified: %.2f -> %.2f (Delta: %.2f)"), 
			OldResource, CurrentResource, Delta);
	}
}

void UDelveDeepStatsComponent::ResetToMaxValues()
{
	float OldHealth = CurrentHealth;
	float OldResource = CurrentResource;

	CurrentHealth = MaxHealth;
	CurrentResource = MaxResource;

	UE_LOG(LogDelveDeepStats, Display, 
		TEXT("Stats reset to max values: Health=%.2f, Resource=%.2f"), 
		MaxHealth, MaxResource);

	// Broadcast events if values changed
	if (!FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		OnStatChanged(FName("Health"), OldHealth, CurrentHealth);
	}

	if (!FMath::IsNearlyEqual(OldResource, CurrentResource))
	{
		OnResourceChanged(OldResource, CurrentResource);
		OnStatChanged(FName("Resource"), OldResource, CurrentResource);
	}
}
