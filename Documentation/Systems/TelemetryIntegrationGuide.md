# Telemetry System Integration Guide

## Overview

This guide provides practical examples for integrating the Performance Telemetry System into your DelveDeep systems. Learn how to add cycle counters, register budgets, capture baselines, and troubleshoot common issues.

## Table of Contents

1. [Adding Cycle Counters to New Systems](#adding-cycle-counters-to-new-systems)
2. [Registering System Budgets](#registering-system-budgets)
3. [Capturing and Comparing Baselines](#capturing-and-comparing-baselines)
4. [Using the Performance Overlay](#using-the-performance-overlay)
5. [Profiling Sessions](#profiling-sessions)
6. [Console Command Reference](#console-command-reference)
7. [Troubleshooting Guide](#troubleshooting-guide)

---

## Adding Cycle Counters to New Systems

### Step 1: Declare Stat Group and Cycle Counter

In your system's header file or in `DelveDeepStats.h`:

```cpp
// In DelveDeepStats.h

// Declare stat group for your system
DECLARE_STATS_GROUP(TEXT("DelveDeep.MySystem"), STATGROUP_DelveDeepMySystem, STATCAT_Advanced);

// Declare cycle counter for your system
DECLARE_CYCLE_STAT_EXTERN(TEXT("My System"), STAT_DelveDeep_MySystem, STATGROUP_DelveDeepMySystem, DELVEDEEP_API);

// Declare subsystem cycle counters
DECLARE_CYCLE_STAT_EXTERN(TEXT("My Subsystem Operation"), STAT_DelveDeep_MySubsystemOp, STATGROUP_DelveDeepMySystem, DELVEDEEP_API);
```

### Step 2: Define Cycle Counter

In your system's implementation file:

```cpp
// In MySystem.cpp

#include "DelveDeepStats.h"

// Define the cycle counter
DEFINE_STAT(STAT_DelveDeep_MySystem);
DEFINE_STAT(STAT_DelveDeep_MySubsystemOp);
```

### Step 3: Add Cycle Counter to Functions

Wrap performance-critical functions with `SCOPE_CYCLE_COUNTER`:

```cpp
void UMySystem::PerformExpensiveOperation()
{
    // Add cycle counter at the start of the function
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySystem);
    
    // Your function code here
    ProcessData();
    UpdateState();
}

void UMySystem::SubsystemOperation()
{
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySubsystemOp);
    
    // Subsystem operation code
}
```

### Step 4: Add Unreal Insights Integration

For deeper profiling with Unreal Insights:

```cpp
// In DelveDeepStats.h - Add trace macro
#if DELVEDEEP_TRACE_ENABLED
    #define TRACE_DELVEDEEP_MYSYSTEM() TRACE_CPUPROFILER_EVENT_SCOPE(DelveDeep_MySystem)
#else
    #define TRACE_DELVEDEEP_MYSYSTEM()
#endif

// In your function
void UMySystem::PerformExpensiveOperation()
{
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySystem);
    TRACE_DELVEDEEP_MYSYSTEM(); // Unreal Insights integration
    
    // Function code
}
```

### Step 5: Add CSV Profiler Integration

For CSV profiling support:

```cpp
// In DelveDeepStats.h - Add CSV macro
#if !UE_BUILD_SHIPPING
    #define CSV_DELVEDEEP_MYSYSTEM() CSV_SCOPED_TIMING_STAT(DelveDeep, MySystem)
#else
    #define CSV_DELVEDEEP_MYSYSTEM()
#endif

// In your function
void UMySystem::PerformExpensiveOperation()
{
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySystem);
    TRACE_DELVEDEEP_MYSYSTEM();
    CSV_DELVEDEEP_MYSYSTEM(); // CSV profiler integration
    
    // Function code
}
```

### Complete Example

```cpp
// MySystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MySystem.generated.h"

UCLASS()
class DELVEDEEP_API UMySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    
    void PerformExpensiveOperation();
    void ProcessBatch(const TArray<FData>& DataBatch);

private:
    void ProcessSingleItem(const FData& Item);
};

// MySystem.cpp
#include "MySystem.h"
#include "DelveDeepStats.h"

// Define cycle counters
DEFINE_STAT(STAT_DelveDeep_MySystem);
DEFINE_STAT(STAT_DelveDeep_MySystemBatch);
DEFINE_STAT(STAT_DelveDeep_MySystemItem);

void UMySystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register system budget with telemetry
    if (UDelveDeepTelemetrySubsystem* Telemetry = 
        GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>())
    {
        Telemetry->RegisterSystemBudget(TEXT("MySystem"), 2.0f); // 2ms budget
    }
}

void UMySystem::PerformExpensiveOperation()
{
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySystem);
    TRACE_DELVEDEEP_MYSYSTEM();
    CSV_DELVEDEEP_MYSYSTEM();
    
    // Expensive operation code
    for (int32 i = 0; i < 1000; ++i)
    {
        // Process data
    }
}

void UMySystem::ProcessBatch(const TArray<FData>& DataBatch)
{
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySystemBatch);
    
    for (const FData& Item : DataBatch)
    {
        ProcessSingleItem(Item);
    }
}

void UMySystem::ProcessSingleItem(const FData& Item)
{
    SCOPE_CYCLE_COUNTER(STAT_DelveDeep_MySystemItem);
    
    // Process individual item
}
```

---

## Registering System Budgets

### Method 1: Register in System Initialize

```cpp
void UMySystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Get telemetry subsystem
    UDelveDeepTelemetrySubsystem* Telemetry = 
        GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>();
    
    if (Telemetry)
    {
        // Register budget for main system (2ms)
        Telemetry->RegisterSystemBudget(TEXT("MySystem"), 2.0f);
        
        // Register budgets for subsystems
        Telemetry->RegisterSystemBudget(TEXT("MySystem_BatchProcessing"), 1.0f);
        Telemetry->RegisterSystemBudget(TEXT("MySystem_ItemProcessing"), 0.5f);
    }
}
```

### Method 2: Load from Data Asset

Create a performance budget data asset:

```cpp
// In Editor or via code
UDelveDeepPerformanceBudget* BudgetAsset = NewObject<UDelveDeepPerformanceBudget>();

// Add budget entries
FPerformanceBudgetEntry Entry;
Entry.SystemName = TEXT("MySystem");
Entry.BudgetMilliseconds = 2.0f;
Entry.bEnforceStrict = false;
Entry.Description = TEXT("Main system budget");
BudgetAsset->SystemBudgets.Add(Entry);

// Load budgets from asset
Telemetry->LoadBudgetsFromAsset(BudgetAsset);
```

### Method 3: Console Command

```bash
# Register budget via console
DelveDeep.Telemetry.RegisterBudget MySystem 2.0
```

### Budget Guidelines

**Recommended Budget Ranges:**
- Core systems (Combat, AI, World): 1.5-3.0ms
- UI systems: 0.5-1.5ms
- Event systems: 0.3-0.8ms
- Configuration systems: 0.1-0.5ms
- Utility systems: 0.1-0.3ms

**Total Frame Budget:**
- Target: 16.67ms (60 FPS)
- Game logic budget: ~10ms
- Rendering budget: ~6ms
- Reserve: ~0.67ms

---

## Capturing and Comparing Baselines

### Capturing a Baseline

#### Via Code

```cpp
void CapturePerformanceBaseline()
{
    UDelveDeepTelemetrySubsystem* Telemetry = 
        GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>();
    
    if (Telemetry)
    {
        // Capture baseline with descriptive name
        FName BaselineName = TEXT("BeforeOptimization_v1.0");
        if (Telemetry->CaptureBaseline(BaselineName))
        {
            UE_LOG(LogTemp, Display, TEXT("Baseline captured: %s"), *BaselineName.ToString());
            
            // Optionally save to disk
            Telemetry->SaveBaseline(BaselineName);
        }
    }
}
```

#### Via Console

```bash
# Capture baseline
DelveDeep.Telemetry.CaptureBaseline BeforeOptimization

# Save baseline to disk
DelveDeep.Telemetry.SaveBaseline BeforeOptimization

# List available baselines
DelveDeep.Telemetry.ListBaselines
```

### Comparing to Baseline

#### Via Code

```cpp
void ComparePerformance()
{
    UDelveDeepTelemetrySubsystem* Telemetry = 
        GetGameInstance()->GetSubsystem<UDelveDeepTelemetrySubsystem>();
    
    if (Telemetry)
    {
        FPerformanceComparison Comparison;
        if (Telemetry->CompareToBaseline(TEXT("BeforeOptimization"), Comparison))
        {
            // Check for regression
            if (Comparison.bIsRegression)
            {
                UE_LOG(LogTemp, Warning, TEXT("Performance regression detected!"));
                UE_LOG(LogTemp, Warning, TEXT("FPS change: %.2f%%"), Comparison.FPSChangePercent);
                UE_LOG(LogTemp, Warning, TEXT("Frame time change: %.2f%%"), Comparison.FrameTimeChangePercent);
            }
            else if (Comparison.bIsImprovement)
            {
                UE_LOG(LogTemp, Display, TEXT("Performance improvement detected!"));
                UE_LOG(LogTemp, Display, TEXT("FPS change: +%.2f%%"), Comparison.FPSChangePercent);
            }
            
            // Print detailed report
            UE_LOG(LogTemp, Display, TEXT("%s"), *Comparison.DetailedReport);
        }
    }
}
```

#### Via Console

```bash
# Compare to baseline
DelveDeep.Telemetry.CompareBaseline BeforeOptimization

# Output shows:
# - FPS change percentage
# - Frame time change percentage
# - Memory change percentage
# - Per-system performance changes
# - Regression/improvement detection
```

### Baseline Workflow Example

```cpp
// 1. Capture baseline before optimization
void BeginOptimizationWork()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    // Capture and save baseline
    Telemetry->CaptureBaseline(TEXT("Pre_CombatOptimization"));
    Telemetry->SaveBaseline(TEXT("Pre_CombatOptimization"));
    
    UE_LOG(LogTemp, Display, TEXT("Baseline captured. Begin optimization work."));
}

// 2. After optimization, compare results
void VerifyOptimizationResults()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    // Compare to baseline
    FPerformanceComparison Comparison;
    if (Telemetry->CompareToBaseline(TEXT("Pre_CombatOptimization"), Comparison))
    {
        // Log results
        UE_LOG(LogTemp, Display, TEXT("Optimization Results:"));
        UE_LOG(LogTemp, Display, TEXT("  FPS: %.2f%% change"), Comparison.FPSChangePercent);
        UE_LOG(LogTemp, Display, TEXT("  Frame Time: %.2f%% change"), Comparison.FrameTimeChangePercent);
        UE_LOG(LogTemp, Display, TEXT("  Memory: %.2f%% change"), Comparison.MemoryChangePercent);
        
        // Check specific system improvements
        for (const auto& Pair : Comparison.SystemTimeChanges)
        {
            if (Pair.Key == TEXT("Combat"))
            {
                UE_LOG(LogTemp, Display, TEXT("  Combat System: %.2f%% change"), Pair.Value);
            }
        }
        
        // Capture new baseline if improvement
        if (Comparison.bIsImprovement)
        {
            Telemetry->CaptureBaseline(TEXT("Post_CombatOptimization"));
            Telemetry->SaveBaseline(TEXT("Post_CombatOptimization"));
        }
    }
}
```

---

## Using the Performance Overlay

### Enabling the Overlay

#### Via Code

```cpp
void EnablePerformanceMonitoring()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    if (Telemetry)
    {
        // Enable with standard mode (FPS + graph)
        Telemetry->EnablePerformanceOverlay(EOverlayMode::Standard);
        
        // Or use minimal mode (FPS only)
        // Telemetry->EnablePerformanceOverlay(EOverlayMode::Minimal);
        
        // Or use detailed mode (FPS + graph + systems + memory)
        // Telemetry->EnablePerformanceOverlay(EOverlayMode::Detailed);
    }
}

void DisablePerformanceMonitoring()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    if (Telemetry)
    {
        Telemetry->DisablePerformanceOverlay();
    }
}
```

#### Via Console

```bash
# Enable overlay with standard mode
DelveDeep.Telemetry.EnableOverlay

# Enable with specific mode
DelveDeep.Telemetry.EnableOverlay Minimal
DelveDeep.Telemetry.EnableOverlay Standard
DelveDeep.Telemetry.EnableOverlay Detailed

# Disable overlay
DelveDeep.Telemetry.DisableOverlay

# Change mode while enabled
DelveDeep.Telemetry.SetOverlayMode Detailed
```

### Overlay Modes

**Minimal Mode:**
- Current FPS only
- Minimal screen space
- <0.05ms overhead

**Standard Mode:**
- Current FPS
- Frame time graph (last 120 frames)
- Color-coded performance zones
- ~0.08ms overhead

**Detailed Mode:**
- Current FPS
- Frame time graph
- Top 5 systems by time
- Memory usage
- Budget violations
- ~0.1ms overhead

---

## Profiling Sessions

### Starting a Profiling Session

```cpp
void BeginDetailedProfiling()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    if (Telemetry)
    {
        // Start profiling session
        FName SessionName = TEXT("CombatEncounter_Boss1");
        if (Telemetry->StartProfilingSession(SessionName))
        {
            UE_LOG(LogTemp, Display, TEXT("Profiling session started: %s"), *SessionName.ToString());
        }
    }
}

void EndDetailedProfiling()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    if (Telemetry)
    {
        // Stop profiling session (auto-saves)
        if (Telemetry->StopProfilingSession())
        {
            // Generate report
            FProfilingSessionReport Report;
            if (Telemetry->GenerateProfilingReport(Report))
            {
                UE_LOG(LogTemp, Display, TEXT("Profiling Report:"));
                UE_LOG(LogTemp, Display, TEXT("  Duration: %.2fs"), Report.DurationSeconds);
                UE_LOG(LogTemp, Display, TEXT("  Avg FPS: %.2f"), Report.AverageFPS);
                UE_LOG(LogTemp, Display, TEXT("  1%% Low: %.2f"), Report.OnePercentLowFPS);
                UE_LOG(LogTemp, Display, TEXT("  Spikes: %d"), Report.SpikeCount);
            }
        }
    }
}
```

### Console Commands

```bash
# Start profiling session
DelveDeep.Telemetry.StartProfiling BossEncounter

# Stop profiling session (auto-saves to Saved/Profiling/)
DelveDeep.Telemetry.StopProfiling

# Check profiling status
DelveDeep.Telemetry.ShowProfilingStatus
```

### Profiling Workflow

```cpp
// Example: Profile a specific gameplay scenario
void ProfileBossEncounter()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    // 1. Start profiling before encounter
    Telemetry->StartProfilingSession(TEXT("Boss_DragonKing"));
    
    // 2. Trigger boss encounter
    SpawnBoss();
    
    // 3. Let encounter play out (profiling captures all data)
    // Session will auto-stop after 60 seconds or manually stop
    
    // 4. Stop profiling after encounter
    Telemetry->StopProfilingSession();
    
    // 5. Generate and analyze report
    FProfilingSessionReport Report;
    if (Telemetry->GenerateProfilingReport(Report))
    {
        // Analyze results
        if (Report.AverageFPS < 60.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Boss encounter below 60 FPS target"));
            
            // Check which systems are bottlenecks
            for (const FSystemPerformanceData& System : Report.SystemBreakdown)
            {
                if (System.AverageTimeMs > System.BudgetTimeMs)
                {
                    UE_LOG(LogTemp, Warning, TEXT("  %s over budget: %.2f ms / %.2f ms"),
                        *System.SystemName.ToString(),
                        System.AverageTimeMs,
                        System.BudgetTimeMs);
                }
            }
        }
    }
}
```

---

## Console Command Reference

### Frame Performance

```bash
# Show current FPS
DelveDeep.Telemetry.ShowFPS

# Show detailed frame statistics
DelveDeep.Telemetry.ShowFrameStats

# Reset frame statistics
DelveDeep.Telemetry.ResetFrameStats
```

### System Profiling

```bash
# Show all system performance data
DelveDeep.Telemetry.ShowSystemStats

# Show budget utilization
DelveDeep.Telemetry.ShowBudgets

# Register new budget
DelveDeep.Telemetry.RegisterBudget <SystemName> <BudgetMs>

# Reset system statistics
DelveDeep.Telemetry.ResetSystemStats
```

### Memory Tracking

```bash
# Show current memory usage
DelveDeep.Telemetry.ShowMemory

# Show memory history
DelveDeep.Telemetry.ShowMemoryHistory

# Check for memory leaks
DelveDeep.Telemetry.CheckMemoryLeaks
```

### Baseline Management

```bash
# Capture baseline
DelveDeep.Telemetry.CaptureBaseline <Name>

# Compare to baseline
DelveDeep.Telemetry.CompareBaseline <Name>

# List all baselines
DelveDeep.Telemetry.ListBaselines

# Save baseline to disk
DelveDeep.Telemetry.SaveBaseline <Name> [Path]

# Load baseline from disk
DelveDeep.Telemetry.LoadBaseline <Name> <Path>

# Delete baseline
DelveDeep.Telemetry.DeleteBaseline <Name>
```

### Performance Reporting

```bash
# Generate performance report (default: 5 minutes)
DelveDeep.Telemetry.GenerateReport

# Generate report for specific duration
DelveDeep.Telemetry.GenerateReport 600

# Export report to CSV
DelveDeep.Telemetry.ExportCSV <Path>

# Export report to JSON
DelveDeep.Telemetry.ExportJSON <Path>
```

### Performance Overlay

```bash
# Enable overlay (default: Standard mode)
DelveDeep.Telemetry.EnableOverlay

# Enable with specific mode
DelveDeep.Telemetry.EnableOverlay Minimal
DelveDeep.Telemetry.EnableOverlay Standard
DelveDeep.Telemetry.EnableOverlay Detailed

# Disable overlay
DelveDeep.Telemetry.DisableOverlay

# Change overlay mode
DelveDeep.Telemetry.SetOverlayMode <Mode>
```

### Profiling Sessions

```bash
# Start profiling session
DelveDeep.Telemetry.StartProfiling <SessionName>

# Stop profiling session
DelveDeep.Telemetry.StopProfiling

# Show profiling status
DelveDeep.Telemetry.ShowProfilingStatus
```

### Gameplay Metrics

```bash
# Show gameplay metrics
DelveDeep.Telemetry.ShowGameplayMetrics

# Show asset load statistics
DelveDeep.Telemetry.ShowAssetLoads
```

---

## Troubleshooting Guide

### Issue: High Telemetry Overhead

**Symptoms:**
- Telemetry system consuming >0.5ms per frame
- Performance degradation when telemetry enabled

**Solutions:**
1. Disable performance overlay if not needed
2. Stop profiling sessions when not actively profiling
3. Reduce memory tracking frequency
4. Check for excessive cycle counter nesting

```cpp
// Check telemetry overhead
stat DelveDeep
// Look for STAT_DelveDeep_TelemetrySystem time

// Disable expensive features
Telemetry->DisablePerformanceOverlay();
Telemetry->StopProfilingSession();
```

### Issue: Missing Stat Groups

**Symptoms:**
- `stat DelveDeep` shows no data
- Cycle counters not appearing in profiler

**Solutions:**
1. Ensure cycle counters are defined (DEFINE_STAT)
2. Check that SCOPE_CYCLE_COUNTER is used correctly
3. Verify stat groups are declared with DELVEDEEP_API

```cpp
// Correct declaration
DECLARE_CYCLE_STAT_EXTERN(TEXT("My System"), STAT_MySystem, STATGROUP_DelveDeep, DELVEDEEP_API);

// Correct definition
DEFINE_STAT(STAT_MySystem);

// Correct usage
void MyFunction()
{
    SCOPE_CYCLE_COUNTER(STAT_MySystem);
    // Function code
}
```

### Issue: Baseline Comparison Fails

**Symptoms:**
- "Baseline not found" error
- Comparison shows unexpected results

**Solutions:**
1. Verify baseline was captured: `DelveDeep.Telemetry.ListBaselines`
2. Check baseline name spelling
3. Ensure baseline is compatible with current build

```cpp
// List available baselines
TArray<FName> Baselines = Telemetry->GetAvailableBaselines();
for (FName Name : Baselines)
{
    UE_LOG(LogTemp, Display, TEXT("Available baseline: %s"), *Name.ToString());
}

// Get baseline details
FPerformanceBaseline Baseline;
if (Telemetry->GetBaseline(TEXT("MyBaseline"), Baseline))
{
    UE_LOG(LogTemp, Display, TEXT("Baseline build: %s"), *Baseline.BuildVersion);
    UE_LOG(LogTemp, Display, TEXT("Baseline FPS: %.2f"), Baseline.AverageFPS);
}
```

### Issue: Budget Violations Not Detected

**Symptoms:**
- System exceeding budget but no warnings
- Budget utilization shows 0%

**Solutions:**
1. Verify budget is registered
2. Check that cycle counters are active
3. Ensure system name matches exactly

```cpp
// Verify budget registration
float Utilization = Telemetry->GetSystemBudgetUtilization(TEXT("MySystem"));
if (Utilization == 0.0f)
{
    // Budget not registered or no data
    Telemetry->RegisterSystemBudget(TEXT("MySystem"), 2.0f);
}

// Check for violations
if (Telemetry->IsSystemBudgetViolated(TEXT("MySystem")))
{
    FSystemPerformanceData PerfData = Telemetry->GetSystemPerformance(TEXT("MySystem"));
    UE_LOG(LogTemp, Warning, TEXT("Budget violated: %.2f ms / %.2f ms"),
        PerfData.AverageTimeMs, PerfData.BudgetTimeMs);
}
```

### Issue: Overlay Not Rendering

**Symptoms:**
- Overlay enabled but not visible
- Overlay rendering errors in log

**Solutions:**
1. Check that HUD is calling RenderPerformanceOverlay
2. Verify canvas is valid
3. Check for rendering exceptions in log

```cpp
// In your HUD class
void AMyHUD::DrawHUD()
{
    Super::DrawHUD();
    
    // Render telemetry overlay
    if (UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry())
    {
        if (Telemetry->IsOverlayEnabled())
        {
            Telemetry->RenderPerformanceOverlay(Canvas);
        }
    }
}
```

### Issue: Profiling Session Not Capturing Data

**Symptoms:**
- Profiling session shows 0 frames
- Report generation fails

**Solutions:**
1. Verify session is active: `DelveDeep.Telemetry.ShowProfilingStatus`
2. Check that telemetry is enabled
3. Ensure session hasn't exceeded max duration (60s)

```cpp
// Check session status
if (Telemetry->IsProfilingActive())
{
    FProfilingSession Session;
    if (Telemetry->GetCurrentSession(Session))
    {
        UE_LOG(LogTemp, Display, TEXT("Session: %s"), *Session.SessionName.ToString());
        UE_LOG(LogTemp, Display, TEXT("Duration: %.2fs"), Session.DurationSeconds);
        UE_LOG(LogTemp, Display, TEXT("Frames: %d"), Session.TotalFrames);
    }
}
```

### Issue: Memory Leak False Positives

**Symptoms:**
- Memory leak warnings when no leak exists
- Memory growth rate fluctuating

**Solutions:**
1. Check memory tracking frequency (every 100 frames)
2. Verify growth rate threshold (10MB/min)
3. Capture longer baseline for accurate trend

```cpp
// Check memory statistics
FMemorySnapshot Memory = Telemetry->GetCurrentMemorySnapshot();
float GrowthRate = Telemetry->GetMemoryGrowthRate();

UE_LOG(LogTemp, Display, TEXT("Memory: %.2f MB"), Memory.TotalMemory / (1024.0 * 1024.0));
UE_LOG(LogTemp, Display, TEXT("Growth Rate: %.2f MB/min"), GrowthRate);

// If false positive, growth rate should stabilize over time
```

---

## Best Practices

### 1. Use Descriptive Names

```cpp
// Good
Telemetry->RegisterSystemBudget(TEXT("Combat_DamageCalculation"), 0.5f);
Telemetry->CaptureBaseline(TEXT("Pre_AIOptimization_v2.1"));

// Bad
Telemetry->RegisterSystemBudget(TEXT("System1"), 0.5f);
Telemetry->CaptureBaseline(TEXT("Baseline1"));
```

### 2. Capture Baselines at Key Points

- Before major optimization work
- After completing optimization
- Before and after feature additions
- At milestone builds

### 3. Use Appropriate Budget Values

- Start with generous budgets
- Tighten budgets as optimization progresses
- Leave headroom for worst-case scenarios
- Consider platform differences

### 4. Profile Representative Scenarios

- Profile worst-case gameplay situations
- Include various entity counts
- Test different map sizes
- Profile with realistic player behavior

### 5. Regular Performance Monitoring

```cpp
// Example: Automated performance checks
void PerformDailyPerformanceCheck()
{
    UDelveDeepTelemetrySubsystem* Telemetry = GetTelemetry();
    
    // Generate daily report
    FPerformanceReport Report;
    if (Telemetry->GeneratePerformanceReport(Report, 3600.0f)) // 1 hour
    {
        // Export to CSV for tracking
        FString FilePath = FString::Printf(
            TEXT("Saved/PerformanceReports/Daily_%s.csv"),
            *FDateTime::Now().ToString(TEXT("%Y%m%d")));
        
        Telemetry->ExportReportToCSV(Report, FilePath);
        
        // Check for regressions
        FPerformanceComparison Comparison;
        if (Telemetry->CompareToBaseline(TEXT("GoldMaster"), Comparison))
        {
            if (Comparison.bIsRegression)
            {
                // Alert team of regression
                UE_LOG(LogTemp, Error, TEXT("Performance regression detected!"));
            }
        }
    }
}
```

---

## Additional Resources

- [Telemetry System Overview](TelemetrySystem.md)
- [Build Configurations](TelemetryBuildConfigurations.md)
- [Performance Testing Guide](Performance-Testing.md)
- [DelveDeep Stats Reference](../../Source/DelveDeep/Public/DelveDeepStats.h)

---

**Last Updated:** October 24, 2025

**Version:** 1.0

**Status:** Complete - Task 19
