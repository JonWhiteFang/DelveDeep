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

	// Update gameplay metrics
	GameplayMetrics.UpdateFrame();

	// Update overlay frame history if enabled
	if (bOverlayEnabled && PerformanceOverlay.IsValid())
	{
		const float FrameTimeMs = DeltaTime * 1000.0f;
		PerformanceOverlay->AddFrameTime(FrameTimeMs);
	}

	// Capture profiling data if session is active
	if (bProfilingActive)
	{
		// Capture frame data
		CurrentSession.FrameData.Add(GetCurrentFrameData());
		CurrentSession.TotalFrames++;

		// Capture system data for all systems
		const TArray<FSystemPerformanceData> SystemData = GetAllSystemPerformance();
		for (const FSystemPerformanceData& SystemPerf : SystemData)
		{
			TArray<FSystemPerformanceData>& SystemFrames = CurrentSession.SystemData.FindOrAdd(SystemPerf.SystemName);
			SystemFrames.Add(SystemPerf);
		}

		// Capture memory snapshot every 10 frames to reduce overhead
		if (ProfilingFrameCounter % 10 == 0)
		{
			CurrentSession.MemorySnapshots.Add(GetCurrentMemorySnapshot());
		}

		ProfilingFrameCounter++;

		// Update session duration
		const FTimespan Duration = FDateTime::Now() - CurrentSession.StartTime;
		CurrentSession.DurationSeconds = static_cast<float>(Duration.GetTotalSeconds());

		// Auto-stop if max duration reached
		if (CurrentSession.HasReachedMaxDuration())
		{
			UE_LOG(LogDelveDeepTelemetry, Warning,
				TEXT("Profiling session '%s' reached maximum duration (%.0fs), auto-stopping"),
				*CurrentSession.SessionName.ToString(), FProfilingSession::MaxDurationSeconds);
			StopProfilingSession();
		}
	}
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


// Performance Overlay

void UDelveDeepTelemetrySubsystem::EnablePerformanceOverlay(EOverlayMode Mode)
{
	if (!PerformanceOverlay.IsValid())
	{
		PerformanceOverlay = MakeShared<FDelveDeepPerformanceOverlay>();
	}

	PerformanceOverlay->SetMode(Mode);
	bOverlayEnabled = true;

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Performance overlay enabled (Mode: %d)"), static_cast<int32>(Mode));
}

void UDelveDeepTelemetrySubsystem::DisablePerformanceOverlay()
{
	bOverlayEnabled = false;

	UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Performance overlay disabled"));
}

bool UDelveDeepTelemetrySubsystem::IsOverlayEnabled() const
{
	return bOverlayEnabled;
}

void UDelveDeepTelemetrySubsystem::SetOverlayMode(EOverlayMode Mode)
{
	if (PerformanceOverlay.IsValid())
	{
		PerformanceOverlay->SetMode(Mode);
		UE_LOG(LogDelveDeepTelemetry, Display, TEXT("Performance overlay mode changed to: %d"), static_cast<int32>(Mode));
	}
	else
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Cannot set overlay mode: overlay not initialized"));
	}
}

EOverlayMode UDelveDeepTelemetrySubsystem::GetOverlayMode() const
{
	if (PerformanceOverlay.IsValid())
	{
		return PerformanceOverlay->GetMode();
	}

	return EOverlayMode::Standard;
}

void UDelveDeepTelemetrySubsystem::RenderPerformanceOverlay(UCanvas* Canvas)
{
	if (!bOverlayEnabled || !PerformanceOverlay.IsValid() || !Canvas)
	{
		return;
	}

	// Get current performance data
	const FFramePerformanceData FrameData = GetCurrentFrameData();
	const TArray<FSystemPerformanceData> SystemData = GetAllSystemPerformance();
	const FMemorySnapshot MemoryData = GetCurrentMemorySnapshot();

	// Render overlay
	PerformanceOverlay->Render(Canvas, FrameData, SystemData, MemoryData);
}


// Profiling Sessions

