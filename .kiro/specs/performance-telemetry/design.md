# Design Document

## Overview

The Performance Telemetry System provides comprehensive performance monitoring infrastructure for DelveDeep, enabling real-time and historical performance analysis. Implemented as a UGameInstanceSubsystem, it integrates with Unreal Engine's stat system while providing game-specific metrics tailored to DelveDeep's architecture.

The system follows a layered architecture: low-level cycle counters and stat groups for precise measurements, mid-level aggregation for system-wide metrics, and high-level reporting for analysis and visualization. It's designed for minimal runtime overhead (<0.5ms per frame) while providing actionable performance insights.

## Architecture

### System Components

```
UDelveDeepTelemetrySubsystem (UGameInstanceSubsystem)
├── Frame Tracker (FFramePerformanceTracker)
│   ├── Frame Time History (TCircularBuffer<float>)
│   ├── FPS Statistics (Current, Average, 1% Low, 0.1% Low)
│   └── Spike Detection (Threshold monitoring)
├── System Profiler (FSystemProfiler)
│   ├── Stat Groups (Combat, AI, World, UI, Events, Config)
│   ├── Cycle Counters (Per-system timing)
│   └── Budget Tracking (Budget vs Actual)
├── Memory Tracker (FMemoryTracker)
│   ├── Total Memory Usage
│   ├── Per-System Allocation
│   └── Leak Detection
├── Gameplay Metrics (FGameplayMetrics)
│   ├── Entity Counts (Monsters, Projectiles, Particles)
│   ├── Event Counts (Per-frame event broadcasts)
│   └── Asset Load Tracking
├── Performance Baselines (TMap<FName, FPerformanceBaseline>)
├── Performance Reports (FReportGenerator)
└── Visualization (FPerformanceOverlay)
```

### Class Hierarchy

```cpp
// Core subsystem
UDelveDeepTelemetrySubsystem : public UGameInstanceSubsystem

// Frame performance tracking
USTRUCT()
struct FFramePerformanceData
{
    float FrameTime;
    float GameThreadTime;
    float RenderThreadTime;
    FDateTime Timestamp;
};

// System performance tracking
USTRUCT()
struct FSystemPerformanceData
{
    FName SystemName;
    double CycleTime;
    double BudgetTime;
    int32 CallCount;
    double PeakTime;
};

// Memory tracking
USTRUCT()
struct FMemorySnapshot
{
    uint64 TotalMemory;
    uint64 NativeMemory;
    uint64 ManagedMemory;
    TMap<FName, uint64> PerSystemMemory;
    FDateTime Timestamp;
};

// Performance baseline
USTRUCT()
struct FPerformanceBaseline
{
    FName BaselineName;
    FDateTime CaptureTime;
    FFramePerformanceData AverageFrameData;
    TMap<FName, FSystemPerformanceData> SystemData;
    FMemorySnapshot MemoryData;
    FString BuildVersion;
    FString MapName;
};

// Budget configuration
USTRUCT(BlueprintType)
struct FPerformanceBudget
{
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName SystemName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float BudgetMilliseconds;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bEnforceStrict;
};
```

## Components and Interfaces

### UDelveDeepTelemetrySubsystem

Primary interface for performance monitoring and reporting.

**Public API:**

