// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "DelveDeepEventBlueprintLibrary.generated.h"

class UDelveDeepEventSubsystem;
class AActor;

/**
 * Dynamic delegate for Blueprint event callbacks.
 * Used to receive event notifications in Blueprint.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelveDeepEventDelegate, const struct FDelveDeepEventPayload&, Payload);

/**
 * Blueprint function library for the DelveDeep Event System.
 * Provides Blueprint-friendly access to event system functionality.
 */
UCLASS()
class DELVEDEEP_API UDelveDeepEventBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets the event subsystem from the world context.
	 * @param WorldContextObject Object providing world context (usually 'self' in Blueprint)
	 * @return The event subsystem, or nullptr if not available
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Events", 
		meta = (WorldContext = "WorldContextObject"))
	static UDelveDeepEventSubsystem* GetEventSubsystem(const UObject* WorldContextObject);

	/**
	 * Broadcasts a damage event to all registered listeners.
	 * @param WorldContextObject Object providing world context
	 * @param Attacker The actor dealing damage
	 * @param Victim The actor receiving damage
	 * @param DamageAmount Amount of damage dealt
	 * @param DamageType GameplayTag identifying the damage type
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events",
		meta = (WorldContext = "WorldContextObject"))
	static void BroadcastDamageEvent(
		UObject* WorldContextObject,
		AActor* Attacker,
		AActor* Victim,
		float DamageAmount,
		FGameplayTag DamageType);

	/**
	 * Broadcasts a health change event to all registered listeners.
	 * @param WorldContextObject Object providing world context
	 * @param Character The character whose health changed
	 * @param PreviousHealth Health value before the change
	 * @param NewHealth Health value after the change
	 * @param MaxHealth Maximum health value
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events",
		meta = (WorldContext = "WorldContextObject"))
	static void BroadcastHealthChangeEvent(
		UObject* WorldContextObject,
		AActor* Character,
		float PreviousHealth,
		float NewHealth,
		float MaxHealth);

	/**
	 * Broadcasts a kill event to all registered listeners.
	 * @param WorldContextObject Object providing world context
	 * @param Killer The actor that performed the kill
	 * @param Victim The actor that was killed
	 * @param ExperienceAwarded Experience points awarded for the kill
	 * @param VictimType GameplayTag identifying the victim type
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events",
		meta = (WorldContext = "WorldContextObject"))
	static void BroadcastKillEvent(
		UObject* WorldContextObject,
		AActor* Killer,
		AActor* Victim,
		int32 ExperienceAwarded,
		FGameplayTag VictimType);

	/**
	 * Broadcasts an attack event to all registered listeners.
	 * @param WorldContextObject Object providing world context
	 * @param Attacker The actor performing the attack
	 * @param AttackLocation World location of the attack
	 * @param AttackType GameplayTag identifying the attack type
	 * @param AttackRadius Radius of the attack (for area attacks)
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events",
		meta = (WorldContext = "WorldContextObject"))
	static void BroadcastAttackEvent(
		UObject* WorldContextObject,
		AActor* Attacker,
		FVector AttackLocation,
		FGameplayTag AttackType,
		float AttackRadius = 0.0f);

	/**
	 * Registers a Blueprint listener for a specific event tag.
	 * Note: For C++ listeners, use UDelveDeepEventSubsystem::RegisterListener directly.
	 * @param WorldContextObject Object providing world context
	 * @param EventTag The GameplayTag identifying the event type to listen for
	 * @param Callback The delegate to invoke when the event is broadcast
	 * @param Owner The object registering the listener (for automatic cleanup)
	 * @param Priority The priority level for this listener
	 * @return Handle that can be used to unregister the listener
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events",
		meta = (WorldContext = "WorldContextObject"))
	static FDelegateHandle RegisterListenerBlueprint(
		UObject* WorldContextObject,
		FGameplayTag EventTag,
		FDelveDeepEventDelegate Callback,
		UObject* Owner,
		EDelveDeepEventPriority Priority = EDelveDeepEventPriority::Normal);

	/**
	 * Unregisters a Blueprint listener using its handle.
	 * @param WorldContextObject Object providing world context
	 * @param Handle The handle returned from RegisterListenerBlueprint
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Events",
		meta = (WorldContext = "WorldContextObject"))
	static void UnregisterListenerBlueprint(
		UObject* WorldContextObject,
		FDelegateHandle Handle);
};
