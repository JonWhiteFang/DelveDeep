// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepWarrior.h"
#include "Character/DelveDeepStatsComponent.h"
#include "DelveDeepLogChannels.h"

ADelveDeepWarrior::ADelveDeepWarrior()
{
	// Set character class name for data asset lookup
	CharacterClassName = FName(TEXT("Warrior"));
}

void ADelveDeepWarrior::GenerateRage(float Amount)
{
	if (!StatsComponent)
	{
		UE_LOG(LogDelveDeep, Warning, TEXT("Warrior '%s' has no stats component"), *GetName());
		return;
	}

	// Validate amount
	if (Amount < 0.0f)
	{
		UE_LOG(LogDelveDeep, Warning, 
			TEXT("Warrior '%s' attempted to generate negative Rage: %.2f"), 
			*GetName(), Amount);
		return;
	}

	// Add Rage (ModifyResource will clamp to MaxRage)
	StatsComponent->ModifyResource(Amount);

	UE_LOG(LogDelveDeep, Verbose, 
		TEXT("Warrior '%s' generated %.2f Rage (Current: %.2f/%.2f)"), 
		*GetName(), Amount, StatsComponent->GetCurrentResource(), StatsComponent->GetMaxResource());
}

void ADelveDeepWarrior::PerformCleaveAttack()
{
	// Placeholder for future combat system integration
	UE_LOG(LogDelveDeep, Display, 
		TEXT("Warrior '%s' performs Cleave Attack (placeholder)"), 
		*GetName());

	// TODO: Implement cleave attack when combat system is available
	// - Check Rage cost
	// - Find enemies in front arc
	// - Deal damage to all enemies in range
	// - Consume Rage
}

void ADelveDeepWarrior::TakeDamage(float DamageAmount, AActor* DamageSource)
{
	// Call parent implementation to apply damage
	Super::TakeDamage(DamageAmount, DamageSource);

	// Generate Rage from taking damage
	float RageGenerated = DamageAmount * (RagePerDamageTaken / 100.0f);
	GenerateRage(RageGenerated);

	UE_LOG(LogDelveDeep, Verbose, 
		TEXT("Warrior '%s' generated %.2f Rage from taking %.2f damage"), 
		*GetName(), RageGenerated, DamageAmount);
}

void ADelveDeepWarrior::OnResourceChanged(float OldValue, float NewValue)
{
	// Log Rage changes for debugging
	UE_LOG(LogDelveDeep, Verbose, 
		TEXT("Warrior '%s' Rage changed: %.2f -> %.2f"), 
		*GetName(), OldValue, NewValue);

	// TODO: Trigger visual effects for Rage changes
	// - Glow effect when Rage is high
	// - Particle effects when Rage is gained
}
