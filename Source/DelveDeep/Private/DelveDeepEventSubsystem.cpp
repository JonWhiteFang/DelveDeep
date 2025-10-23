// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepEventSubsystem.h"

DEFINE_LOG_CATEGORY(LogDelveDeepEvents);

void UDelveDeepEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem initializing..."));
	
	// Initialization logic will be added in subsequent tasks
	
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem initialized successfully"));
}

void UDelveDeepEventSubsystem::Deinitialize()
{
	UE_LOG(LogDelveDeepEvents, Display, TEXT("Event Subsystem shutting down..."));
	
	// Cleanup logic will be added in subsequent tasks
	
	Super::Deinitialize();
}