```cpp
// Lifecycle
virtual void Initialize(FSubsystemCollectionBase& Collection) override;
virtual void Deinitialize() override;
virtual void Tick(float DeltaTime) override;

// Frame tracking
float GetCurrentFPS() const;
float GetAverageFPS() const;
float GetOnePercentLowFPS() const;
FFramePerformanceData GetCurrentFrameData() const;
TArray<float> GetFrameTimeHistory(int32 NumFrames = 120) const;

// System profiling
void RegisterSystemBudget(FName SystemName, float BudgetMs);
FSystemPerformanceData GetSystemPerformance(FName SystemName) const;
TArray<FSystemPerformanceData> GetAllSystemPerformance() const;
float GetSystemBudgetUtilization(FName SystemName) const;

// Memory tracking
FMemorySnapshot GetCurrentMemorySnapshot() const;
uint64 GetSystemMemoryUsage(FName SystemName) const;
bool IsMemoryLeakDetected() const;

// Baseline management
void CaptureBaseline(FName BaselineName);
bool CompareToBaseline(FName BaselineName, FPerformanceComparison& OutComparison) const;
TArray<FName> GetAvailableBaselines() const;
bool SaveBaseline(FName BaselineName, const FString& FilePath);
bool LoadBaseline(FName BaselineName, const FString& FilePath);

// Reporting
void GeneratePerformanceReport(FPerformanceReport& OutReport, float DurationSeconds = 300.0f);
bool ExportReportToCSV(const FPerformanceReport& Report, const FString& FilePath);
bool ExportReportToJSON(const FPerformanceReport& Report, const FString& FilePath);

// Profiling sessions
void StartProfilingSession(FName SessionName);
void StopProfilingSession();
bool IsProfilingActive() const;
FProfilingSession GetCurrentSession() const;

// Visualization
void EnablePerformanceOverlay(EOverlayMode Mode = EOverlayMode::Standard);
void DisablePerformanceOverlay();
bool IsOverlayEnabled() const;
void SetOverlayMode(EOverlayMode Mode);

// Gameplay metrics
void TrackEntityCount(FName EntityType, int32 Count);
int32 GetEntityCount(FName EntityType) const;
void RecordAssetLoad(const FString& AssetPath, float LoadTime, uint64 AssetSize);
```

**Private Implementation:**

```cpp
// Frame tracking
TCircularBuffer<FFramePerformanceData> FrameHistory;
static constexpr int32 MaxFrameHistory = 3600; // 60 seconds at 60 FPS
float CurrentFPS = 0.0f;
float AverageFPS = 0.0f;
float OnePercentLowFPS = 0.0f;
int32 ConsecutiveSlowFrames = 0;

// System profiling
TMap<FName, FSystemPerformanceData> SystemPerformance;
TMap<FName, float> SystemBudgets;
TArray<FBudgetViolation> BudgetViolationHistory;
static constexpr int32 MaxViolationHistory = 100;

// Memory tracking
FMemorySnapshot CurrentMemory;
TArray<FMemorySnapshot> MemoryHistory;
uint64 LastMemoryCheck = 0;
float MemoryGrowthRate = 0.0f;

// Baselines
TMap<FName, FPerformanceBaseline> Baselines;

// Profiling
bool bProfilingActive = false;
FProfilingSession CurrentSession;
FDateTime SessionStartTime;

// Visualization
bool bOverlayEnabled = false;
EOverlayMode OverlayMode = EOverlayMode::Standard;
TSharedPtr<FPerformanceOverlay> PerformanceOverlay;

// Gameplay metrics
TMap<FName, int32> EntityCounts;
TArray<FAssetLoadRecord> AssetLoadHistory;
```

### Stat Groups

**DelveDeep Stat Groups:**

