// Copyright Epic Games, Inc. All Rights Reserved.

#include "DelveDeepTelemetrySubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Dom/JsonObject.h"

DEFINE_LOG_CATEGORY(LogDelveDeepTelemetry);

UDelveDeepTelemetrySubsystem::UDelveDeepTelemetrySubsystem()
	: CurrentBudgetAsset(nullptr)
	, bTelemetryEnabled(true)
	, bInitialized(false)
{
}

void UDelveDeepTelemetrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Telemetry Subsystem initializing..."));

	// Reset frame tracker
	FrameTracker.ResetStatistics();

	// Reset system profiler
	SystemProfiler.ResetStatistics();

	// Reset memory tracker
	MemoryTracker.ResetStatistics();

	// Register default system budgets
	RegisterDefaultBudgets();

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

	// Update system profiler frame
	SystemProfiler.UpdateFrame();

	// Update memory tracking
	MemoryTracker.UpdateMemorySnapshot();
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

// System Profiling

void UDelveDeepTelemetrySubsystem::RegisterSystemBudget(FName SystemName, float BudgetMs)
{
	SystemProfiler.RegisterSystem(SystemName, BudgetMs);
}

void UDelveDeepTelemetrySubsystem::LoadBudgetsFromAsset(UDelveDeepPerformanceBudget* BudgetAsset)
{
	if (!BudgetAsset)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Attempted to load budgets from null asset"));
		return;
	}

	// Validate budget asset
	FValidationContext Context;
	Context.SystemName = TEXT("Telemetry");
	Context.OperationName = TEXT("LoadBudgetsFromAsset");

	if (!BudgetAsset->Validate(Context))
	{
		UE_LOG(LogDelveDeepTelemetry, Error,
			TEXT("Failed to load budgets from asset: %s"),
			*Context.GetReport());
		return;
	}

	// Load budgets from asset
	for (const FPerformanceBudgetEntry& Entry : BudgetAsset->SystemBudgets)
	{
		RegisterSystemBudget(Entry.SystemName, Entry.BudgetMilliseconds);
	}

	// Store reference to current budget asset
	CurrentBudgetAsset = BudgetAsset;

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Loaded %d system budgets from asset '%s'"),
		BudgetAsset->SystemBudgets.Num(),
		*BudgetAsset->GetName());
}

void UDelveDeepTelemetrySubsystem::RecordSystemTime(FName SystemName, double CycleTimeMs)
{
	SystemProfiler.RecordSystemTime(SystemName, CycleTimeMs);
}

FSystemPerformanceData UDelveDeepTelemetrySubsystem::GetSystemPerformance(FName SystemName) const
{
	return SystemProfiler.GetSystemData(SystemName);
}

TArray<FSystemPerformanceData> UDelveDeepTelemetrySubsystem::GetAllSystemPerformance() const
{
	return SystemProfiler.GetAllSystemData();
}

float UDelveDeepTelemetrySubsystem::GetSystemBudgetUtilization(FName SystemName) const
{
	return SystemProfiler.GetBudgetUtilization(SystemName);
}

bool UDelveDeepTelemetrySubsystem::IsSystemBudgetViolated(FName SystemName) const
{
	return SystemProfiler.IsBudgetViolated(SystemName);
}

TArray<FBudgetViolation> UDelveDeepTelemetrySubsystem::GetBudgetViolationHistory() const
{
	return SystemProfiler.GetViolationHistory();
}

void UDelveDeepTelemetrySubsystem::RegisterDefaultBudgets()
{
	// Register default budgets for major systems
	// These can be overridden by configuration data or console commands

	// Core systems
	RegisterSystemBudget(TEXT("Combat"), 2.0f);        // 2ms budget
	RegisterSystemBudget(TEXT("AI"), 2.0f);            // 2ms budget
	RegisterSystemBudget(TEXT("World"), 1.5f);         // 1.5ms budget
	RegisterSystemBudget(TEXT("UI"), 1.0f);            // 1ms budget
	RegisterSystemBudget(TEXT("Events"), 0.5f);        // 0.5ms budget
	RegisterSystemBudget(TEXT("Config"), 0.5f);        // 0.5ms budget
	RegisterSystemBudget(TEXT("Telemetry"), 0.5f);     // 0.5ms budget

	// Subsystems
	RegisterSystemBudget(TEXT("DamageCalculation"), 0.5f);
	RegisterSystemBudget(TEXT("TargetingSystem"), 0.5f);
	RegisterSystemBudget(TEXT("BehaviorTree"), 1.0f);
	RegisterSystemBudget(TEXT("Pathfinding"), 1.0f);
	RegisterSystemBudget(TEXT("ProceduralGeneration"), 1.0f);
	RegisterSystemBudget(TEXT("CollisionDetection"), 0.5f);
	RegisterSystemBudget(TEXT("HUDUpdate"), 0.5f);
	RegisterSystemBudget(TEXT("MenuRendering"), 0.5f);
	RegisterSystemBudget(TEXT("EventBroadcast"), 0.3f);
	RegisterSystemBudget(TEXT("EventProcessing"), 0.2f);
	RegisterSystemBudget(TEXT("DataAssetQuery"), 0.1f);
	RegisterSystemBudget(TEXT("Validation"), 0.2f);

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Registered default system budgets"));
}

// Memory Tracking

FMemorySnapshot UDelveDeepTelemetrySubsystem::GetCurrentMemorySnapshot() const
{
	return MemoryTracker.GetCurrentSnapshot();
}

uint64 UDelveDeepTelemetrySubsystem::GetSystemMemoryUsage(FName SystemName) const
{
	return MemoryTracker.GetSystemMemory(SystemName);
}

void UDelveDeepTelemetrySubsystem::TrackSystemAllocation(FName SystemName, int64 AllocationSize)
{
	if (AllocationSize > 0)
	{
		MemoryTracker.TrackSystemAllocation(SystemName, static_cast<uint64>(AllocationSize));
	}
}

void UDelveDeepTelemetrySubsystem::TrackSystemDeallocation(FName SystemName, int64 DeallocationSize)
{
	if (DeallocationSize > 0)
	{
		MemoryTracker.TrackSystemDeallocation(SystemName, static_cast<uint64>(DeallocationSize));
	}
}

bool UDelveDeepTelemetrySubsystem::IsMemoryLeakDetected() const
{
	return MemoryTracker.IsLeakDetected();
}

float UDelveDeepTelemetrySubsystem::GetMemoryGrowthRate() const
{
	return MemoryTracker.GetMemoryGrowthRate();
}

int64 UDelveDeepTelemetrySubsystem::GetPeakMemoryUsage() const
{
	return static_cast<int64>(MemoryTracker.GetPeakMemoryUsage());
}
