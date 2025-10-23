// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventPayload.h"

bool FDelveDeepEventPayload::Validate(FValidationContext& Context) const
{
	Context.SystemName = TEXT("EventSystem");
	Context.OperationName = TEXT("ValidateEventPayload");

	bool bIsValid = true;

	// Validate EventTag
	if (!EventTag.IsValid())
	{
		Context.AddError(TEXT("EventTag is not valid"));
		bIsValid = false;
	}

	// Timestamp is always valid (set in constructor)
	// Instigator is optional (can be null for system events)

	return bIsValid;
}

bool FDelveDeepEventPayload::ValidateActorReference(const TWeakObjectPtr<AActor>& Actor, const FString& ActorName, FValidationContext& Context) const
{
	if (!Actor.IsValid())
	{
		Context.AddError(FString::Printf(TEXT("%s reference is invalid"), *ActorName));
		return false;
	}

	return true;
}

bool FDelveDeepDamageEventPayload::Validate(FValidationContext& Context) const
{
	Context.SystemName = TEXT("EventSystem");
	Context.OperationName = TEXT("ValidateDamageEvent");

	bool bIsValid = FDelveDeepEventPayload::Validate(Context);

	if (!ValidateActorReference(Attacker, TEXT("Attacker"), Context))
	{
		bIsValid = false;
	}

	if (!ValidateActorReference(Victim, TEXT("Victim"), Context))
	{
		bIsValid = false;
	}

	if (DamageAmount < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Damage amount is negative: %.2f"), DamageAmount));
		bIsValid = false;
	}

	if (DamageAmount > 100000.0f)
	{
		Context.AddWarning(FString::Printf(TEXT("Unusually high damage amount: %.2f"), DamageAmount));
	}

	if (!DamageType.IsValid())
	{
		Context.AddWarning(TEXT("Damage type tag is not set"));
	}

	return bIsValid;
}

bool FDelveDeepHealthChangeEventPayload::Validate(FValidationContext& Context) const
{
	Context.SystemName = TEXT("EventSystem");
	Context.OperationName = TEXT("ValidateHealthChangeEvent");

	bool bIsValid = FDelveDeepEventPayload::Validate(Context);

	if (!ValidateActorReference(Character, TEXT("Character"), Context))
	{
		bIsValid = false;
	}

	if (PreviousHealth < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Previous health is negative: %.2f"), PreviousHealth));
		bIsValid = false;
	}

	if (NewHealth < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("New health is negative: %.2f"), NewHealth));
		bIsValid = false;
	}

	if (MaxHealth <= 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Max health must be positive: %.2f"), MaxHealth));
		bIsValid = false;
	}

	if (PreviousHealth > MaxHealth)
	{
		Context.AddWarning(FString::Printf(
			TEXT("Previous health (%.2f) exceeds max health (%.2f)"), 
			PreviousHealth, MaxHealth));
	}

	if (NewHealth > MaxHealth)
	{
		Context.AddWarning(FString::Printf(
			TEXT("New health (%.2f) exceeds max health (%.2f)"), 
			NewHealth, MaxHealth));
	}

	return bIsValid;
}

bool FDelveDeepKillEventPayload::Validate(FValidationContext& Context) const
{
	Context.SystemName = TEXT("EventSystem");
	Context.OperationName = TEXT("ValidateKillEvent");

	bool bIsValid = FDelveDeepEventPayload::Validate(Context);

	if (!ValidateActorReference(Killer, TEXT("Killer"), Context))
	{
		bIsValid = false;
	}

	if (!ValidateActorReference(Victim, TEXT("Victim"), Context))
	{
		bIsValid = false;
	}

	if (ExperienceAwarded < 0)
	{
		Context.AddError(FString::Printf(TEXT("Experience awarded is negative: %d"), ExperienceAwarded));
		bIsValid = false;
	}

	if (ExperienceAwarded == 0)
	{
		Context.AddWarning(TEXT("No experience awarded for kill"));
	}

	if (!VictimType.IsValid())
	{
		Context.AddWarning(TEXT("Victim type tag is not set"));
	}

	return bIsValid;
}

bool FDelveDeepAttackEventPayload::Validate(FValidationContext& Context) const
{
	Context.SystemName = TEXT("EventSystem");
	Context.OperationName = TEXT("ValidateAttackEvent");

	bool bIsValid = FDelveDeepEventPayload::Validate(Context);

	if (!ValidateActorReference(Attacker, TEXT("Attacker"), Context))
	{
		bIsValid = false;
	}

	if (AttackRadius < 0.0f)
	{
		Context.AddError(FString::Printf(TEXT("Attack radius is negative: %.2f"), AttackRadius));
		bIsValid = false;
	}

	if (AttackRadius > 10000.0f)
	{
		Context.AddWarning(FString::Printf(TEXT("Unusually large attack radius: %.2f"), AttackRadius));
	}

	if (!AttackType.IsValid())
	{
		Context.AddWarning(TEXT("Attack type tag is not set"));
	}

	return bIsValid;
}

bool FDelveDeepEventPayload::ValidateRange(float Value, float MinValue, float MaxValue, const FString& ValueName, FValidationContext& Context) const
{
	if (Value < MinValue || Value > MaxValue)
	{
		Context.AddError(FString::Printf(
			TEXT("%s out of range: %.2f (expected %.2f-%.2f)"), 
			*ValueName, Value, MinValue, MaxValue));
		return false;
	}

	return true;
}