```cpp
// Main stat group
DECLARE_STATS_GROUP(TEXT("DelveDeep"), STATGROUP_DelveDeep, STATCAT_Advanced);

// System-specific stat groups
DECLARE_STATS_GROUP(TEXT("DelveDeep.Combat"), STATGROUP_DelveDeepCombat, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.AI"), STATGROUP_DelveDeepAI, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.World"), STATGROUP_DelveDeepWorld, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.UI"), STATGROUP_DelveDeepUI, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.Events"), STATGROUP_DelveDeepEvents, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("DelveDeep.Config"), STATGROUP_DelveDeepConfig, STATCAT_Advanced);

// Cycle counters
DECLARE_CYCLE_STAT(TEXT("Frame Total"), STAT_FrameTotal, STATGROUP_DelveDeep);
DECLARE_CYCLE_STAT(TEXT("Combat System"), STAT_CombatSystem, STATGROUP_DelveDeepCombat);
DECLARE_CYCLE_STAT(TEXT("AI System"), STAT_AISystem, STATGROUP_DelveDeepAI);
DECLARE_CYCLE_STAT(TEXT("World System"), STAT_WorldSystem, STATGROUP_DelveDeepWorld);
DECLARE_CYCLE_STAT(TEXT("UI System"), STAT_UISystem, STATGROUP_DelveDeepUI);
DECLARE_CYCLE_STAT(TEXT("Event System"), STAT_EventSystem, STATGROUP_DelveDeepEvents);
DECLARE_CYCLE_STAT(TEXT("Config System"), STAT_ConfigSystem, STATGROUP_DelveDeepConfig);

// Memory counters
DECLARE_MEMORY_STAT(TEXT("Total Memory"), STAT_TotalMemory, STATGROUP_DelveDeep);
DECLARE_MEMORY_STAT(TEXT("Combat Memory"), STAT_CombatMemory, STATGROUP_DelveDeepCombat);
DECLARE_MEMORY_STAT(TEXT("AI Memory"), STAT_AIMemory, STATGROUP_DelveDeepAI);

// Dword counters
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Monsters"), STAT_ActiveMonsters, STATGROUP_DelveDeep);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Projectiles"), STAT_ActiveProjectiles, STATGROUP_DelveDeep);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Particles"), STAT_ActiveParticles, STATGROUP_DelveDeep);
DECLARE_DWORD_COUNTER_STAT(TEXT("Events Per Frame"), STAT_EventsPerFrame, STATGROUP_DelveDeepEvents);
```

### Frame Performance Tracker

```cpp
class FFramePerformanceTracker
{
public:
    void RecordFrame(float DeltaTime);
    float GetCurrentFPS() const;
    float GetAverageFPS(int32 NumFrames = 60) const;
    float GetOnePercentLowFPS() const;
    float GetPointOnePercentLowFPS() const;
    bool IsSpikeDetected() const;
    TArray<float> GetFrameTimeHistory(int32 NumFrames) const;

private:
    TCircularBuffer<float> FrameTimes;
    float CurrentFPS = 0.0f;
    int32 ConsecutiveSlowFrames = 0;
    
    void CalculateFPSStatistics();
    void DetectSpikes();
};
```

### System Profiler

```cpp
class FSystemProfiler
{
public:
    void RegisterSystem(FName SystemName, float BudgetMs);
    void RecordSystemTime(FName SystemName, double CycleTime);
    FSystemPerformanceData GetSystemData(FName SystemName) const;
    TArray<FSystemPerformanceData> GetAllSystemData() const;
    float GetBudgetUtilization(FName SystemName) const;
    bool IsBudgetViolated(FName SystemName) const;
    void ResetStatistics();

private:
    TMap<FName, FSystemPerformanceData> SystemData;
    TMap<FName, float> SystemBudgets;
    TArray<FBudgetViolation> ViolationHistory;
    
    void CheckBudgetViolation(FName SystemName);
    void RecordViolation(const FBudgetViolation& Violation);
};
```

### Memory Tracker

```cpp
class FMemoryTracker
{
public:
    void UpdateMemorySnapshot();
    FMemorySnapshot GetCurrentSnapshot() const;
    uint64 GetSystemMemory(FName SystemName) const;
    bool IsLeakDetected() const;
    float GetMemoryGrowthRate() const;
    TArray<FMemorySnapshot> GetMemoryHistory() const;

private:
    FMemorySnapshot CurrentSnapshot;
    TArray<FMemorySnapshot> MemoryHistory;
    uint64 LastTotalMemory = 0;
    FDateTime LastCheckTime;
    
    void DetectMemoryLeaks();
    void CalculateGrowthRate();
};
```

