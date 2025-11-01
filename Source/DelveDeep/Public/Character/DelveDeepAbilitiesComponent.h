// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/DelveDeepCharacterComponent.h"
#include "DelveDeepAbilitiesComponent.generated.h"

class UDelveDeepCharacterData;
class UDelveDeepAbilityData;
struct FDelveDeepValidationContext;

/**
 * Abilities component managing character abilities and cooldowns.
 * Placeholder implementation for character system foundation.
 */
UCLASS(BlueprintType, ClassGroup = (DelveDeep), meta = (BlueprintSpawnableComponent))
class DELVEDEEP_API UDelveDeepAbilitiesComponent : public UDelveDeepCharacterComponent
{
	GENERATED_BODY()

public:
	UDelveDeepAbilitiesComponent();

	// Component lifecycle
	virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* Data) override;
	virtual bool ValidateComponent(FDelveDeepValidationContext& Context) const override;

	// Ability management
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Abilities")
	void AddAbility(const UDelveDeepAbilityData* Ability);

	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Abilities")
	void RemoveAbility(const UDelveDeepAbilityData* Ability);

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Abilities")
	int32 GetAbilityCount() const { return Abilities.Num(); }

	// Ability usage (placeholder)
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character|Abilities")
	bool UseAbility(int32 AbilityIndex);

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character|Abilities")
	bool CanUseAbility(int32 AbilityIndex) const;

protected:
	/** Array of ability references loaded from character data */
	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character|Abilities")
	TArray<const UDelveDeepAbilityData*> Abilities;

	/** Map tracking ability cooldowns */
	UPROPERTY()
	TMap<const UDelveDeepAbilityData*, float> AbilityCooldowns;
};
