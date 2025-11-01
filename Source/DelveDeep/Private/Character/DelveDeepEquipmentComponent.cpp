// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/DelveDeepEquipmentComponent.h"
#include "Configuration/DelveDeepCharacterData.h"
#include "Validation/ValidationContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogDelveDeepEquipment, Log, All);

UDelveDeepEquipmentComponent::UDelveDeepEquipmentComponent()
{
	// Disable tick by default for performance
	PrimaryComponentTick.bCanEverTick = false;
}

void UDelveDeepEquipmentComponent::InitializeFromCharacterData(const UDelveDeepCharacterData* Data)
{
	if (!Data)
	{
		UE_LOG(LogDelveDeepEquipment, Error, TEXT("Cannot initialize equipment component with null character data"));
		return;
	}

	// Placeholder: Will load starting weapon in future implementation
	UE_LOG(LogDelveDeepEquipment, Display, TEXT("Equipment component initialized (placeholder)"));
}

bool UDelveDeepEquipmentComponent::ValidateComponent(FValidationContext& Context) const
{
	// Placeholder: Will validate equipment in future implementation
	return true;
}
