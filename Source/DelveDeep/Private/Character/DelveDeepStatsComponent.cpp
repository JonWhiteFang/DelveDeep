// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepStatsComponent.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "GameplayTagsManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/DelveDeepCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepStats, Log, All);

// Performance profiling stats
DECLARE_STATS_GROUP(TEXT("DelveDeep"), STATGROUP_DelveDeep, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Stats RecalculateStats"), STAT_StatsRecalculate, STATGROUP_DelveDeep);

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

	// Initialize cached stats
	CachedMaxHealth = MaxHealth;
	CachedMaxResource = MaxResource;
	CachedMoveSpeed = BaseMoveSpeed;

	// Set up timer for cleaning up expired modifiers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CleanupTimerHandle,
			this,
			&UDelveDeepStatsComponent::CleanupExpiredModifiers,
			1.0f, // Check every second
			true  // Loop
		);
	}

	UE_LOG(LogDelveDeepStats, Display, 
		TEXT("Stats initialized: Health=%.2f, Resource=%.2f, Damage=%.2f, MoveSpeed=%.2f"),
		BaseHealth, BaseResource, BaseDamage, BaseMoveSpeed);
}

bool UDelveDeepStatsComponent::ValidateComponent(FDelveDeepValidationContext& Context) const
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

void UDelveDeepStatsComponent::AddStatModifier(FName StatName, float Modifier, float Duration)
{
	if (StatName.IsNone())
	{
		UE_LOG(LogDelveDeepStats, Warning, TEXT("Attempted to add modifier with empty stat name"));
		return;
	}

	// Create new modifier
	FDelveDeepStatModifier NewModifier(Modifier, Duration);
	
	// Add or update modifier in map
	ActiveModifiers.Add(StatName, NewModifier);
	
	// Mark stats as dirty for recalculation
	bStatsDirty = true;
	
	// Recalculate stats immediately
	RecalculateStats();
	
	UE_LOG(LogDelveDeepStats, Verbose, 
		TEXT("Added stat modifier: %s = %.2f for %.2f seconds"), 
		*StatName.ToString(), Modifier, Duration);
}

void UDelveDeepStatsComponent::RemoveStatModifier(FName StatName)
{
	if (ActiveModifiers.Remove(StatName) > 0)
	{
		// Mark stats as dirty for recalculation
		bStatsDirty = true;
		
		// Recalculate stats immediately
		RecalculateStats();
		
		UE_LOG(LogDelveDeepStats, Verbose, 
			TEXT("Removed stat modifier: %s"), *StatName.ToString());
	}
}

void UDelveDeepStatsComponent::ClearAllModifiers()
{
	if (ActiveModifiers.Num() > 0)
	{
		ActiveModifiers.Empty();
		
		// Mark stats as dirty for recalculation
		bStatsDirty = true;
		
		// Recalculate stats immediately
		RecalculateStats();
		
		UE_LOG(LogDelveDeepStats, Display, TEXT("Cleared all stat modifiers"));
	}
}

float UDelveDeepStatsComponent::GetModifiedStat(FName StatName) const
{
	// Return cached values if stats are clean
	if (!bStatsDirty)
	{
		if (StatName == FName("MaxHealth"))
		{
			return CachedMaxHealth;
		}
		else if (StatName == FName("MaxResource"))
		{
			return CachedMaxResource;
		}
		else if (StatName == FName("MoveSpeed"))
		{
			return CachedMoveSpeed;
		}
		else if (StatName == FName("Damage"))
		{
			return ApplyModifiers(StatName, BaseDamage);
		}
	}
	
	// Recalculate if dirty (shouldn't happen in const function, but handle it)
	if (StatName == FName("MaxHealth"))
	{
		return ApplyModifiers(StatName, BaseHealth);
	}
	else if (StatName == FName("MaxResource"))
	{
		return ApplyModifiers(StatName, BaseResource);
	}
	else if (StatName == FName("MoveSpeed"))
	{
		return ApplyModifiers(StatName, BaseMoveSpeed);
	}
	else if (StatName == FName("Damage"))
	{
		return ApplyModifiers(StatName, BaseDamage);
	}
	
	return 0.0f;
}