bool UDelveDeepTelemetrySubsystem::StartProfilingSession(FName SessionName)
{
	if (bProfilingActive)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning,
			TEXT("Cannot start profiling session '%s': session '%s' is already active"),
			*SessionName.ToString(), *CurrentSession.SessionName.ToString());
		return false;
	}

	if (SessionName.IsNone())
	{
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("Cannot start profiling session with empty name"));
		return false;
	}

	// Initialize new session
	CurrentSession.Clear();
	CurrentSession.SessionName = SessionName;
	CurrentSession.StartTime = FDateTime::Now();
	CurrentSession.bActive = true;
	CurrentSession.DurationSeconds = 0.0f;
	CurrentSession.TotalFrames = 0;

	bProfilingActive = true;
	ProfilingFrameCounter = 0;

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Started profiling session '%s' (max duration: %.0f seconds)"),
		*SessionName.ToString(), FProfilingSession::MaxDurationSeconds);

	return true;
}

bool UDelveDeepTelemetrySubsystem::StopProfilingSession()
{
	if (!bProfilingActive)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("No profiling session is currently active"));
		return false;
	}

	// Finalize session
	CurrentSession.EndTime = FDateTime::Now();
	CurrentSession.bActive = false;
	bProfilingActive = false;

	const FTimespan Duration = CurrentSession.EndTime - CurrentSession.StartTime;
	CurrentSession.DurationSeconds = static_cast<float>(Duration.GetTotalSeconds());

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Stopped profiling session '%s' (Duration: %.2fs, Frames: %d, Memory: %s)"),
		*CurrentSession.SessionName.ToString(),
		CurrentSession.DurationSeconds,
		CurrentSession.TotalFrames,
		*FormatBytes(CurrentSession.GetEstimatedMemoryUsage()));

	// Auto-save session
	const FString AutoSaveFilePath = GetDefaultProfilingDirectory() /
		FString::Printf(TEXT("ProfilingSession_%s_%s.json"),
			*CurrentSession.SessionName.ToString(),
			*CurrentSession.StartTime.ToString(TEXT("%Y%m%d_%H%M%S")));

	if (SaveProfilingSession(AutoSaveFilePath))
	{
		UE_LOG(LogDelveDeepTelemetry, Display,
			TEXT("Auto-saved profiling session to: %s"), *AutoSaveFilePath);
	}

	return true;
}

bool UDelveDeepTelemetrySubsystem::IsProfilingActive() const
{
	return bProfilingActive;
}

bool UDelveDeepTelemetrySubsystem::GetCurrentSession(FProfilingSession& OutSession) const
{
	if (!bProfilingActive)
	{
		return false;
	}

	OutSession = CurrentSession;
	return true;
}

