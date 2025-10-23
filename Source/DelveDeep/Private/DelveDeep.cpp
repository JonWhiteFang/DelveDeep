// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeep.h"
#include "DelveDeepValidation.h"

#define LOCTEXT_NAMESPACE "FDelveDeepModule"

void FDelveDeepModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogDelveDeepConfig, Display, TEXT("DelveDeep module starting up"));
}

void FDelveDeepModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogDelveDeepConfig, Display, TEXT("DelveDeep module shutting down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDelveDeepModule, DelveDeep)
