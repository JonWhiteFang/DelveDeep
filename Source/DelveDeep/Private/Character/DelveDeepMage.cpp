// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepMage.h"
#include "Character/DelveDeepStatsComponent.h"
#include "DelveDeepLogChannels.h"
#include "TimerManager.h"

ADelveDeepMage::ADelveDeepMage()
{
	// Set character class name for data asset lookup
	CharacterClassName = FName(TEXT("Mage"));
}

void ADelveDeepMage::BeginPlay()
{
	Super::BeginPlay();

	// Start Mana regeneration
	StartManaRegeneration();
}

void ADelveDeepMage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop Mana regeneration timer
	if (GetWorld() && ManaRegenTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ManaRegenTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ADelveDeepMage::StartManaRegeneration()
{
	if (!GetWorld())
	{
		UE_LOG(LogDelveDeep, Warning, 
			TEXT("Mage '%s' cannot start Mana regeneration - no world"), 
			*GetName());
		return;
	}

	// Set up timer for Mana regeneration (0.1 second intervals)
	GetWorld()->GetTimerManager().SetTimer(
		ManaRegenTimerHandle,
		[this]()
		{
			RegenerateMana(0.1f);
		},
		0.1f,
		true  // Loop
	);

	UE_LOG(LogDelveDeep, Display, 
		TEXT("Mage '%s' started Mana regeneration"), 
		*GetName());
}

void ADelveDeepMage::RegenerateMana(float DeltaTime)
{
	if (!StatsComponent)
	{
		return;
	}

	// Don't regenerate if already at max
	if (StatsComponent->GetCurrentResource() >= StatsComponent->GetMaxResource())
	{
		return;
	}

	// Calculate regeneration amount
	float RegenAmount = ManaRegenRate * DeltaTime;

	// Add Mana (ModifyResource will clamp to MaxMana)
	StatsComponent->ModifyResource(RegenAmount);

	UE_LOG(LogDelveDeep, VeryVerbose, 
		TEXT("Mage '%s' regenerated %.2f Mana (Current: %.2f/%.2f)"), 
		*GetName(), RegenAmount, StatsComponent->GetCurrentResource(), StatsComponent->GetMaxResource());
}

void ADelveDeepMage::CastFireball(FVector TargetLocation)
{
	// Placeholder for future combat system integration
	UE_LOG(LogDelveDeep, Display, 
		TEXT("Mage '%s' casts Fireball at location (%.2f, %.2f, %.2f) (placeholder)"), 
		*GetName(), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);

	// TODO: Implement fireball when combat system is available
	// - Check Mana cost
	// - Spawn fireball projectile
	// - Projectile travels to target location
	// - Explodes dealing AoE damage
	// - Consume Mana
}

void ADelveDeepMage::OnResourceChanged(float OldValue, float NewValue)
{
	// Log Mana changes for debugging
	UE_LOG(LogDelveDeep, VeryVerbose, 
		TEXT("Mage '%s' Mana changed: %.2f -> %.2f"), 
		*GetName(), OldValue, NewValue);

	// TODO: Trigger visual effects for Mana changes
	// - Update Mana bar UI
	// - Glow effect when casting spells
	// - Particle effects when Mana is consumed
}