bool UDelveDeepTelemetrySubsystem::GenerateProfilingReport(FProfilingSessionReport& OutReport)
{
	if (CurrentSession.FrameData.Num() == 0)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Cannot generate report: no frame data captured"));
		return false;
	}

	// Initialize report
	OutReport.SessionName = CurrentSession.SessionName;
	OutReport.DurationSeconds = CurrentSession.DurationSeconds;
	OutReport.TotalFrames = CurrentSession.TotalFrames;
	OutReport.StartTime = CurrentSession.StartTime;
	OutReport.EndTime = CurrentSession.EndTime;

	// Calculate FPS statistics
	float TotalFPS = 0.0f;
	OutReport.MinFPS = FLT_MAX;
	OutReport.MaxFPS = 0.0f;
	OutReport.SpikeCount = 0;

	TArray<float> FrameTimes;
	FrameTimes.Reserve(CurrentSession.FrameData.Num());

	for (const FFramePerformanceData& Frame : CurrentSession.FrameData)
	{
		const float FPS = Frame.FrameTime > 0.0f ? 1000.0f / Frame.FrameTime : 0.0f;
		TotalFPS += FPS;
		OutReport.MinFPS = FMath::Min(OutReport.MinFPS, FPS);
		OutReport.MaxFPS = FMath::Max(OutReport.MaxFPS, FPS);

		FrameTimes.Add(Frame.FrameTime);

		// Count spikes (>16.67ms)
		if (Frame.FrameTime > 16.67f)
		{
			OutReport.SpikeCount++;
		}
	}

	OutReport.AverageFPS = TotalFPS / CurrentSession.FrameData.Num();

	// Calculate 1% low FPS (99th percentile worst frame times)
	FrameTimes.Sort();
	const int32 OnePercentIndex = FMath::Max(0, FrameTimes.Num() - (FrameTimes.Num() / 100));
	if (OnePercentIndex < FrameTimes.Num())
	{
		const float OnePercentFrameTime = FrameTimes[OnePercentIndex];
		OutReport.OnePercentLowFPS = OnePercentFrameTime > 0.0f ? 1000.0f / OnePercentFrameTime : 0.0f;
	}

	// Aggregate system performance data
	TMap<FName, FSystemPerformanceData> AggregatedSystemData;

	for (const auto& Pair : CurrentSession.SystemData)
	{
		const FName SystemName = Pair.Key;
		const TArray<FSystemPerformanceData>& SystemFrames = Pair.Value;

		if (SystemFrames.Num() == 0)
		{
			continue;
		}

		FSystemPerformanceData& Aggregated = AggregatedSystemData.FindOrAdd(SystemName);
		Aggregated.SystemName = SystemName;
		Aggregated.CycleTime = 0.0;
		Aggregated.CallCount = 0;
		Aggregated.PeakTime = 0.0;

		for (const FSystemPerformanceData& SystemFrame : SystemFrames)
		{
			Aggregated.CycleTime += SystemFrame.CycleTime;
			Aggregated.CallCount += SystemFrame.CallCount;
			Aggregated.PeakTime = FMath::Max(Aggregated.PeakTime, SystemFrame.PeakTime);
		}

		// Calculate average
		Aggregated.CycleTime /= SystemFrames.Num();
		Aggregated.BudgetTime = SystemFrames[0].BudgetTime; // Use first frame's budget
	}

	AggregatedSystemData.GenerateValueArray(OutReport.SystemBreakdown);

	// Calculate memory statistics
	uint64 TotalMemory = 0;
	OutReport.PeakMemoryUsage = 0;

	for (const FMemorySnapshot& Snapshot : CurrentSession.MemorySnapshots)
	{
		TotalMemory += Snapshot.TotalMemory;
		OutReport.PeakMemoryUsage = FMath::Max(OutReport.PeakMemoryUsage, Snapshot.TotalMemory);
	}

	if (CurrentSession.MemorySnapshots.Num() > 0)
	{
		OutReport.AverageMemoryUsage = TotalMemory / CurrentSession.MemorySnapshots.Num();
	}

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Generated profiling report for session '%s': Avg FPS: %.1f, 1%% Low: %.1f, Spikes: %d"),
		*OutReport.SessionName.ToString(), OutReport.AverageFPS, OutReport.OnePercentLowFPS, OutReport.SpikeCount);

	return true;
}

