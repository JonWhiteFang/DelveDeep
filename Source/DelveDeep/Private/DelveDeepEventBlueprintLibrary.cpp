// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventBlueprintLibrary.h"
#include "DelveDeepEventSubsystem.h"
#include "DelveDeepEventPayload.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UDelveDeepEventSubsystem* UDelveDeepEventBlueprintLibrary::GetEventSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("GetEventSubsystem: Invalid WorldContextObject"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("GetEventSubsystem: Failed to get world from context object"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogDelveDeepEvents, Warning, TEXT("GetEventSubsystem: No game instance available"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<UDelveDeepEventSubsystem>();
}

void UDelveDeepEventBlueprintLibrary::BroadcastDamageEvent(
	UObject* WorldContextObject,
	AActor* Attacker,
	AActor* Victim,
	float DamageAmount,
	FGameplayTag DamageType)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem(WorldContextObject);
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("BroadcastDamageEvent: Event subsystem not available"));
		return;
	}

	// Create damage event payload
	FDelveDeepDamageEventPayload Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Damage.Dealt"));
	Payload.Timestamp = FDateTime::Now();
	Payload.Instigator = Attacker;
	Payload.Attacker = Attacker;
	Payload.Victim = Victim;
	Payload.DamageAmount = DamageAmount;
	Payload.DamageType = DamageType;

	// Broadcast the event
	EventSubsystem->BroadcastEvent(Payload);
}

void UDelveDeepEventBlueprintLibrary::BroadcastHealthChangeEvent(
	UObject* WorldContextObject,
	AActor* Character,
	float PreviousHealth,
	float NewHealth,
	float MaxHealth)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem(WorldContextObject);
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("BroadcastHealthChangeEvent: Event subsystem not available"));
		return;
	}

	// Create health change event payload
	FDelveDeepHealthChangeEventPayload Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Character.Health.Changed"));
	Payload.Timestamp = FDateTime::Now();
	Payload.Instigator = Character;
	Payload.Character = Character;
	Payload.PreviousHealth = PreviousHealth;
	Payload.NewHealth = NewHealth;
	Payload.MaxHealth = MaxHealth;

	// Broadcast the event
	EventSubsystem->BroadcastEvent(Payload);
}

void UDelveDeepEventBlueprintLibrary::BroadcastKillEvent(
	UObject* WorldContextObject,
	AActor* Killer,
	AActor* Victim,
	int32 ExperienceAwarded,
	FGameplayTag VictimType)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem(WorldContextObject);
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("BroadcastKillEvent: Event subsystem not available"));
		return;
	}

	// Create kill event payload
	FDelveDeepKillEventPayload Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Kill.Player"));
	Payload.Timestamp = FDateTime::Now();
	Payload.Instigator = Killer;
	Payload.Killer = Killer;
	Payload.Victim = Victim;
	Payload.ExperienceAwarded = ExperienceAwarded;
	Payload.VictimType = VictimType;

	// Broadcast the event
	EventSubsystem->BroadcastEvent(Payload);
}

void UDelveDeepEventBlueprintLibrary::BroadcastAttackEvent(
	UObject* WorldContextObject,
	AActor* Attacker,
	FVector AttackLocation,
	FGameplayTag AttackType,
	float AttackRadius)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem(WorldContextObject);
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("BroadcastAttackEvent: Event subsystem not available"));
		return;
	}

	// Create attack event payload
	FDelveDeepAttackEventPayload Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("DelveDeep.Event.Combat.Attack"));
	Payload.Timestamp = FDateTime::Now();
	Payload.Instigator = Attacker;
	Payload.Attacker = Attacker;
	Payload.AttackLocation = AttackLocation;
	Payload.AttackType = AttackType;
	Payload.AttackRadius = AttackRadius;

	// Broadcast the event
	EventSubsystem->BroadcastEvent(Payload);
}

FDelegateHandle UDelveDeepEventBlueprintLibrary::RegisterListenerBlueprint(
	UObject* WorldContextObject,
	FGameplayTag EventTag,
	FDelveDeepEventDelegate Callback,
	UObject* Owner,
	EDelveDeepEventPriority Priority)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem(WorldContextObject);
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("RegisterListenerBlueprint: Event subsystem not available"));
		return FDelegateHandle();
	}

	if (!Callback.IsBound())
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("RegisterListenerBlueprint: Callback is not bound"));
		return FDelegateHandle();
	}

	// Create a lambda that wraps the dynamic delegate
	TFunction<void(const FDelveDeepEventPayload&)> CallbackWrapper = 
		[Callback](const FDelveDeepEventPayload& Payload)
		{
			// Execute the dynamic delegate
			if (Callback.IsBound())
			{
				Callback.Execute(Payload);
			}
		};

	// Register the listener with the event subsystem
	return EventSubsystem->RegisterListener(EventTag, CallbackWrapper, Owner, Priority);
}

void UDelveDeepEventBlueprintLibrary::UnregisterListenerBlueprint(
	UObject* WorldContextObject,
	FDelegateHandle Handle)
{
	UDelveDeepEventSubsystem* EventSubsystem = GetEventSubsystem(WorldContextObject);
	if (!EventSubsystem)
	{
		UE_LOG(LogDelveDeepEvents, Error, TEXT("UnregisterListenerBlueprint: Event subsystem not available"));
		return;
	}

	EventSubsystem->UnregisterListener(Handle);
}
