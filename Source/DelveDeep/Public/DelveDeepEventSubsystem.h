// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "DelveDeepEventSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDelveDeepEvents, Log, All);

/**
 * Centralized event system subsystem for DelveDeep.
 * Provides a gameplay event bus using GameplayTags for loose coupling between systems.
 * Enables event-driven communication without direct dependencies.
 */
UCLASS(BlueprintType)
class DELVEDEEP_API UDelveDeepEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