void UDelveDeepStatsComponent::RecalculateStats()
{
	SCOPE_CYCLE_COUNTER(STAT_StatsRecalculate);
	if (!bStatsDirty)
	{
		return;
	}
	
	// Store old values
	float OldMaxHealth = MaxHealth;
	float OldMaxResource = MaxResource;
	
	// Recalculate max stats with modifiers
	CachedMaxHealth = ApplyModifiers(FName("MaxHealth"), BaseHealth);
	CachedMaxResource = ApplyModifiers(FName("MaxResource"), BaseResource);
	CachedMoveSpeed = ApplyModifiers(FName("MoveSpeed"), BaseMoveSpeed);
	
	// Update actual max values
	MaxHealth = CachedMaxHealth;
	MaxResource = CachedMaxResource;
	
	// Clamp current values to new maximums
	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
	if (CurrentResource > MaxResource)
	{
		CurrentResource = MaxResource;
	}
	
	// Update character movement speed if we have a character owner
	if (ADelveDeepCharacter* Character = Cast<ADelveDeepCharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->MaxWalkSpeed = CachedMoveSpeed;
		}
	}
	
	// Mark stats as clean
	bStatsDirty = false;
	
	// Broadcast events if max values changed
	if (!FMath::IsNearlyEqual(OldMaxHealth, MaxHealth))
	{
		OnStatChanged(FName("MaxHealth"), OldMaxHealth, MaxHealth);
	}
	
	if (!FMath::IsNearlyEqual(OldMaxResource, MaxResource))
	{
		OnStatChanged(FName("MaxResource"), OldMaxResource, MaxResource);
	}
	
	UE_LOG(LogDelveDeepStats, Verbose, 
		TEXT("Stats recalculated: MaxHealth=%.2f, MaxResource=%.2f, MoveSpeed=%.2f"), 
		MaxHealth, MaxResource, CachedMoveSpeed);
}

float UDelveDeepStatsComponent::ApplyModifiers(FName StatName, float BaseValue) const
{
	float ModifiedValue = BaseValue;
	
	// Apply all modifiers for this stat
	for (const auto& ModifierPair : ActiveModifiers)
	{
		if (ModifierPair.Key == StatName)
		{
			// Additive modifier
			ModifiedValue += ModifierPair.Value.Modifier;
		}
	}
	
	// Ensure modified value is not negative
	return FMath::Max(ModifiedValue, 0.0f);
}

void UDelveDeepStatsComponent::CleanupExpiredModifiers()
{
	if (ActiveModifiers.Num() == 0)
	{
		return;
	}
	
	bool bAnyExpired = false;
	TArray<FName> ExpiredModifiers;
	
	// Find expired modifiers
	for (auto& ModifierPair : ActiveModifiers)
	{
		FDelveDeepStatModifier& Modifier = ModifierPair.Value;
		
		// Skip permanent modifiers (Duration == 0)
		if (Modifier.Duration <= 0.0f)
		{
			continue;
		}
		
		// Decrease remaining time
		Modifier.RemainingTime -= 1.0f; // Timer runs every second
		
		// Check if expired
		if (Modifier.RemainingTime <= 0.0f)
		{
			ExpiredModifiers.Add(ModifierPair.Key);
			bAnyExpired = true;
		}
	}
	
	// Remove expired modifiers
	if (bAnyExpired)
	{
		for (const FName& StatName : ExpiredModifiers)
		{
			ActiveModifiers.Remove(StatName);
			UE_LOG(LogDelveDeepStats, Verbose, 
				TEXT("Stat modifier expired: %s"), *StatName.ToString());
		}
		
		// Mark stats as dirty and recalculate
		bStatsDirty = true;
		const_cast<UDelveDeepStatsComponent*>(this)->RecalculateStats();
	}
}