### Performance Overlay

```cpp
enum class EOverlayMode : uint8
{
    Minimal,    // FPS only
    Standard,   // FPS + frame time graph
    Detailed    // FPS + graph + system breakdown + memory
};

class FPerformanceOverlay
{
public:
    void Render(UCanvas* Canvas, const FFramePerformanceData& FrameData,
                const TArray<FSystemPerformanceData>& SystemData,
                const FMemorySnapshot& MemoryData);
    void SetMode(EOverlayMode Mode);
    EOverlayMode GetMode() const;

private:
    EOverlayMode Mode = EOverlayMode::Standard;
    TArray<float> FrameTimeGraph;
    
    void RenderMinimal(UCanvas* Canvas, const FFramePerformanceData& FrameData);
    void RenderStandard(UCanvas* Canvas, const FFramePerformanceData& FrameData);
    void RenderDetailed(UCanvas* Canvas, const FFramePerformanceData& FrameData,
                       const TArray<FSystemPerformanceData>& SystemData,
                       const FMemorySnapshot& MemoryData);
    void RenderFrameTimeGraph(UCanvas* Canvas, const TArray<float>& FrameTimes);
    void RenderSystemBreakdown(UCanvas* Canvas, const TArray<FSystemPerformanceData>& SystemData);
};
```

## Data Models

### Performance Budget Configuration

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FPerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    FName SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance",
        meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float BudgetMilliseconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bEnforceStrict = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    FLinearColor VisualizationColor = FLinearColor::White;
};
```

### Budget Violation Record

```cpp
USTRUCT()
struct FBudgetViolation
{
    GENERATED_BODY()

    FName SystemName;
    float ActualTime;
    float BudgetTime;
    float OveragePercentage;
    FDateTime Timestamp;
    int32 FrameNumber;
};
```

### Performance Report

```cpp
USTRUCT(BlueprintType)
struct DELVEDEEP_API FPerformanceReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FDateTime GenerationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float DurationSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString MapName;

    // Frame statistics
    UPROPERTY(BlueprintReadOnly, Category = "Frame")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Frame")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Frame")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Frame")
    float OnePercentLowFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Frame")
    int32 TotalFrames;

    UPROPERTY(BlueprintReadOnly, Category = "Frame")
    int32 SpikeCount;

    // System breakdown
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TArray<FSystemPerformanceData> SystemBreakdown;

    // Memory statistics
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    uint64 AverageMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    uint64 PeakMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float MemoryGrowthRate;

    // Gameplay metrics
    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 PeakMonsterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 PeakProjectileCount;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
    int32 TotalEventsProcessed;
};
```

### Profiling Session

```cpp
USTRUCT()
struct FProfilingSession
{
    GENERATED_BODY()

    FName SessionName;
    FDateTime StartTime;
    FDateTime EndTime;
    TArray<FFramePerformanceData> FrameData;
    TMap<FName, TArray<FSystemPerformanceData>> SystemData;
    TArray<FMemorySnapshot> MemorySnapshots;
    bool bActive = false;
};
```

### Asset Load Record

```cpp
USTRUCT()
struct FAssetLoadRecord
{
    GENERATED_BODY()

