// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepCharacterComponent.h"
#include "Character/DelveDeepCharacter.h"
#include "DelveDeepCharacterData.h"
#include "DelveDeepValidation.h"

// Logging
DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepCharacter, Log, All);

UDelveDeepCharacterComponent::UDelveDeepCharacterComponent()
{
	// Disable tick by default for performance optimization
	// Components should use timer-based updates or event-driven logic instead
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// Initialize cached owner reference
	CharacterOwner = nullptr;
}

ADelveDeepCharacter* UDelveDeepCharacterComponent::GetCharacterOwner() const
{
	// Return cached owner reference
	// This is cached in BeginPlay() to avoid repeated casts
	return CharacterOwner;
}

void UDelveDeepCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the character owner reference for performance
	CharacterOwner = Cast<ADelveDeepCharacter>(GetOwner());

	if (!CharacterOwner)
	{
		UE_LOG(LogDelveDeepCharacter, Warning,
			TEXT("Component '%s' is not owned by a DelveDeepCharacter actor"),
			*GetName());
	}
}

void UDelveDeepCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear cached reference
	CharacterOwner = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UDelveDeepCharacterComponent::BeginDestroy()
{
	// Clear cached reference before destruction
	CharacterOwner = nullptr;

	Super::BeginDestroy();
}

void UDelveDeepCharacterComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Base implementation - override in derived classes for custom initialization
}

void UDelveDeepCharacterComponent::InitializeFromCharacterData(const UDelveDeepCharacterData* CharacterData)
{
	// Base implementation - override in derived classes to load component-specific data
	// This method is called by the character actor after loading its data asset

	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Warning,
			TEXT("Component '%s' received null character data during initialization"),
			*GetName());
		return;
	}

	// Derived classes should override this method to load their specific data
}

bool UDelveDeepCharacterComponent::ValidateComponent(FValidationContext& Context) const
{
	// Base validation - check that component has a valid owner
	Context.SystemName = TEXT("Character");
	Context.OperationName = FString::Printf(TEXT("ValidateComponent_%s"), *GetClass()->GetName());

	bool bIsValid = true;

	// Validate owner reference
	if (!CharacterOwner)
	{
		Context.AddError(FString::Printf(
			TEXT("Component '%s' has no valid character owner"),
			*GetName()));
		bIsValid = false;
	}

	// Derived classes should override this method to add component-specific validation
	return bIsValid;
}
