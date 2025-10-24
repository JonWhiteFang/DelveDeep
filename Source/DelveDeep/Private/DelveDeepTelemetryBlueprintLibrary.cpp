// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTelemetryBlueprintLibrary.h"
#include "DelveDeepTelemetrySubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDelveDeepTelemetrySubsystem* UDelveDeepTelemetryBlueprintLibrary::GetTelemetrySubsystem(
	const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
		EGetWorldErrorMode::LogAndReturnNull);

	if (!World || !World->GetGameInstance())
	{
		return nullptr;
	}

	return World->GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>();
}

float UDelveDeepTelemetryBlueprintLibrary::GetCurrentFPS(const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->GetCurrentFPS() : 0.0f;
}

float UDelveDeepTelemetryBlueprintLibrary::GetAverageFPS(const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->GetAverageFPS() : 0.0f;
}

float UDelveDeepTelemetryBlueprintLibrary::GetOnePercentLowFPS(const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->GetOnePercentLowFPS() : 0.0f;
}

bool UDelveDeepTelemetryBlueprintLibrary::CapturePerformanceBaseline(
	const UObject* WorldContextObject,
	FName BaselineName)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->CaptureBaseline(BaselineName) : false;
}

void UDelveDeepTelemetryBlueprintLibrary::EnablePerformanceOverlay(
	const UObject* WorldContextObject,
	EOverlayMode Mode)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	if (Telemetry)
	{
		Telemetry->EnablePerformanceOverlay(Mode);
	}
}

void UDelveDeepTelemetryBlueprintLibrary::DisablePerformanceOverlay(
	const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	if (Telemetry)
	{
		Telemetry->DisablePerformanceOverlay();
	}
}

bool UDelveDeepTelemetryBlueprintLibrary::IsPerformanceOverlayEnabled(
	const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->IsOverlayEnabled() : false;
}

bool UDelveDeepTelemetryBlueprintLibrary::StartProfilingSession(
	const UObject* WorldContextObject,
	FName SessionName)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->StartProfilingSession(SessionName) : false;
}

bool UDelveDeepTelemetryBlueprintLibrary::StopProfilingSession(
	const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->StopProfilingSession() : false;
}

bool UDelveDeepTelemetryBlueprintLibrary::IsProfilingActive(
	const UObject* WorldContextObject)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->IsProfilingActive() : false;
}

void UDelveDeepTelemetryBlueprintLibrary::TrackEntityCount(
	const UObject* WorldContextObject,
	FName EntityType,
	int32 Count)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	if (Telemetry)
	{
		Telemetry->TrackEntityCount(EntityType, Count);
	}
}

int32 UDelveDeepTelemetryBlueprintLibrary::GetEntityCount(
	const UObject* WorldContextObject,
	FName EntityType)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	return Telemetry ? Telemetry->GetEntityCount(EntityType) : 0;
}

void UDelveDeepTelemetryBlueprintLibrary::RecordAssetLoad(
	const UObject* WorldContextObject,
	const FString& AssetPath,
	float LoadTimeMs,
	int64 AssetSize,
	bool bSynchronous)
{
	UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetrySubsystem(WorldContextObject);
	if (Telemetry)
	{
		Telemetry->RecordAssetLoad(AssetPath, LoadTimeMs, AssetSize, bSynchronous);
	}
}