bool UDelveDeepTelemetrySubsystem::SaveProfilingSession(const FString& FilePath)
{
	if (CurrentSession.FrameData.Num() == 0)
	{
		UE_LOG(LogDelveDeepTelemetry, Warning, TEXT("Cannot save profiling session: no data captured"));
		return false;
	}

	// Use default path if not specified
	FString SavePath = FilePath;
	if (SavePath.IsEmpty())
	{
		SavePath = GetDefaultProfilingDirectory() /
			FString::Printf(TEXT("ProfilingSession_%s_%s.json"),
				*CurrentSession.SessionName.ToString(),
				*CurrentSession.StartTime.ToString(TEXT("%Y%m%d_%H%M%S")));
	}

	// Create JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	// Session metadata
	JsonObject->SetStringField(TEXT("SessionName"), CurrentSession.SessionName.ToString());
	JsonObject->SetStringField(TEXT("StartTime"), CurrentSession.StartTime.ToString());
	JsonObject->SetStringField(TEXT("EndTime"), CurrentSession.EndTime.ToString());
	JsonObject->SetNumberField(TEXT("DurationSeconds"), CurrentSession.DurationSeconds);
	JsonObject->SetNumberField(TEXT("TotalFrames"), CurrentSession.TotalFrames);

	// Frame data (sample every 10th frame to reduce file size)
	TArray<TSharedPtr<FJsonValue>> FrameArray;
	for (int32 i = 0; i < CurrentSession.FrameData.Num(); i += 10)
	{
		const FFramePerformanceData& Frame = CurrentSession.FrameData[i];
		TSharedPtr<FJsonObject> FrameObj = MakeShared<FJsonObject>();
		FrameObj->SetNumberField(TEXT("FrameTime"), Frame.FrameTime);
		FrameObj->SetNumberField(TEXT("GameThreadTime"), Frame.GameThreadTime);
		FrameObj->SetNumberField(TEXT("RenderThreadTime"), Frame.RenderThreadTime);
		FrameArray.Add(MakeShared<FJsonValueObject>(FrameObj));
	}
	JsonObject->SetArrayField(TEXT("FrameData"), FrameArray);

	// System data summary
	TArray<TSharedPtr<FJsonValue>> SystemArray;
	for (const auto& Pair : CurrentSession.SystemData)
	{
		TSharedPtr<FJsonObject> SystemObj = MakeShared<FJsonObject>();
		SystemObj->SetStringField(TEXT("SystemName"), Pair.Key.ToString());
		SystemObj->SetNumberField(TEXT("FrameCount"), Pair.Value.Num());
		SystemArray.Add(MakeShared<FJsonValueObject>(SystemObj));
	}
	JsonObject->SetArrayField(TEXT("SystemData"), SystemArray);

	// Write to file
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter))
	{
		UE_LOG(LogDelveDeepTelemetry, Error, TEXT("Failed to serialize profiling session to JSON"));
		return false;
	}

	// Ensure directory exists
	const FString Directory = FPaths::GetPath(SavePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		if (!PlatformFile.CreateDirectoryTree(*Directory))
		{
			UE_LOG(LogDelveDeepTelemetry, Error,
				TEXT("Failed to create directory: %s"), *Directory);
			return false;
		}
	}

	// Save to file
	if (!FFileHelper::SaveStringToFile(JsonString, *SavePath))
	{
		UE_LOG(LogDelveDeepTelemetry, Error,
			TEXT("Failed to save profiling session to file: %s"), *SavePath);
		return false;
	}

	UE_LOG(LogDelveDeepTelemetry, Display,
		TEXT("Saved profiling session to: %s (Size: %s)"),
		*SavePath, *FormatBytes(JsonString.Len()));

	return true;
}

FString UDelveDeepTelemetrySubsystem::GetDefaultProfilingDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("Profiling");
}

FString UDelveDeepTelemetrySubsystem::FormatBytes(uint64 Bytes) const
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%llu B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f KB"), Bytes / 1024.0);
	}
	else if (Bytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f MB"), Bytes / (1024.0 * 1024.0));
	}
	else
	{
		return FString::Printf(TEXT("%.2f GB"), Bytes / (1024.0 * 1024.0 * 1024.0));
	}
}


// Gameplay Metrics

void UDelveDeepTelemetrySubsystem::TrackEntityCount(FName EntityType, int32 Count)
{
	GameplayMetrics.TrackEntityCount(EntityType, Count);
}

int32 UDelveDeepTelemetrySubsystem::GetEntityCount(FName EntityType) const
{
	return GameplayMetrics.GetEntityCount(EntityType);
}

int32 UDelveDeepTelemetrySubsystem::GetPeakEntityCount(FName EntityType) const
{
	return GameplayMetrics.GetPeakEntityCount(EntityType);
}

float UDelveDeepTelemetrySubsystem::GetAverageEntityCount(FName EntityType) const
{
	return GameplayMetrics.GetAverageEntityCount(EntityType);
}

bool UDelveDeepTelemetrySubsystem::IsEntityCountExceedingLimit(FName EntityType) const
{
	return GameplayMetrics.IsEntityCountExceedingLimit(EntityType);
}

int32 UDelveDeepTelemetrySubsystem::GetRecommendedEntityLimit(FName EntityType) const
{
	return GameplayMetrics.GetRecommendedLimit(EntityType);
}
