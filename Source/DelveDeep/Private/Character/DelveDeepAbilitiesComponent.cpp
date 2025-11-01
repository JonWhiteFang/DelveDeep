// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepAbilitiesComponent.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepAbilities, Log, All);

UDelveDeepAbilitiesComponent::UDelveDeepAbilitiesComponent()
{
	// Disable tick by default for performance
	PrimaryComponentTick.bCanEverTick = false;
}

void UDelveDeepAbilitiesComponent::InitializeFromCharacterData(const UDelveDeepCharacterData* Data)
{
	if (!Data)
	{
		UE_LOG(LogDelveDeepAbilities, Error, TEXT("Cannot initialize abilities component with null character data"));
		return;
	}

	// Placeholder: Will load starting abilities in future implementation
	UE_LOG(LogDelveDeepAbilities, Display, TEXT("Abilities component initialized (placeholder)"));
}

bool UDelveDeepAbilitiesComponent::ValidateComponent(FValidationContext& Context) const
{
	// Placeholder: Will validate abilities in future implementation
	return true;
}
