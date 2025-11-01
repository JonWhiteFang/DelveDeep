// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepCharacter.h"
#include "Character/DelveDeepCharacterComponent.h"
#include "Character/DelveDeepStatsComponent.h"
#include "Character/DelveDeepAbilitiesComponent.h"
#include "Character/DelveDeepEquipmentComponent.h"
#include "Configuration/DelveDeepConfigurationManager.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepCharacter, Log, All);

ADelveDeepCharacter::ADelveDeepCharacter()
{
	// Disable tick by default for performance
	PrimaryActorTick.bCanEverTick = false;

	// Create stats component
	StatsComponent = CreateDefaultSubobject<UDelveDeepStatsComponent>(TEXT("StatsComponent"));

	// Create abilities component
	AbilitiesComponent = CreateDefaultSubobject<UDelveDeepAbilitiesComponent>(TEXT("AbilitiesComponent"));

	// Create equipment component
	EquipmentComponent = CreateDefaultSubobject<UDelveDeepEquipmentComponent>(TEXT("EquipmentComponent"));

	// Initialize character data to nullptr
	CharacterData = nullptr;
}

void ADelveDeepCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize character from configuration data
	InitializeFromData();
}

void ADelveDeepCharacter::InitializeFromData()
{
	// Validate character class name is set
	if (CharacterClassName.IsNone())
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("Character class name not set for %s"), *GetName());
		return;
	}

	// Get configuration manager
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("Failed to get game instance for %s"), *GetName());
		return;
	}

	UDelveDeepConfigurationManager* ConfigManager = GameInstance->GetSubsystem<UDelveDeepConfigurationManager>();
	if (!ConfigManager)
	{
		UE_LOG(LogDelveDeepCharacter, Error, TEXT("Failed to get configuration manager for %s"), *GetName());
		return;
	}

	// Query character data
	CharacterData = ConfigManager->GetCharacterData(CharacterClassName);
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Failed to load character data for class '%s' on %s"), 
			*CharacterClassName.ToString(), *GetName());
		return;
	}

	// Validate character data
	FValidationContext Context;
	Context.SystemName = TEXT("Character");
	Context.OperationName = TEXT("InitializeFromData");

	if (!ValidateCharacterData(Context))
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Character data validation failed for %s: %s"), 
			*GetName(), *Context.GetReport());
		
		// Continue with fallback values rather than failing completely
		UE_LOG(LogDelveDeepCharacter, Warning, TEXT("Using fallback values for %s"), *GetName());
	}

	// Initialize components with character data
	InitializeComponents();

	UE_LOG(LogDelveDeepCharacter, Display, 
		TEXT("Character initialized: %s (Class: %s)"), 
		*GetName(), *CharacterClassName.ToString());
}

bool ADelveDeepCharacter::ValidateCharacterData(FValidationContext& Context) const
{
	bool bIsValid = true;

	// Validate character data exists
	if (!CharacterData)
	{
		Context.AddError(TEXT("Character data is null"));
		return false;
	}

	// Validate base stats are in reasonable ranges
	if (CharacterData->BaseHealth <= 0.0f || CharacterData->BaseHealth > 10000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseHealth out of range: %.2f (expected 1-10000)"), 
			CharacterData->BaseHealth));
		bIsValid = false;
	}

	if (CharacterData->BaseDamage < 0.0f || CharacterData->BaseDamage > 1000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseDamage out of range: %.2f (expected 0-1000)"), 
			CharacterData->BaseDamage));
		bIsValid = false;
	}

	if (CharacterData->BaseMoveSpeed <= 0.0f || CharacterData->BaseMoveSpeed > 2000.0f)
	{
		Context.AddError(FString::Printf(
			TEXT("BaseMoveSpeed out of range: %.2f (expected 1-2000)"), 
			CharacterData->BaseMoveSpeed));
		bIsValid = false;
	}

	// Validate starting weapon reference (warning only, not critical)
	if (CharacterData->StartingWeapon.IsNull())
	{
		Context.AddWarning(TEXT("No starting weapon assigned"));
	}

	// Validate starting abilities (warning only)
	if (CharacterData->StartingAbilities.Num() == 0)
	{
		Context.AddWarning(TEXT("No starting abilities assigned"));
	}

	return bIsValid;
}

void ADelveDeepCharacter::InitializeComponents()
{
	// Validate components exist
	if (!StatsComponent || !AbilitiesComponent || !EquipmentComponent)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("One or more components missing on %s"), *GetName());
		return;
	}

	// Validate character data exists
	if (!CharacterData)
	{
		UE_LOG(LogDelveDeepCharacter, Error, 
			TEXT("Cannot initialize components without character data on %s"), *GetName());
		return;
	}

	// Initialize stats component
	StatsComponent->InitializeFromCharacterData(CharacterData);

	// Initialize abilities component
	AbilitiesComponent->InitializeFromCharacterData(CharacterData);

	// Initialize equipment component
	EquipmentComponent->InitializeFromCharacterData(CharacterData);

	UE_LOG(LogDelveDeepCharacter, Verbose, 
		TEXT("Components initialized for %s"), *GetName());
}
