// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTelemetrySubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogDelveDeepTelemetry);

UDelveDeepTelemetrySubsystem::UDelveDeepTelemetrySubsystem()
	: bTelemetryEnabled(true)
	, bInitialized(false)
{
}

void UDelveDeepTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Telemetry Subsystem initializing..."));

	// Reset frame tracker
	FrameTracker.ResetStatistics();

	bInitialized = true;
	bTelemetryEnabled = true;

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Telemetry Subsystem initialized successfully"));
}

void UDelveDeepTelemetrySubsystem::Deinitialize()
{
	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Telemetry Subsystem shutting down..."));

	bInitialized = false;
	bTelemetryEnabled = false;

	Super::Deinitialize();
}

void UDelveDeepTelemetrySubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_DelveDeep_TelemetrySystem);

	if (!bTelemetryEnabled || !bInitialized)
	{
		return;
	}

	// Record frame performance
	FrameTracker.RecordFrame(DeltaTime);
}

TStatId UDelveDeepTelemetrySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDelveDeepTelemetrySubsystem, STATGROUP_Tickables);
}

UWorld* UDelveDeepTelemetrySubsystem::GetTickableGameObjectWorld() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetWorld();
	}
	return nullptr;
}

float UDelveDeepTelemetrySubsystem::GetCurrentFPS() const
{
	return FrameTracker.GetCurrentFPS();
}

float UDelveDeepTelemetrySubsystem::GetAverageFPS() const
{
	return FrameTracker.GetAverageFPS();
}

float UDelveDeepTelemetrySubsystem::GetOnePercentLowFPS() const
{
	return FrameTracker.GetOnePercentLowFPS();
}

FFramePerformanceData UDelveDeepTelemetrySubsystem::GetCurrentFrameData() const
{
	return FrameTracker.GetCurrentFrameData();
}

TArray<float> UDelveDeepTelemetrySubsystem::GetFrameTimeHistory(int32 NumFrames) const
{
	return FrameTracker.GetFrameTimeHistory(NumFrames);
}
