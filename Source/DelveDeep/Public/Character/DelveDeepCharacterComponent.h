// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DelveDeepCharacterComponent.generated.h"

// Forward declarations
class ADelveDeepCharacter;
class UDelveDeepCharacterData;
struct FDelveDeepValidationContext;

/**
 * Base component class for all character-specific functionality.
 * Provides common functionality for modular character systems through composition.
 * 
 * Design Philosophy:
 * - Component-based architecture for modularity and reusability
 * - Tick disabled by default for performance optimization
 * - Blueprint-ready design for designer extensibility
 * - Validation support using FDelveDeepValidationContext
 * - Integration with data-driven configuration system
 * 
 * Usage:
 * - Inherit from this class to create specialized character components
 * - Override InitializeFromCharacterData() to load component-specific data
 * - Override ValidateComponent() to implement component-specific validation
 * - Use GetCharacterOwner() to access the owning character actor
 * 
 * Example:
 * @code
 * UCLASS()
 * class UMyCharacterComponent : public UDelveDeepCharacterComponent
 * {
 *     GENERATED_BODY()
 * 
 * protected:
 *     virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData) override
 *     {
 *         // Load component-specific data from character data asset
 *     }
 * 
 *     virtual bool ValidateComponent(FDelveDeepValidationContext& Context) const override
 *     {
 *         // Validate component state
 *         return Super::ValidateComponent(Context);
 *     }
 * };
 * @endcode
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Category = "DelveDeep|Character", meta = (BlueprintSpawnableComponent))
class DELVEDEEP_API UDelveDeepCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Constructor - initializes component with tick disabled by default.
	 */
	UDelveDeepCharacterComponent();

	/**
	 * Gets the character actor that owns this component.
	 * Provides type-safe access to the owning character.
	 * 
	 * @return Pointer to the owning ADelveDeepCharacter, or nullptr if owner is not a character
	 */
	UFUNCTION(BlueprintPure, Category = "DelveDeep|Character")
	ADelveDeepCharacter* GetCharacterOwner() const;

	/**
	 * Called when the component begins play.
	 * Caches the character owner reference for performance.
	 */
	virtual void BeginPlay() override;

	/**
	 * Called when the component ends play.
	 * Cleans up cached references.
	 * 
	 * @param EndPlayReason The reason why play is ending
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Called before the component is destroyed.
	 * Override to perform custom cleanup.
	 */
	virtual void BeginDestroy() override;

protected:
	/**
	 * Cached reference to the owning character actor.
	 * Cached for performance to avoid repeated casts.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "DelveDeep|Character")
	ADelveDeepCharacter* CharacterOwner;

	/**
	 * Called during component initialization.
	 * Override to perform custom initialization logic.
	 */
	virtual void InitializeComponent() override;

	/**
	 * Initializes the component from character data asset.
	 * Override this method to load component-specific data from the character data asset.
	 * 
	 * Called by the character actor after loading its data asset.
	 * 
	 * @param CharacterData The character data asset to initialize from
	 */
	virtual void InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData);

	/**
	 * Validates the component state.
	 * Override this method to implement component-specific validation logic.
	 * 
	 * @param Context The validation context to populate with errors and warnings
	 * @return True if validation passed, false if there are errors
	 */
	virtual bool ValidateComponent(FDelveDeepValidationContext& Context) const;

	// Allow character actor to call protected initialization methods
	friend class ADelveDeepCharacter;
};
