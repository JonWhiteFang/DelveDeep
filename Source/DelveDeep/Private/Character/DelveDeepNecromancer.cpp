// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepNecromancer.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Events/DelveDeepEventSubsystem.h"
#include "DelveDeepLogChannels.h"
#include "GameplayTagContainer.h"

ADelveDeepNecromancer::ADelveDeepNecromancer()
{
	// Set character class name for data asset lookup
	CharacterClassName = FName(TEXT("Necromancer"));
}

void ADelveDeepNecromancer::BeginPlay()
{
	Super::BeginPlay();

	// Register for enemy death events
	RegisterForEnemyDeathEvents();
}

void ADelveDeepNecromancer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from enemy death events
	UnregisterFromEnemyDeathEvents();

	// Clean up active minions
	ActiveMinions.Empty();

	Super::EndPlay(EndPlayReason);
}

void ADelveDeepNecromancer::RegisterForEnemyDeathEvents()
{
	// Get event subsystem
	UDelveDeepEventSubsystem* EventSubsystem = nullptr;
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>();
	}

	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeep, Warning, 
			TEXT("Necromancer '%s' cannot register for enemy death events - no event subsystem"), 
			*GetName());
		return;
	}

	// Register listener for enemy death events
	FGameplayTag EnemyDeathTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Enemy.Death"));
	
	FDelveDeepEventDelegate Delegate;
	Delegate.BindUObject(this, &ADelveDeepNecromancer::OnEnemyDeath);
	
	EnemyDeathEventHandle = EventSubsystem->RegisterListener(
		EnemyDeathTag,
		Delegate,
		EDelveDeepEventPriority::Normal
	);

	UE_LOG(LogDelveDeep, Display, 
		TEXT("Necromancer '%s' registered for enemy death events"), 
		*GetName());
}

void ADelveDeepNecromancer::UnregisterFromEnemyDeathEvents()
{
	if (!EnemyDeathEventHandle.IsValid())
	{
		return;
	}

	// Get event subsystem
	UDelveDeepEventSubsystem* EventSubsystem = nullptr;
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		EventSubsystem = GameInstance->GetSubsystem<UDelveDeepEventSubsystem>();
	}

	if (EventSubsystem)
	{
		FGameplayTag EnemyDeathTag = FGameplayTag::RequestGameplayTag(TEXT("DelveDeep.Enemy.Death"));
		EventSubsystem->UnregisterListener(EnemyDeathTag, EnemyDeathEventHandle);

		UE_LOG(LogDelveDeep, Display, 
			TEXT("Necromancer '%s' unregistered from enemy death events"), 
			*GetName());
	}

	EnemyDeathEventHandle.Reset();
}

void ADelveDeepNecromancer::OnEnemyDeath(const FDelveDeepEventPayload& Payload)
{
	// Collect a Soul when an enemy dies
	CollectSoul();

	UE_LOG(LogDelveDeep, Verbose, 
		TEXT("Necromancer '%s' detected enemy death and collected Soul"), 
		*GetName());
}

void ADelveDeepNecromancer::CollectSoul()
{
	if (!StatsComponent)
	{
		UE_LOG(LogDelveDeep, Warning, TEXT("Necromancer '%s' has no stats component"), *GetName());
		return;
	}

	// Check if already at max Souls
	if (StatsComponent->GetCurrentResource() >= MaxSouls)
	{
		UE_LOG(LogDelveDeep, Verbose, 
			TEXT("Necromancer '%s' cannot collect Soul - already at maximum (%.0f/%.0f)"), 
			*GetName(), StatsComponent->GetCurrentResource(), MaxSouls);
		return;
	}

	// Add one Soul (ModifyResource will clamp to MaxSouls)
	StatsComponent->ModifyResource(1.0f);

	UE_LOG(LogDelveDeep, Display, 
		TEXT("Necromancer '%s' collected Soul (Current: %.0f/%.0f)"), 
		*GetName(), StatsComponent->GetCurrentResource(), MaxSouls);
}

void ADelveDeepNecromancer::SummonMinion()
{
	// Placeholder for future minion system integration
	UE_LOG(LogDelveDeep, Display, 
		TEXT("Necromancer '%s' summons minion (placeholder)"), 
		*GetName());

	// TODO: Implement minion summoning when minion system is available
	// - Check Soul cost (e.g., 3 Souls per minion)
	// - Check if at max minion count
	// - Spawn minion actor
	// - Add to ActiveMinions array
	// - Consume Souls

	// Check if at max minions
	if (ActiveMinions.Num() >= MaxMinions)
	{
		UE_LOG(LogDelveDeep, Warning, 
			TEXT("Necromancer '%s' cannot summon minion - already at maximum (%d/%d)"), 
			*GetName(), ActiveMinions.Num(), MaxMinions);
		return;
	}

	// Check if enough Souls (placeholder cost: 3 Souls)
	const float SoulCost = 3.0f;
	if (StatsComponent && StatsComponent->GetCurrentResource() < SoulCost)
	{
		UE_LOG(LogDelveDeep, Warning, 
			TEXT("Necromancer '%s' cannot summon minion - not enough Souls (%.0f/%.0f required)"), 
			*GetName(), StatsComponent->GetCurrentResource(), SoulCost);
		return;
	}

	// Consume Souls
	if (StatsComponent)
	{
		StatsComponent->ModifyResource(-SoulCost);
	}
}

void ADelveDeepNecromancer::OnResourceChanged(float OldValue, float NewValue)
{
	// Log Soul changes for debugging
	UE_LOG(LogDelveDeep, Verbose, 
		TEXT("Necromancer '%s' Souls changed: %.0f -> %.0f"), 
		*GetName(), OldValue, NewValue);

	// TODO: Trigger visual effects for Soul changes
	// - Update Soul counter UI
	// - Particle effects when Souls are collected
	// - Visual indicator when enough Souls for summoning
}
