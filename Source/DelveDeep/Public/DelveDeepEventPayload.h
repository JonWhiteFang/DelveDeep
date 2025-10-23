// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DelveDeepValidation.h"
#include "DelveDeepEventPayload.generated.h"

/**
 * Base structure for all event payloads in the DelveDeep event system.
 * Contains common fields shared by all events: EventTag, Timestamp, and Instigator.
 * Derived payload types add specific data relevant to their event type.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The GameplayTag identifying this event type (e.g., "DelveDeep.Event.Combat.Damage.Dealt") */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FGameplayTag EventTag;

	/** Timestamp when the event was created */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FDateTime Timestamp;

	/** The actor that instigated this event (may be null for system events) */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	TWeakObjectPtr<AActor> Instigator;

	FDelveDeepEventPayload()
		: Timestamp(FDateTime::Now())
	{}

	virtual ~FDelveDeepEventPayload() = default;

	/**
	 * Validates the event payload data.
	 * @param Context Validation context for error/warning tracking
	 * @return True if payload is valid, false otherwise
	 */
	virtual bool Validate(FValidationContext& Context) const;

protected:
	/** Helper method to validate actor references */
	bool ValidateActorReference(const TWeakObjectPtr<AActor>& Actor, const FString& ActorName, FValidationContext& Context) const;

	/** Helper method to validate numeric ranges */
	bool ValidateRange(float Value, float MinValue, float MaxValue, const FString& ValueName, FValidationContext& Context) const;
};

/**
 * Event payload for damage events.
 * Contains information about damage dealt, including attacker, victim, amount, and type.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepDamageEventPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The actor dealing the damage */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	TWeakObjectPtr<AActor> Attacker;

	/** The actor receiving the damage */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	TWeakObjectPtr<AActor> Victim;

	/** Amount of damage dealt */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float DamageAmount = 0.0f;

	/** Type of damage (e.g., Physical, Fire, Ice) */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayTag DamageType;

	virtual bool Validate(FValidationContext& Context) const override;
};

/**
 * Event payload for health change events.
 * Contains information about character health changes, including previous and new values.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepHealthChangeEventPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The character whose health changed */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	TWeakObjectPtr<AActor> Character;

	/** Health value before the change */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float PreviousHealth = 0.0f;

	/** Health value after the change */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float NewHealth = 0.0f;

	/** Maximum health of the character */
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float MaxHealth = 0.0f;

	virtual bool Validate(FValidationContext& Context) const override;
};

/**
 * Event payload for kill events.
 * Contains information about enemy kills, including killer, victim, and experience awarded.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepKillEventPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The actor that performed the kill */
	UPROPERTY(BlueprintReadOnly, Category = "Kill")
	TWeakObjectPtr<AActor> Killer;

	/** The actor that was killed */
	UPROPERTY(BlueprintReadOnly, Category = "Kill")
	TWeakObjectPtr<AActor> Victim;

	/** Experience points awarded for the kill */
	UPROPERTY(BlueprintReadOnly, Category = "Kill")
	int32 ExperienceAwarded = 0;

	/** Type of victim (e.g., Monster, Boss, Elite) */
	UPROPERTY(BlueprintReadOnly, Category = "Kill")
	FGameplayTag VictimType;

	virtual bool Validate(FValidationContext& Context) const override;
};

/**
 * Event payload for attack events.
 * Contains information about attacks, including attacker, location, type, and radius.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepAttackEventPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The actor performing the attack */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	TWeakObjectPtr<AActor> Attacker;

	/** Location where the attack occurred */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	FVector AttackLocation = FVector::ZeroVector;

	/** Type of attack (e.g., Melee, Ranged, Ability) */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	FGameplayTag AttackType;

	/** Radius of the attack effect (0 for single-target) */
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	float AttackRadius = 0.0f;

	virtual bool Validate(FValidationContext& Context) const override;
};
