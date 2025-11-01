// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepRanger.h"
#include "Character/DelveDeepStatsComponent.h"
#include "DelveDeepLogChannels.h"
#include "TimerManager.h"

ADelveDeepRanger::ADelveDeepRanger()
{
	// Set character class name for data asset lookup
	CharacterClassName = FName(TEXT("Ranger"));
}

void ADelveDeepRanger::BeginPlay()
{
	Super::BeginPlay();

	// Start Energy regeneration
	StartEnergyRegeneration();
}

void ADelveDeepRanger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop Energy regeneration timer
	if (GetWorld() && EnergyRegenTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(EnergyRegenTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ADelveDeepRanger::StartEnergyRegeneration()
{
	if (!GetWorld())
	{
		UE_LOG(LogDelveDeep, Warning, 
			TEXT("Ranger '%s' cannot start Energy regeneration - no world"), 
			*GetName());
		return;
	}

	// Set up timer for Energy regeneration (0.1 second intervals)
	GetWorld()->GetTimerManager().SetTimer(
		EnergyRegenTimerHandle,
		[this]()
		{
			RegenerateEnergy(0.1f);
		},
		0.1f,
		true  // Loop
	);

	UE_LOG(LogDelveDeep, Display, 
		TEXT("Ranger '%s' started Energy regeneration"), 
		*GetName());
}

void ADelveDeepRanger::RegenerateEnergy(float DeltaTime)
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
	float RegenAmount = EnergyRegenRate * DeltaTime;

	// Add Energy (ModifyResource will clamp to MaxEnergy)
	StatsComponent->ModifyResource(RegenAmount);

	UE_LOG(LogDelveDeep, VeryVerbose, 
		TEXT("Ranger '%s' regenerated %.2f Energy (Current: %.2f/%.2f)"), 
		*GetName(), RegenAmount, StatsComponent->GetCurrentResource(), StatsComponent->GetMaxResource());
}

void ADelveDeepRanger::PerformPiercingShot()
{
	// Placeholder for future combat system integration
	UE_LOG(LogDelveDeep, Display, 
		TEXT("Ranger '%s' performs Piercing Shot (placeholder)"), 
		*GetName());

	// TODO: Implement piercing shot when combat system is available
	// - Check Energy cost
	// - Spawn projectile
	// - Projectile pierces through multiple enemies
	// - Consume Energy
}

void ADelveDeepRanger::OnResourceChanged(float OldValue, float NewValue)
{
	// Log Energy changes for debugging
	UE_LOG(LogDelveDeep, VeryVerbose, 
		TEXT("Ranger '%s' Energy changed: %.2f -> %.2f"), 
		*GetName(), OldValue, NewValue);

	// TODO: Trigger visual effects for Energy changes
	// - Update Energy bar UI
	// - Particle effects when Energy is consumed
}