    FString AssetPath;
    FName AssetType;
    float LoadTime;
    uint64 AssetSize;
    bool bSynchronous;
    FDateTime Timestamp;
};
```

## Error Handling

### Validation Strategy

All telemetry operations include validation:

```cpp
bool UDelveDeepTelemetrySubsystem::CaptureBaseline(FName BaselineName)
{
    FValidationContext Context;
    Context.SystemName = TEXT("Telemetry");
    Context.OperationName = TEXT("CaptureBaseline");

    if (BaselineName.IsNone())
    {
        Context.AddError(TEXT("Baseline name cannot be empty"));
        UE_LOG(LogDelveDeepTelemetry, Error, TEXT("%s"), *Context.GetReport());
        return false;
    }

    if (Baselines.Contains(BaselineName))
    {
        Context.AddWarning(FString::Printf(
            TEXT("Baseline '%s' already exists and will be overwritten"),
            *BaselineName.ToString()));
    }

    // Capture baseline...
    return true;
}
```

### Error Recovery

**Memory Tracking Failure:**
- Log warning and continue with reduced functionality
- Disable per-system memory tracking
- Keep total memory tracking active

**Stat Group Registration Failure:**
- Log error with stat group name
- Continue with remaining stat groups
- Provide fallback timing using FPlatformTime

**Overlay Rendering Failure:**
- Catch rendering exceptions
- Disable overlay automatically
- Log error with stack trace

**Export Failure:**
- Validate file path before export
- Retry with alternative path if permission denied
- Log detailed error message with file path

## Testing Strategy

### Unit Tests

**Frame Tracking:**
- Test FPS calculation accuracy
- Test spike detection thresholds
- Test frame time history management
- Test percentile calculations

**System Profiling:**
- Test budget violation detection
- Test system time aggregation
- Test budget utilization calculation
- Test violation history management

**Memory Tracking:**
- Test memory snapshot capture
- Test leak detection algorithm
- Test growth rate calculation
- Test per-system memory tracking

**Baseline Management:**
- Test baseline capture
- Test baseline comparison
- Test baseline save/load
- Test baseline validation

### Integration Tests

**Telemetry with Event System:**
- Track event system performance
- Verify event count metrics
- Test event system budget tracking

**Telemetry with Configuration System:**
- Track configuration query performance
- Verify cache hit rate correlation
- Test configuration system budget

**Cross-System Performance:**
- Measure total frame time breakdown
- Verify budget allocations sum correctly
- Test performance under load

### Performance Tests

**Telemetry Overhead:**
- Measure telemetry system overhead
- Verify <0.5ms per frame target
- Test with overlay enabled/disabled
- Measure memory footprint

**Profiling Session Performance:**
- Test profiling session overhead
- Verify data collection performance
- Test session export performance

### Console Commands

```cpp
// Frame tracking
DelveDeep.Telemetry.ShowFPS
DelveDeep.Telemetry.ShowFrameStats
DelveDeep.Telemetry.ResetFrameStats

// System profiling
DelveDeep.Telemetry.ShowSystemStats
DelveDeep.Telemetry.ShowBudgets
DelveDeep.Telemetry.ResetBudgets

// Memory tracking
DelveDeep.Telemetry.ShowMemory
DelveDeep.Telemetry.ShowMemoryHistory
DelveDeep.Telemetry.CheckMemoryLeaks

// Baseline management
DelveDeep.Telemetry.CaptureBaseline <Name>
DelveDeep.Telemetry.CompareBaseline <Name>
DelveDeep.Telemetry.ListBaselines
DelveDeep.Telemetry.SaveBaseline <Name> <Path>
DelveDeep.Telemetry.LoadBaseline <Name> <Path>

// Reporting
DelveDeep.Telemetry.GenerateReport [Duration]
DelveDeep.Telemetry.ExportCSV <Path>
DelveDeep.Telemetry.ExportJSON <Path>

// Profiling
DelveDeep.Telemetry.StartProfiling <SessionName>
DelveDeep.Telemetry.StopProfiling
DelveDeep.Telemetry.ShowProfilingStatus

// Visualization
DelveDeep.Telemetry.EnableOverlay [Mode]
DelveDeep.Telemetry.DisableOverlay
DelveDeep.Telemetry.SetOverlayMode <Minimal|Standard|Detailed>

