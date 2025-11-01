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

	/** Whether this event should be replicated over the network (for future multiplayer support) */
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool bNetworkRelevant = false;

	/** Whether this event should use reliable replication (for future multiplayer support) */
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool bReliable = true;

	FDelveDeepEventPayload()
		: Timestamp(FDateTime::Now())
	{}

	virtual ~FDelveDeepEventPayload() = default;

	/**
	 * Validates the event payload data.
	 * @param Context Validation context for error/warning tracking
	 * @return True if payload is valid, false otherwise
	 */
	virtual bool Validate(FDelveDeepValidationContext& Context) const;

	/**
	 * Determines if this event should be replicated over the network.
	 * @return True if the event should be replicated, false otherwise
	 */
	virtual bool ShouldReplicate() const { return bNetworkRelevant; }

	/**
	 * Serializes the event payload for network transmission.
	 * This is a stub for future multiplayer support.
	 * @param Ar Archive for serialization
	 */
	virtual void Serialize(FArchive& Ar) { /* Implement when network support is added */ }

protected:
	/** Helper method to validate actor references */
	bool ValidateActorReference(const TWeakObjectPtr<AActor>& Actor, const FString& ActorName, FDelveDeepValidationContext& Context) const;

	/** Helper method to validate numeric ranges */
	bool ValidateRange(float Value, float MinValue, float MaxValue, const FString& ValueName, FDelveDeepValidationContext& Context) const;
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

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
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

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
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

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
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

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
};

/**
 * Event payload for character death events.
 * Contains information about character deaths, including killer and death location.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepCharacterDeathEventPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The character that died */
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	TWeakObjectPtr<AActor> Character;

	/** The actor that killed the character (may be null for environmental deaths) */
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	TWeakObjectPtr<AActor> Killer;

	/** Location where the death occurred */
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	FVector DeathLocation = FVector::ZeroVector;

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
};



/**
 * Event payload for character stat changed events.
 * Contains information about stat changes, including stat name and old/new values.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepStatChangedPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The character whose stat changed */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	TWeakObjectPtr<AActor> Character;

	/** Name of the stat that changed (e.g., "Health", "Mana", "Damage") */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FName StatName;

	/** Stat value before the change */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float OldValue = 0.0f;

	/** Stat value after the change */
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float NewValue = 0.0f;

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
};

/**
 * Event payload for ability used events.
 * Contains information about ability usage, including character, ability, and resource cost.
 */
USTRUCT(BlueprintType)
struct DELVEDEEP_API FDelveDeepAbilityUsedPayload : public FDelveDeepEventPayload
{
	GENERATED_BODY()

	/** The character that used the ability */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TWeakObjectPtr<AActor> Character;

	/** The ability that was used (soft object pointer to ability data asset) */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	TSoftObjectPtr<class UDelveDeepAbilityData> Ability;

	/** Resource cost paid to use the ability */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	float ResourceCost = 0.0f;

	virtual bool Validate(FDelveDeepValidationContext& Context) const override;
};
