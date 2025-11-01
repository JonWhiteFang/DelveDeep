// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "DelveDeepCharacter.generated.h"

class UDelveDeepStatsComponent;
class UDelveDeepAbilitiesComponent;
class UDelveDeepEquipmentComponent;
class UDelveDeepCharacterData;
struct FValidationContext;

/**
 * Base character class for DelveDeep.
 * Inherits from APaperCharacter for 2D sprite support.
 * Uses component-based architecture for stats, abilities, and equipment.
 */
UCLASS(BlueprintType, Blueprintable, Category = "DelveDeep|Character")
class DELVEDEEP_API ADelveDeepCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ADelveDeepCharacter();

	// Character data loading
	virtual void BeginPlay() override;

	/**
	 * Initialize character from configuration data.
	 * Queries UDelveDeepConfigurationManager using CharacterClassName.
	 */
	UFUNCTION(BlueprintCallable, Category = "DelveDeep|Character")
	void InitializeFromData();

	/**
	 * Validate character data using FValidationContext.
	 * @param Context Validation context for error/warning tracking
	 * @return True if character data is valid
	 */
	bool ValidateCharacterData(FValidationContext& Context) const;

	// Component accessors
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	UDelveDeepStatsComponent* GetStatsComponent() const { return StatsComponent; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	UDelveDeepAbilitiesComponent* GetAbilitiesComponent() const { return AbilitiesComponent; }

	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	UDelveDeepEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

protected:
	/**
	 * Initialize all character components after data is loaded.
	 */
	void InitializeComponents();

	/**
	 * Name used to lookup character data from configuration manager.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	FName CharacterClassName;

	/**
	 * Stats component managing health, resource, damage, and move speed.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	UDelveDeepStatsComponent* StatsComponent;

	/**
	 * Abilities component managing character abilities and cooldowns.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	UDelveDeepAbilitiesComponent* AbilitiesComponent;

	/**
	 * Equipment component managing weapons and equipment.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DelveDeep|Character")
	UDelveDeepEquipmentComponent* EquipmentComponent;

	/**
	 * Cached reference to loaded character data.
	 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DelveDeep|Character")
	const UDelveDeepCharacterData* CharacterData;
};