// Gameplay metrics
DelveDeep.Telemetry.ShowGameplayMetrics
DelveDeep.Telemetry.ShowAssetLoads
```

## Performance Considerations

### Optimization Strategies

**Minimal Overhead:**
- Use TCircularBuffer for fixed-size history
- Update memory metrics every 100 frames
- Cache FPS calculations
- Use fast path for disabled features

**Conditional Compilation:**
- Full telemetry in development builds
- Reduced telemetry in shipping builds
- Use preprocessor guards for expensive operations

**Efficient Data Structures:**
- TMap for O(1) system lookup
- TCircularBuffer for frame history
- Pre-allocated arrays for reports

**Lazy Evaluation:**
- Calculate statistics on demand
- Defer report generation
- Batch export operations

### Performance Targets

- Telemetry overhead: <0.5ms per frame
- Overlay rendering: <0.1ms per frame
- Memory snapshot: <1ms per capture
- Report generation: <100ms for 5-minute report
- Baseline capture: <50ms
- Export operation: <500ms for 5-minute data

### Memory Budget

- Frame history: ~14KB (3600 frames × 4 bytes)
- System data: ~1KB per system
- Memory history: ~10KB (100 snapshots)
- Baseline storage: ~5KB per baseline
- Profiling session: ~500KB for 60-second session

## Integration with Unreal Profiling

### Stat Command Integration

```cpp
// Enable DelveDeep stats
stat DelveDeep

// Enable specific system stats
stat DelveDeep.Combat
stat DelveDeep.AI
stat DelveDeep.Events

// Export to CSV
stat startfile
// ... gameplay ...
stat stopfile
```

### Unreal Insights Integration

```cpp
// Named scopes for Insights
TRACE_CPUPROFILER_EVENT_SCOPE(DelveDeep_CombatSystem);
TRACE_CPUPROFILER_EVENT_SCOPE(DelveDeep_AISystem);
TRACE_CPUPROFILER_EVENT_SCOPE(DelveDeep_EventSystem);
```

### CSV Profiler Integration

```cpp
// Custom CSV stats
CSV_SCOPED_TIMING_STAT(DelveDeep, CombatSystem);
CSV_CUSTOM_STAT(DelveDeep, ActiveMonsters, MonsterCount, ECsvCustomStatOp::Set);
```

## Blueprint Integration

### Blueprint Function Library

```cpp
UCLASS()
class DELVEDEEP_API UDelveDeepTelemetryBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry",
        meta = (WorldContext = "WorldContextObject"))
    static UDelveDeepTelemetrySubsystem* GetTelemetrySubsystem(
        const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "DelveDeep|Telemetry")
    static float GetCurrentFPS(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
    static void CapturePerformanceBaseline(
        UObject* WorldContextObject,
        FName BaselineName);

    UFUNCTION(BlueprintCallable, Category = "DelveDeep|Telemetry")
    static void EnablePerformanceOverlay(
        UObject* WorldContextObject,
        EOverlayMode Mode);
};
```

## Future Extensibility

### Network Performance Tracking

```cpp
struct FNetworkPerformanceData
{
    float Ping;
    float PacketLoss;
    uint64 BytesSent;
    uint64 BytesReceived;
    int32 ActiveConnections;
};
```

### GPU Performance Tracking

```cpp
struct FGPUPerformanceData
{
    float GPUFrameTime;
    uint64 GPUMemoryUsed;
    float DrawCallCount;
    float TriangleCount;
};
```

### Automated Performance Regression Detection

```cpp
class FPerformanceRegressionDetector
{
    bool DetectRegression(const FPerformanceBaseline& Baseline,
                         const FPerformanceReport& Current,
                         float ThresholdPercentage = 10.0f);
    
    TArray<FRegressionReport> GetRegressions() const;
};
```

## Dependencies

- **Unreal Engine Stat System**: Core profiling infrastructure
- **FValidationContext**: From Enhanced Validation System
- **UGameInstanceSubsystem**: Core Unreal Engine subsystem framework
- **UDelveDeepEventSubsystem**: For performance event broadcasting
- **No external dependencies**: Self-contained system
